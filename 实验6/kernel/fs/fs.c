#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "proc.h"
#include "string.h"

// 外部函数声明
void printf(const char *fmt, ...);

// 内部函数声明
static void format_fs(uint32_t dev);

//===========================================
// 实验七：文件系统初始化和主函数 (fs.c)
//===========================================

struct superblock sb;  // 全局超级块

// 初始化文件系统
void fs_init(uint32_t dev) {
    struct buf *bp;

    printf("Initializing file system on device %d...\n", dev);

    // 初始化块缓存系统
    binit();

    // 读取超级块
    bp = bread(dev, 1);  // 超级块通常在块1
    if (bp == 0) {
        printf("fs_init: cannot read superblock\n");
        return;
    }

    memcpy(&sb, bp->data, sizeof(sb));
    brelse(bp);

    // 检查文件系统魔数
    if (sb.magic != FS_MAGIC) {
        printf("fs_init: invalid file system magic number\n");
        // 创建新的文件系统
        format_fs(dev);
    } else {
        printf("fs_init: valid file system found\n");
    }

    printf("File system info:\n");
    printf("  Total blocks: %d\n", sb.size);
    printf("  Data blocks: %d\n", sb.nblocks);
    printf("  Inodes: %d\n", sb.ninodes);
    printf("  Log blocks: %d\n", sb.nlog);
    printf("  Log start: %d\n", sb.logstart);
    printf("  Inode start: %d\n", sb.inodestart);
    printf("  Bitmap start: %d\n", sb.bmapstart);

    // 初始化inode缓存
    iinit(dev, &sb);

    // 初始化日志系统
    loginit(dev, &sb);

    printf("File system initialization completed.\n");
}

// 格式化新的文件系统
static void format_fs(uint32_t dev) {
    struct buf *bp;
    struct dinode *dip;
    int i;

    printf("Formatting new file system...\n");

    // 设置超级块
    sb.magic = FS_MAGIC;
    sb.size = 1000;       // 总共1000个块
    sb.nblocks = 900;     // 900个数据块
    sb.ninodes = 200;     // 200个inode
    sb.nlog = 30;         // 30个日志块
    sb.logstart = 2;      // 日志从块2开始
    sb.inodestart = 32;   // inode从块32开始
    sb.bmapstart = 58;    // 位图从块58开始

    // 写入超级块
    bp = bread(dev, 1);
    if (bp == 0) {
        printf("format_fs: cannot read superblock\n");
        return;
    }
    memcpy(bp->data, &sb, sizeof(sb));
    log_write(bp);
    brelse(bp);

    // 清空inode区域
    for (i = sb.inodestart; i < sb.inodestart + (sb.ninodes * sizeof(struct dinode) + BLOCK_SIZE - 1) / BLOCK_SIZE; i++) {
        bp = bread(dev, i);
        if (bp == 0) continue;
        memset(bp->data, 0, BLOCK_SIZE);
        log_write(bp);
        brelse(bp);
    }

    // 初始化位图区域
    // 计算需要多少个块来存储位图（每个块可以管理 BLOCK_SIZE * 8 个数据块）
    int bitmap_blocks = (sb.size + BLOCK_SIZE * 8 - 1) / (BLOCK_SIZE * 8);
    printf("Initializing bitmap (%d blocks)...\n", bitmap_blocks);

    for (i = 0; i < bitmap_blocks; i++) {
        bp = bread(dev, sb.bmapstart + i);
        if (bp == 0) continue;

        memset(bp->data, 0, BLOCK_SIZE);

        // 标记已使用的块（超级块、日志、inode、位图本身）
        if (i == 0) {
            // 第一个位图块：标记系统块为已使用
            for (int j = 0; j < sb.bmapstart + bitmap_blocks; j++) {
                int byte_idx = j / 8;
                int bit_idx = j % 8;
                if (byte_idx < BLOCK_SIZE) {
                    bp->data[byte_idx] |= (1 << bit_idx);
                }
            }
        }

        log_write(bp);
        brelse(bp);
    }
    printf("Bitmap initialized.\n");

    // 创建根目录
    bp = bread(dev, sb.inodestart);
    if (bp == 0) {
        printf("format_fs: cannot read inode block\n");
        return;
    }

    dip = (struct dinode*)bp->data + ROOTINO;  // 根目录是inode 1
    dip->type = T_DIR;
    dip->major = 0;
    dip->minor = 0;
    dip->nlink = 1;
    dip->size = 0;
    log_write(bp);
    brelse(bp);

    printf("File system formatting completed.\n");
}

