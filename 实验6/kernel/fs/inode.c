#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "proc.h"
#include "string.h"

// 外部函数声明
void printf(const char *fmt, ...);
extern struct superblock sb;

//===========================================
// 实验七：inode操作实现 (inode.c)
//===========================================

static struct {
    struct spinlock lock;
    struct inode inode[50];  // 内存中的inode缓存
} icache;

// 初始化inode缓存
void iinit(uint32_t dev, struct superblock *sb_ptr) {
    initlock(&icache.lock, "icache");

    for (int i = 0; i < 50; i++) {
        icache.inode[i].ref = 0;
        icache.inode[i].valid = 0;
    }

    printf("inode cache initialized\n");
}

// 从磁盘读取inode
static void iread(struct inode *ip) {
    struct buf *bp;
    struct dinode *dip;

    // 计算inode在磁盘上的位置
    uint32_t block = sb.inodestart + ip->inum / (BLOCK_SIZE / sizeof(struct dinode));
    uint32_t offset = ip->inum % (BLOCK_SIZE / sizeof(struct dinode));

    bp = bread(ip->dev, block);
    if (bp == 0) {
        printf("iread: bread failed\n");
        return;
    }

    dip = (struct dinode*)bp->data + offset;

    // 复制磁盘inode到内存inode
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    memcpy(ip->addrs, dip->addrs, sizeof(ip->addrs));

    brelse(bp);
    ip->valid = 1;

    printf("iread: loaded inode %d, type=%d, size=%d\n", ip->inum, ip->type, ip->size);
}

// 将内存inode写入磁盘
void iwrite(struct inode *ip) {
    struct buf *bp;
    struct dinode *dip;

    // 计算inode在磁盘上的位置
    uint32_t block = sb.inodestart + ip->inum / (BLOCK_SIZE / sizeof(struct dinode));
    uint32_t offset = ip->inum % (BLOCK_SIZE / sizeof(struct dinode));

    bp = bread(ip->dev, block);
    if (bp == 0) {
        printf("iwrite: bread failed\n");
        return;
    }

    dip = (struct dinode*)bp->data + offset;

    // 复制内存inode到磁盘inode
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;
    memcpy(dip->addrs, ip->addrs, sizeof(dip->addrs));

    log_write(bp);  // 通过日志系统写入
    brelse(bp);

    printf("iwrite: saved inode %d to disk\n", ip->inum);
}

// 分配一个新的inode
struct inode* ialloc(uint32_t dev, short type) {
    struct buf *bp;
    struct dinode *dip;

    for (uint32_t inum = 1; inum < sb.ninodes; inum++) {
        uint32_t block = sb.inodestart + inum / (BLOCK_SIZE / sizeof(struct dinode));
        uint32_t offset = inum % (BLOCK_SIZE / sizeof(struct dinode));

        bp = bread(dev, block);
        if (bp == 0) continue;

        dip = (struct dinode*)bp->data + offset;

        if (dip->type == 0) {  // 空闲inode
            memset(dip, 0, sizeof(*dip));
            dip->type = type;
            log_write(bp);
            brelse(bp);

            printf("ialloc: allocated inode %d, type=%d\n", inum, type);
            return iget(dev, inum);
        }
        brelse(bp);
    }

    printf("ialloc: no free inodes\n");
    return 0;
}

// 获取内存中的inode，如果不存在则从磁盘加载
struct inode* iget(uint32_t dev, uint32_t inum) {
    struct inode *ip, *empty;

    acquire(&icache.lock);

    // 在缓存中查找
    empty = 0;
    for (ip = &icache.inode[0]; ip < &icache.inode[50]; ip++) {
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            release(&icache.lock);
            return ip;  // 返回时不持有锁
        }
        if (empty == 0 && ip->ref == 0) {  // 记住第一个空闲槽
            empty = ip;
        }
    }

    // 未在缓存中找到，分配新的缓存项
    if (empty == 0) {
        release(&icache.lock);
        printf("iget: no free cache entries\n");
        return 0;
    }

    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->ref = 1;
    ip->valid = 0;
    initlock(&ip->lock, "inode");

    release(&icache.lock);
    return ip;  // 返回时不持有锁
}

// 增加inode引用计数
struct inode* idup(struct inode *ip) {
    acquire(&icache.lock);
    ip->ref++;
    release(&icache.lock);
    return ip;
}

// 减少inode引用计数
void iput(struct inode *ip) {
    acquire(&icache.lock);

    if (ip->ref == 1 && ip->valid && ip->nlink == 0) {
        // 这是最后一个引用且文件已删除，需要释放inode
        acquire(&ip->lock);
        release(&icache.lock);

        itrunc(ip);  // 释放所有数据块
        ip->type = 0;
        iwrite(ip);
        ip->valid = 0;

        release(&ip->lock);
        acquire(&icache.lock);
    }

    ip->ref--;
    release(&icache.lock);
}

// 锁定inode
void ilock(struct inode *ip) {
    if (ip == 0) return;

    acquire(&ip->lock);
    if (!ip->valid) {
        iread(ip);
    }
}

// 解锁inode
void iunlock(struct inode *ip) {
    if (ip == 0) return;
    release(&ip->lock);
}

