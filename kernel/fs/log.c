#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "proc.h"
#include "string.h"

// 外部函数声明
void printf(const char *fmt, ...);
extern struct superblock sb;

//===========================================
// 实验七：日志系统实现 (log.c)
//===========================================

struct log {
    struct spinlock lock;
    int start;              // 日志区在磁盘上的起始块号
    int size;               // 日志区大小（块数）
    int outstanding;        // 正在进行的文件系统调用数量
    int committing;         // 正在提交事务
    int dev;                // 设备号
    struct logheader lh;    // 内存中的日志头
};

struct log log;

// 内部函数声明
static void recover_from_log(void);
static void commit(void);
static void read_head(void);
static void write_head(void);
static void write_log(void);
static void install_trans(void);

// 初始化日志系统
void loginit(uint32_t dev, struct superblock *sb) {
    if (sizeof(struct logheader) >= BLOCK_SIZE) {
        printf("loginit: logheader too big\n");
        return;
    }

    initlock(&log.lock, "log");
    log.start = sb->logstart;
    log.size = sb->nlog;
    log.dev = dev;

    printf("Log initialized: start=%d, size=%d\n", log.start, log.size);

    recover_from_log();
}

// 从磁盘读取日志头
static void read_head(void) {
    struct buf *buf = bread(log.dev, log.start);
    if (buf == 0) {
        printf("read_head: bread failed\n");
        return;
    }

    struct logheader *lh = (struct logheader *) (buf->data);
    int i;
    log.lh.n = lh->n;
    for (i = 0; i < log.lh.n; i++) {
        log.lh.block[i] = lh->block[i];
    }
    brelse(buf);
}

// 将日志头写入磁盘
static void write_head(void) {
    struct buf *buf = bread(log.dev, log.start);
    if (buf == 0) {
        printf("write_head: bread failed\n");
        return;
    }

    struct logheader *hb = (struct logheader *) (buf->data);
    int i;
    hb->n = log.lh.n;
    for (i = 0; i < log.lh.n; i++) {
        hb->block[i] = log.lh.block[i];
    }
    bwrite(buf);
    brelse(buf);
}

// 从日志恢复文件系统
static void recover_from_log(void) {
    read_head();
    install_trans(); // 如果已提交，则从日志复制
    log.lh.n = 0;
    write_head(); // 清除日志
    printf("Log recovery completed\n");
}

// 开始一个文件系统操作
void begin_op(void) {
    acquire(&log.lock);
    while (1) {
        if (log.committing) {
            // 正在提交，等待
            sleep(&log, &log.lock);
        } else if (log.lh.n + (log.outstanding + 1) * MAXOPBLOCKS > LOGSIZE) {
            // 日志空间不足，等待
            sleep(&log, &log.lock);
        } else {
            log.outstanding += 1;
            release(&log.lock);
            break;
        }
    }
}

// 结束一个文件系统操作
void end_op(void) {
    int do_commit = 0;

    acquire(&log.lock);
    log.outstanding -= 1;

    if (log.committing) {
        printf("end_op: committing in progress\n");
        release(&log.lock);
        return;
    }

    if (log.outstanding == 0) {
        do_commit = 1;
        log.committing = 1;
    } else {
        // 唤醒等待开始新操作的进程
        wakeup(&log);
    }
    release(&log.lock);

    if (do_commit) {
        // 调用commit()而不持有log.lock,
        // 因为commit()内部会获取锁
        commit();
        acquire(&log.lock);
        log.committing = 0;
        wakeup(&log);
        release(&log.lock);
    }
}

// 将修改过的块添加到日志
void log_write(struct buf *b) {
    int i;

    if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1) {
        printf("log_write: too big a transaction\n");
        return;
    }

    if (log.outstanding < 1) {
        printf("log_write: outside of transaction\n");
        return;
    }

    acquire(&log.lock);

    for (i = 0; i < log.lh.n; i++) {
        if (log.lh.block[i] == b->blockno) // 已存在于日志中
            break;
    }

    log.lh.block[i] = b->blockno;
    if (i == log.lh.n) {
        log.lh.n++;
    }

    b->disk = 1; // 标记缓存块为脏（需要写入）
    release(&log.lock);

    printf("log_write: logged block %d (entry %d)\n", b->blockno, i);
}

// 提交当前事务
static void commit() {
    if (log.lh.n > 0) {
        write_log();     // 将修改的块从缓存写入日志
        write_head();    // 将头块写入磁盘 -- 真正的提交点
        install_trans(); // 现在将修改的块从日志写入它们在磁盘上的位置
        log.lh.n = 0;
        write_head();    // 通过将n设置为0来清除事务
        printf("Log commit completed\n");
    }
}

// 将修改的块从缓存复制到日志
static void write_log(void) {
    int tail;

    for (tail = 0; tail < log.lh.n; tail++) {
        struct buf *to = bread(log.dev, log.start + tail + 1); // 日志块
        struct buf *from = bread(log.dev, log.lh.block[tail]); // 缓存块

        if (to == 0 || from == 0) {
            printf("write_log: bread failed\n");
            if (to) brelse(to);
            if (from) brelse(from);
            continue;
        }

        memcpy(to->data, from->data, BLOCK_SIZE);
        bwrite(to);  // 写入日志块
        brelse(from);
        brelse(to);
    }

    printf("write_log: wrote %d blocks to log\n", tail);
}

// 将日志中的块安装到它们在文件系统中的位置
static void install_trans(void) {
    int tail;

    for (tail = 0; tail < log.lh.n; tail++) {
        struct buf *lbuf = bread(log.dev, log.start + tail + 1); // 从日志读取
        struct buf *dbuf = bread(log.dev, log.lh.block[tail]);   // 从文件系统读取

        if (lbuf == 0 || dbuf == 0) {
            printf("install_trans: bread failed\n");
            if (lbuf) brelse(lbuf);
            if (dbuf) brelse(dbuf);
            continue;
        }

        memcpy(dbuf->data, lbuf->data, BLOCK_SIZE);
        bwrite(dbuf);  // 写入文件系统位置
        brelse(lbuf);
        brelse(dbuf);
    }

    if (tail > 0) {
        printf("install_trans: installed %d blocks from log\n", tail);
    }
}