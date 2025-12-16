#include "types.h"
#include "riscv.h"
#include "virtio.h"
#include "proc.h"
#include "vm.h"

void printf(const char *fmt, ...);
extern void* alloc_page(void);

// QEMU 将 virtio-blk 设备映射到这个地址
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

static struct disk disk;

// 读取 MMIO 寄存器
static uint32_t virtio_read32(uint64_t addr) {
    return *(volatile uint32_t*)addr;
}

// 写入 MMIO 寄存器
static void virtio_write32(uint64_t addr, uint32_t value) {
    *(volatile uint32_t*)addr = value;
}

// 初始化 virtio-blk 设备
void virtio_disk_init(void) {
    uint32_t status = 0;

    initlock(&disk.vdisk_lock, "virtio_disk");

    printf("Initializing virtio-blk device...\n");

    // 1. 重置设备
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_STATUS, 0);

    // 2. 设置 ACKNOWLEDGE 状态位
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_STATUS, status);

    // 3. 设置 DRIVER 状态位
    status |= VIRTIO_CONFIG_S_DRIVER;
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_STATUS, status);

    // 4. 读取设备特性
    uint32_t features = virtio_read32(VIRTIO0 + VIRTIO_MMIO_DEVICE_FEATURES);
    printf("Device features: 0x%x\n", features);

    // 5. 写入我们理解的特性（不需要特殊特性）
    features = 0;
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_DRIVER_FEATURES, features);

    // 6. 设置 FEATURES_OK 状态位
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_STATUS, status);

    // 7. 重新读取状态，确保设备接受特性
    status = virtio_read32(VIRTIO0 + VIRTIO_MMIO_STATUS);
    if(!(status & VIRTIO_CONFIG_S_FEATURES_OK)) {
        printf("virtio disk FEATURES_OK failed\n");
        return;
    }

    // 8. 分配和初始化队列
    // 检查 MMIO 版本来决定内存分配方式
    uint32_t version = virtio_read32(VIRTIO0 + VIRTIO_MMIO_VERSION);
    printf("VirtIO MMIO version: %d\n", version);

    if(version == 1) {
        // Legacy 模式需要连续内存：desc, avail, used 必须连在一起
        // 为整个 virtqueue 分配一个大的连续内存块

        // 计算所需大小（参考 VirtIO 规范）
        // Descriptor Table: 16 * NUM_DESC bytes
        // Available Ring: 6 + 2 * NUM_DESC bytes
        // Used Ring: 6 + 8 * NUM_DESC bytes
        // 需要对齐到页边界

        char *vq_mem = (char*)alloc_page();  // 分配一个页面
        if(!vq_mem) {
            printf("virtio disk alloc_page failed for queue\n");
            return;
        }

        // 清零内存
        for(int i = 0; i < 4096; i++) {
            vq_mem[i] = 0;
        }

        // 按照 VirtIO legacy 布局设置指针
        disk.desc = (struct virtq_desc*)vq_mem;

        // Available ring 跟在描述符表后面
        disk.avail = (struct virtq_avail*)(vq_mem + 16 * NUM_DESC);

        // Used ring 需要对齐到页边界（或至少对齐到合适的边界）
        // 简化：直接跟在 avail 后面
        disk.used = (struct virtq_used*)(vq_mem + 16 * NUM_DESC + 6 + 2 * NUM_DESC);

        printf("Legacy mode: using contiguous memory at %p\n", vq_mem);
        printf("  desc=%p, avail=%p, used=%p\n", disk.desc, disk.avail, disk.used);
    } else {
        // Modern 模式可以使用分离的页面
        disk.desc = (struct virtq_desc*)alloc_page();
        disk.avail = (struct virtq_avail*)alloc_page();
        disk.used = (struct virtq_used*)alloc_page();

        if(!disk.desc || !disk.avail || !disk.used) {
            printf("virtio disk alloc_page failed\n");
            return;
        }

        // 清零内存
        for(int i = 0; i < 4096; i++) {
            ((char*)disk.desc)[i] = 0;
            ((char*)disk.avail)[i] = 0;
            ((char*)disk.used)[i] = 0;
        }
    }

    // 初始化空闲描述符链表
    for(int i = 0; i < NUM_DESC; i++)
        disk.free[i] = 1;

    // 9. 设置队列 0
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_SEL, 0);

    // 检查队列最大大小
    uint32_t max = virtio_read32(VIRTIO0 + VIRTIO_MMIO_QUEUE_NUM_MAX);
    if(max == 0) {
        printf("virtio disk has no queue 0\n");
        return;
    }
    if(max < NUM_DESC) {
        printf("virtio disk max queue too short\n");
        return;
    }

    // 设置队列大小
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_NUM, NUM_DESC);

    // Legacy 模式：设置客户机页面大小和队列地址
    if(version == 1) {
        // 设置客户机页面大小 (必须是 4096)
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_GUEST_PAGE_SIZE, 4096);

        // 设置队列物理页帧号（PFN）
        // 这会让设备知道整个 virtqueue 的起始地址
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_PFN,
                       ((uint64_t)disk.desc) >> 12);
    } else {
        // Modern 模式：分别设置每个部分的地址
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_DESC_LOW, (uint64_t)disk.desc);
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_DESC_HIGH, ((uint64_t)disk.desc) >> 32);
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_AVAIL_LOW, (uint64_t)disk.avail);
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_AVAIL_HIGH, ((uint64_t)disk.avail) >> 32);
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_USED_LOW, (uint64_t)disk.used);
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_USED_HIGH, ((uint64_t)disk.used) >> 32);
        virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_READY, 1);
    }

    // 10. 设置 DRIVER_OK 状态位
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_STATUS, status);

    printf("virtio-blk device initialized successfully\n");
}

