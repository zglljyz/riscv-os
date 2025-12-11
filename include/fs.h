#pragma once

#include "types.h"
#include "proc.h"  // 包含 proc.h 以获取 spinlock 定义

//===========================================
// 实验七：文件系统数据结构和常量定义
//===========================================

#define BLOCK_SIZE    4096    // 块大小 4KB
#define MAX_NAME_LEN  14      // 最大文件名长度
#define MAX_PATH_LEN  256     // 最大路径长度
#define DIRECT_BLOCKS 12      // 直接块数量
#define MAX_OPEN_FILES 16     // 最大打开文件数
#define MAXFILE (DIRECT_BLOCKS + BLOCK_SIZE/sizeof(uint32_t))  // 最大文件块数
#define ROOTDEV 1             // 根文件系统设备号
#define ROOTINO 1             // 根目录inode号
#define MAXOPBLOCKS 10        // 每个操作的最大块数
#define LOGSIZE (MAXOPBLOCKS*3)  // 日志大小

// 文件系统魔数
#define FS_MAGIC      0x12345678

// 位图操作宏
// BBLOCK: 给定块号 b，返回包含该块位图的块号
#define BBLOCK(b, sb) ((b) / (BLOCK_SIZE * 8) + (sb).bmapstart)

// 超级块结构
struct superblock {
    uint32_t magic;           // 文件系统魔数
    uint32_t size;            // 文件系统总块数
    uint32_t nblocks;         // 数据块数量
    uint32_t ninodes;         // inode数量
    uint32_t nlog;            // 日志块数量
    uint32_t logstart;        // 日志区起始块号
    uint32_t inodestart;      // inode区起始块号
    uint32_t bmapstart;       // 位图起始块号
    uint32_t reserved[24];    // 预留空间，保持超级块为一个完整块
};

// 文件类型
#define T_DIR     1   // 目录
#define T_FILE    2   // 普通文件
#define T_DEVICE  3   // 设备文件

// inode结构（磁盘上的格式）
struct dinode {
    uint16_t type;            // 文件类型
    uint16_t major;           // 主设备号
    uint16_t minor;           // 次设备号
    uint16_t nlink;           // 链接数
    uint32_t size;            // 文件大小（字节）
    uint32_t addrs[DIRECT_BLOCKS + 1];  // 数据块地址（最后一个是间接块）
};

// 内存中的inode
struct inode {
    uint32_t dev;             // 设备号
    uint32_t inum;            // inode号
    int ref;                  // 引用计数
    int valid;                // 是否从磁盘加载
    struct spinlock lock;     // inode锁

    // 从磁盘inode复制的内容
    uint16_t type;
    uint16_t major;
    uint16_t minor;
    uint16_t nlink;
    uint32_t size;
    uint32_t addrs[DIRECT_BLOCKS + 1];
};

// 目录项结构
struct dirent {
    uint16_t inum;            // inode号，0表示空闲
    char name[MAX_NAME_LEN];  // 文件名
};

// 日志头结构
struct logheader {
    int n;                    // 日志中的块数
    int block[30];            // 每个块在文件系统中的位置
};

// 文件描述符
struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref;                  // 引用计数
    char readable;
    char writable;
    struct inode *ip;         // inode指针
    uint32_t off;             // 文件偏移量
};

// 块缓存结构 - 定义在这里供其他结构使用
struct buf {
    int valid;                // 缓存是否有效
    int disk;                 // 是否需要写回磁盘
    uint32_t dev;             // 设备号
    uint32_t blockno;         // 块号
    uint32_t refcnt;          // 引用计数
    struct buf *prev, *next;  // LRU链表
    struct spinlock lock;     // 缓存块锁
    unsigned char data[BLOCK_SIZE];  // 实际数据
};

// 文件状态结构
struct stat {
    uint32_t dev;     // 设备号
    uint32_t ino;     // inode号
    uint16_t type;    // 文件类型
    uint16_t nlink;   // 链接数
    uint32_t size;    // 文件大小
};

// 文件系统函数声明
void fs_init(uint32_t dev);
void test_filesystem(void);
void test_concurrent_access(void);
void test_crash_recovery(void);
void iinit(uint32_t dev, struct superblock *sb_ptr);
struct inode* ialloc(uint32_t dev, short type);
struct inode* iget(uint32_t dev, uint32_t inum);
void iput(struct inode *ip);
void ilock(struct inode *ip);
void iunlock(struct inode *ip);
struct inode* idup(struct inode *ip);
int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n);
int writei(struct inode *ip, char *src, uint32_t off, uint32_t n);
void iwrite(struct inode *ip);
void itrunc(struct inode *ip);
uint32_t balloc(uint32_t dev);
void bfree(uint32_t dev, uint32_t b);
struct inode* namei(char *path);
struct inode* nameiparent(char *path, char *name);
int dirlink(struct inode *dp, char *name, uint32_t inum);
int dirlookup(struct inode *dp, char *name, uint32_t *poff);

// 目录操作函数
struct inode* create(char *path, short type, short major, short minor);
int sys_create(char *path);
int sys_mkdir(char *path);
int sys_unlink(char *path);

// 内部辅助函数
struct inode* namex(char *path, int nameiparent, char *name);
char* skipelem(char *path, char *name);
int isdirempty(struct inode *dp);

// 文件操作函数
struct file* filealloc(void);
void fileclose(struct file *f);
struct file* filedup(struct file *f);
int fileread(struct file *f, char *addr, int n);
int filestat(struct file *f, struct stat *st);
int filewrite(struct file *f, char *addr, int n);

// 日志系统函数
void loginit(uint32_t dev, struct superblock *sb);
void log_write(struct buf *b);
void begin_op(void);
void end_op(void);

// 块缓存系统函数
void binit(void);
void bsync(void);
struct buf* bread(uint32_t dev, uint32_t blockno);
void bwrite(struct buf *b);
void brelse(struct buf *b);