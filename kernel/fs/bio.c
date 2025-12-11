#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "proc.h"
#include "string.h"
#include "virtio.h"

// 外部函数声明
void printf(const char *fmt, ...);
void *alloc_page(void);
void free_page(void *pa);

//===========================================
// 实验七：块缓存系统实现 (bio.c)
// 使用真实的 virtio-blk 设备
//===========================================

#define NBUF 30  // 缓存块数量

struct {
    struct spinlock lock;
    struct buf buf[NBUF];

    // 链表头，连接所有缓存块形成LRU链表
    struct buf head;
} bcache;

// 使用 virtio-blk 设备进行真实的磁盘读写
static void disk_read(struct buf *b) {
    virtio_disk_rw((char*)b->data, b->blockno, 0);  // 0 表示读
}

static void disk_write(struct buf *b) {
    virtio_disk_rw((char*)b->data, b->blockno, 1);  // 1 表示写
}

// 初始化块缓存系统
void binit(void) {
    struct buf *b;

    initlock(&bcache.lock, "bcache");

    // 创建链表头
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;

    // 初始化所有缓存块并加入LRU链表
    for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
        initlock(&b->lock, "buffer");
    }

    printf("Block cache initialized with %d buffers\n", NBUF);
}

// 在缓存中查找指定的块，如果不存在则分配一个新的缓存块
static struct buf* bget(uint32_t dev, uint32_t blockno) {
    struct buf *b;

    acquire(&bcache.lock);

    // 首先检查块是否已经在缓存中
    for (b = bcache.head.next; b != &bcache.head; b = b->next) {
        if (b->dev == dev && b->blockno == blockno) {
            b->refcnt++;
            release(&bcache.lock);
            acquire(&b->lock);
            return b;
        }
    }

    // 块不在缓存中，需要分配一个新的缓存块
    // 从链表尾部向前查找未使用的块（LRU替换策略）
    for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
        if (b->refcnt == 0) {
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0;
            b->refcnt = 1;
            release(&bcache.lock);
            acquire(&b->lock);
            return b;
        }
    }

    // 没有可用的缓存块
    release(&bcache.lock);
    printf("bget: no buffers available\n");
    return 0;
}

// 返回指定块的锁定缓存
struct buf* bread(uint32_t dev, uint32_t blockno) {
    struct buf *b;

    b = bget(dev, blockno);
    if (b == 0) {
        return 0;
    }

    if (!b->valid) {
        disk_read(b);
        b->valid = 1;
    }

    return b;
}

// 将缓存块写入磁盘
void bwrite(struct buf *b) {
    if (!b->valid) {
        printf("bwrite: buffer not valid\n");
        return;
    }

    disk_write(b);
    b->disk = 0;  // 标记为已写入磁盘
}

// 释放缓存块
void brelse(struct buf *b) {
    if (b->refcnt == 0) {
        printf("brelse: buffer not locked\n");
        return;
    }

    release(&b->lock);

    acquire(&bcache.lock);
    b->refcnt--;
    if (b->refcnt == 0) {
        // 将块移动到链表头部（MRU位置）
        b->next->prev = b->prev;
        b->prev->next = b->next;
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
    release(&bcache.lock);
}

// 将所有脏缓存块写入磁盘
void bsync(void) {
    struct buf *b;

    acquire(&bcache.lock);
    for (b = bcache.head.next; b != &bcache.head; b = b->next) {
        if (b->valid && b->disk) {
            acquire(&b->lock);
            b->refcnt++;
            release(&bcache.lock);

            bwrite(b);

            acquire(&bcache.lock);
            b->refcnt--;
            release(&b->lock);
        }
    }
    release(&bcache.lock);

    printf("All dirty buffers synced to disk\n");
}