// 分配三个描述符（链式）
// 返回第一个描述符的索引，如果失败返回 -1
static int alloc3_desc(int *idx) {
    for(int i = 0; i < 3; i++) {
        idx[i] = -1;
        for(int j = 0; j < NUM_DESC; j++) {
            if(disk.free[j]) {
                disk.free[j] = 0;
                idx[i] = j;
                break;
            }
        }
        if(idx[i] < 0) {
            // 分配失败，释放已分配的
            for(int j = 0; j < i; j++) {
                if(idx[j] >= 0)
                    disk.free[idx[j]] = 1;
            }
            return -1;
        }
    }
    return 0;
}

// 释放描述符链
static void free_desc(int i) {
    if(i >= NUM_DESC)
        return;
    if(disk.free[i])
        return;
    disk.free[i] = 1;
    struct virtq_desc *d = &disk.desc[i];
    if(d->flags & VRING_DESC_F_NEXT)
        free_desc(d->next);
}

// 读写磁盘块
// write: 1 表示写，0 表示读
void virtio_disk_rw(char *buf, uint32_t blockno, int write) {
    uint64_t sector = blockno * (512 / 512);  // 扇区号

    acquire(&disk.vdisk_lock);

    // 分配三个描述符：请求头、数据缓冲区、状态字节
    int idx[3];
    while(alloc3_desc(idx) != 0)
        ;  // 等待描述符可用

    // 填充请求头（存储在 disk.info 中）
    disk.info[idx[0]].req.type = write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    disk.info[idx[0]].req.reserved = 0;
    disk.info[idx[0]].req.sector = sector;

    // 格式化三个描述符
    // 1. 请求头描述符
    disk.desc[idx[0]].addr = (uint64_t)&disk.info[idx[0]].req;
    disk.desc[idx[0]].len = sizeof(struct virtio_blk_req);
    disk.desc[idx[0]].flags = VRING_DESC_F_NEXT;
    disk.desc[idx[0]].next = idx[1];

    // 2. 数据缓冲区描述符
    disk.desc[idx[1]].addr = (uint64_t)buf;
    disk.desc[idx[1]].len = 512;
    disk.desc[idx[1]].flags = (write ? 0 : VRING_DESC_F_WRITE) | VRING_DESC_F_NEXT;
    disk.desc[idx[1]].next = idx[2];

    // 3. 状态字节描述符
    disk.info[idx[0]].status = 0xff;  // 设备会写入状态
    disk.desc[idx[2]].addr = (uint64_t)&disk.info[idx[0]].status;
    disk.desc[idx[2]].len = 1;
    disk.desc[idx[2]].flags = VRING_DESC_F_WRITE;  // 设备写入
    disk.desc[idx[2]].next = 0;

    // 记录缓冲区指针
    disk.info[idx[0]].buf = buf;

    // 将第一个描述符添加到可用环
    disk.avail->ring[disk.avail->idx % NUM_DESC] = idx[0];

    // 内存屏障：确保描述符在更新 avail->idx 之前写入
    __sync_synchronize();

    // 更新 avail->idx
    disk.avail->idx++;

    // 内存屏障：确保 avail->idx 在通知设备之前写入
    __sync_synchronize();

    // 通知设备
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_QUEUE_NOTIFY, 0);

    // 等待设备完成
    while(disk.info[idx[0]].status == 0xff)
        ;  // 忙等待

    // 检查状态
    if(disk.info[idx[0]].status != 0) {
        printf("virtio_disk_rw: error status %d\n", disk.info[idx[0]].status);
    }

    // 释放描述符
    free_desc(idx[0]);

    release(&disk.vdisk_lock);
}

// 处理 virtio-blk 中断
void virtio_disk_intr(void) {
    acquire(&disk.vdisk_lock);

    // 确认中断
    virtio_write32(VIRTIO0 + VIRTIO_MMIO_INTERRUPT_ACK,
                   virtio_read32(VIRTIO0 + VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3);

    __sync_synchronize();

    // 处理已完成的请求
    while(disk.used_idx != disk.used->idx) {
        __sync_synchronize();
        int id = disk.used->ring[disk.used_idx % NUM_DESC].id;

        if(disk.info[id].status != 0)
            printf("virtio_disk_intr: error status\n");

        disk.info[id].buf = 0;
        disk.used_idx++;
    }

    release(&disk.vdisk_lock);
}
