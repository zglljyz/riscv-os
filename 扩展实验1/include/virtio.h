#pragma once

#include "types.h"
#include "proc.h"  // 需要 struct spinlock 定义

// VirtIO MMIO 设备寄存器偏移 (规范定义)
#define VIRTIO_MMIO_MAGIC_VALUE         0x000  // 0x74726976
#define VIRTIO_MMIO_VERSION             0x004  // version; 1 is legacy
#define VIRTIO_MMIO_DEVICE_ID           0x008  // device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_VENDOR_ID           0x00c  // 0x554d4551
#define VIRTIO_MMIO_DEVICE_FEATURES     0x010
#define VIRTIO_MMIO_DRIVER_FEATURES     0x020
#define VIRTIO_MMIO_GUEST_PAGE_SIZE     0x028  // page size for PFN, write-only
#define VIRTIO_MMIO_QUEUE_SEL           0x030  // select queue, write-only
#define VIRTIO_MMIO_QUEUE_NUM_MAX       0x034  // max size of current queue, read-only
#define VIRTIO_MMIO_QUEUE_NUM           0x038  // size of current queue, write-only
#define VIRTIO_MMIO_QUEUE_ALIGN         0x03c  // used ring alignment, write-only
#define VIRTIO_MMIO_QUEUE_PFN           0x040  // physical page number for queue, read/write
#define VIRTIO_MMIO_QUEUE_READY         0x044  // ready bit
#define VIRTIO_MMIO_QUEUE_NOTIFY        0x050  // write-only
#define VIRTIO_MMIO_INTERRUPT_STATUS    0x060  // read-only
#define VIRTIO_MMIO_INTERRUPT_ACK       0x064  // write-only
#define VIRTIO_MMIO_STATUS              0x070  // read/write

// VirtIO MMIO v2 队列地址寄存器 (仅用于 version >= 2)
#define VIRTIO_MMIO_QUEUE_DESC_LOW      0x080  // 描述符表地址低32位
#define VIRTIO_MMIO_QUEUE_DESC_HIGH     0x084  // 描述符表地址高32位
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW     0x090  // 可用环地址低32位
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH    0x094  // 可用环地址高32位
#define VIRTIO_MMIO_QUEUE_USED_LOW      0x0a0  // 已用环地址低32位
#define VIRTIO_MMIO_QUEUE_USED_HIGH     0x0a4  // 已用环地址高32位

// status register bits
#define VIRTIO_CONFIG_S_ACKNOWLEDGE     1
#define VIRTIO_CONFIG_S_DRIVER          2
#define VIRTIO_CONFIG_S_DRIVER_OK       4
#define VIRTIO_CONFIG_S_FEATURES_OK     8

// device feature bits
#define VIRTIO_BLK_F_RO                 5   // Disk is read-only
#define VIRTIO_BLK_F_SCSI               7   // Supports scsi command passthru
#define VIRTIO_BLK_F_CONFIG_WCE         11  // Writeback mode available in config
#define VIRTIO_BLK_F_MQ                 12  // support more than one vq
#define VIRTIO_F_ANY_LAYOUT             27
#define VIRTIO_RING_F_INDIRECT_DESC     28
#define VIRTIO_RING_F_EVENT_IDX         29

// VirtIO Block 设备的配置空间偏移
#define VIRTIO_BLK_CONFIG_CAPACITY      0x100
#define VIRTIO_BLK_CONFIG_SIZE_MAX      0x108
#define VIRTIO_BLK_CONFIG_SEG_MAX       0x10c
#define VIRTIO_BLK_CONFIG_BLK_SIZE      0x114

// virtqueue 描述符标志
#define VRING_DESC_F_NEXT       1  // chained with another descriptor
#define VRING_DESC_F_WRITE      2  // device writes (vs read)
#define VRING_DESC_F_INDIRECT   4  // buffer contains a list of buffer descriptors

// virtqueue 大小
#define NUM_DESC 8   // 描述符数量

// virtio_blk 请求类型
#define VIRTIO_BLK_T_IN         0   // read the disk
#define VIRTIO_BLK_T_OUT        1   // write to disk

// virtio_blk 请求状态
#define VIRTIO_BLK_S_OK         0
#define VIRTIO_BLK_S_IOERR      1
#define VIRTIO_BLK_S_UNSUPP     2

// virtqueue 描述符表
struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

// virtqueue 可用环
struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[NUM_DESC];
    uint16_t used_event;  // only if VIRTIO_F_EVENT_IDX
};

// virtqueue 使用环元素
struct virtq_used_elem {
    uint32_t id;    // index of start of used descriptor chain
    uint32_t len;   // total length of the descriptor chain
};

// virtqueue 使用环
struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[NUM_DESC];
    uint16_t avail_event;  // only if VIRTIO_F_EVENT_IDX
};

// VirtIO Block 请求头
struct virtio_blk_req {
    uint32_t type;      // VIRTIO_BLK_T_IN or VIRTIO_BLK_T_OUT
    uint32_t reserved;
    uint64_t sector;
};

// VirtIO 磁盘信息结构
struct disk {
    // virtqueue 的内存布局
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;

    // 我们自己的跟踪信息
    char free[NUM_DESC];  // 每个描述符是否空闲
    uint16_t used_idx;    // 我们上次检查的 used->idx

    // 每个描述符对应的请求信息
    struct {
        char *buf;
        char status;
        struct virtio_blk_req req;  // 存储请求头
    } info[NUM_DESC];

    // 描述符分配锁
    struct spinlock vdisk_lock;
};

// virtio-blk 驱动函数
void virtio_disk_init(void);
void virtio_disk_rw(char *buf, uint32_t blockno, int write);
void virtio_disk_intr(void);
