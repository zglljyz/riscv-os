#ifndef USER_H
#define USER_H

// 系统调用号定义
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_getpid  4
#define SYS_kill    5
#define SYS_read    6
#define SYS_write   7
#define SYS_open    8
#define SYS_close   9
#define SYS_setpriority 10
#define SYS_getpriority 11

// 系统调用包装函数
int fork(void);
int exit(int status) __attribute__((noreturn));
int wait(int *status);
int getpid(void);
int kill(int pid);
int read(int fd, void *buf, int n);
int write(int fd, const void *buf, int n);
int open(const char *path, int flags);
int close(int fd);
int setpriority(int pid, int priority);
int getpriority(int pid);

// 用户库函数
void printf(const char *fmt, ...);

#endif // USER_H