// 释放inode的所有数据块
void itrunc(struct inode *ip) {
    printf("itrunc: truncating inode %d\n", ip->inum);

    // 释放直接块
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (ip->addrs[i]) {
            bfree(ip->dev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    // 释放间接块
    if (ip->addrs[DIRECT_BLOCKS]) {
        struct buf *bp = bread(ip->dev, ip->addrs[DIRECT_BLOCKS]);
        if (bp) {
            uint32_t *a = (uint32_t*)bp->data;
            for (int i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
                if (a[i]) {
                    bfree(ip->dev, a[i]);
                }
            }
            brelse(bp);
            bfree(ip->dev, ip->addrs[DIRECT_BLOCKS]);
            ip->addrs[DIRECT_BLOCKS] = 0;
        }
    }

    ip->size = 0;
    iwrite(ip);
}

// 获取inode的第n个数据块地址
static uint32_t bmap(struct inode *ip, uint32_t bn) {
    uint32_t addr, *a;
    struct buf *bp;

    if (bn < DIRECT_BLOCKS) {
        if ((addr = ip->addrs[bn]) == 0) {
            ip->addrs[bn] = addr = balloc(ip->dev);
        }
        return addr;
    }
    bn -= DIRECT_BLOCKS;

    if (bn < BLOCK_SIZE / sizeof(uint32_t)) {
        // 间接块
        if ((addr = ip->addrs[DIRECT_BLOCKS]) == 0) {
            ip->addrs[DIRECT_BLOCKS] = addr = balloc(ip->dev);
        }
        bp = bread(ip->dev, addr);
        if (bp == 0) return 0;

        a = (uint32_t*)bp->data;
        if ((addr = a[bn]) == 0) {
            a[bn] = addr = balloc(ip->dev);
            log_write(bp);
        }
        brelse(bp);
        return addr;
    }

    printf("bmap: block number too large\n");
    return 0;
}

// 从inode读取数据
int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n) {
    uint32_t tot, m;
    struct buf *bp;

    if (off > ip->size || off + n < off) {
        return -1;
    }
    if (off + n > ip->size) {
        n = ip->size - off;
    }

    for (tot = 0; tot < n; tot += m, off += m, dst += m) {
        bp = bread(ip->dev, bmap(ip, off / BLOCK_SIZE));
        if (bp == 0) break;

        m = BLOCK_SIZE - off % BLOCK_SIZE;
        if (n - tot < m) {
            m = n - tot;
        }

        memcpy(dst, bp->data + (off % BLOCK_SIZE), m);
        brelse(bp);
    }

    return tot;
}

// 向inode写入数据
int writei(struct inode *ip, char *src, uint32_t off, uint32_t n) {
    uint32_t tot, m;
    struct buf *bp;

    if (off > ip->size || off + n < off) {
        return -1;
    }
    if (off + n > MAXFILE * BLOCK_SIZE) {
        return -1;
    }

    for (tot = 0; tot < n; tot += m, off += m, src += m) {
        bp = bread(ip->dev, bmap(ip, off / BLOCK_SIZE));
        if (bp == 0) break;

        m = BLOCK_SIZE - off % BLOCK_SIZE;
        if (n - tot < m) {
            m = n - tot;
        }

        memcpy(bp->data + (off % BLOCK_SIZE), src, m);
        log_write(bp);
        brelse(bp);
    }

    if (n > 0 && off > ip->size) {
        ip->size = off;
        iwrite(ip);
    }

    return tot;
}

// 从位图中分配一个数据块
uint32_t balloc(uint32_t dev) {
    struct buf *bp;
    int bi, m;
    uint32_t b;

    // 遍历所有数据块
    for (b = 0; b < sb.size; b += BLOCK_SIZE * 8) {
        // 读取包含该块位图的块
        bp = bread(dev, BBLOCK(b, sb));
        if (bp == 0) {
            continue;
        }

        // 检查这个位图块中的每一位
        for (bi = 0; bi < BLOCK_SIZE * 8 && b + bi < sb.size; bi++) {
            m = 1 << (bi % 8);
            // 如果该位为 0，表示块空闲
            if ((bp->data[bi / 8] & m) == 0) {
                // 标记为已使用
                bp->data[bi / 8] |= m;
                log_write(bp);
                brelse(bp);

                // 清零这个新分配的块
                struct buf *dbp = bread(dev, b + bi);
                if (dbp) {
                    memset(dbp->data, 0, BLOCK_SIZE);
                    log_write(dbp);
                    brelse(dbp);
                }

                printf("balloc: allocated block %d\n", b + bi);
                return b + bi;
            }
        }
        brelse(bp);
    }

    printf("balloc: out of blocks\n");
    return 0;
}

// 释放一个数据块到位图
void bfree(uint32_t dev, uint32_t b) {
    struct buf *bp;
    int bi, m;

    // 读取包含该块位图的块
    bp = bread(dev, BBLOCK(b, sb));
    if (bp == 0) {
        printf("bfree: cannot read bitmap block\n");
        return;
    }

    bi = b % (BLOCK_SIZE * 8);
    m = 1 << (bi % 8);

    // 检查该块是否确实已分配
    if ((bp->data[bi / 8] & m) == 0) {
        printf("bfree: freeing free block %d\n", b);
    }

    // 清除位图中的位，标记为空闲
    bp->data[bi / 8] &= ~m;
    log_write(bp);
    brelse(bp);

    printf("bfree: freed block %d\n", b);
}