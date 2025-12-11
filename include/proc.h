
#pragma once

#include "types.h"
#include "riscv.h"
#include "vm.h"
#include "trap.h" 
// 前向声明，打破循环依赖
struct cpu;
struct spinlock; // 自旋锁结构体
#define NPROC 64    // 系统支持的最大进程数

// 进程状态枚举
enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// 进程上下文 (由 swtch.S 保存/恢复)
struct context {
  uint64_t ra;
  uint64_t sp;
  uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
};

// 进程优先级常量定义
#define PRIORITY_HIGH     0  // 高优先级
#define PRIORITY_NORMAL   1  // 普通优先级
#define PRIORITY_LOW      2  // 低优先级
#define PRIORITY_IDLE     3  // 空闲优先级
#define MAX_PRIORITY      3  // 最大优先级值

// Aging 机制常量
#define AGING_THRESHOLD   50    // aging阈值（时间单位） - 降低阈值便于演示
#define AGING_INTERVAL    10    // aging检查间隔 - 增加检查频率

// 进程控制块 (PCB)
struct proc {
  enum procstate state;
  int pid;
  struct proc *parent;
  uint64_t kstack;
  pagetable_t pagetable;
  struct trapframe *trapframe;
  struct context context;
  void *chan;
  int killed;
  int xstate;
  uint64_t sz;
  char name[16];

  // 实验五：添加优先级调度相关字段
  int priority;           // 进程优先级 (0=高优先级, 3=低优先级)
  int base_priority;      // 基础优先级（不会因aging改变）
  uint64_t time_slice;    // 时间片大小
  uint64_t cpu_time;      // 已使用的CPU时间
  uint64_t last_run;      // 上次运行时间
  uint64_t wait_start;    // 开始等待的时间
  uint64_t total_wait;    // 累计等待时间
};

void proc_init(void);
void scheduler(void) __attribute__((noreturn));
void yield(void);
void sleep(void *chan, struct spinlock *lk);
void wakeup(void *chan);
int fork(void);
void exit(int status);
int wait(uint64_t addr);
struct proc* allocproc(void);
struct cpu* mycpu(void);
struct proc* myproc(void);
int create_kthread(void (*func)(void *), void *arg);

// 实验五：优先级调度相关函数
void set_priority(struct proc *p, int priority);
struct proc* find_highest_priority_proc(void);
void update_time_slice(struct proc *p);
uint64_t get_time(void);  // 获取当前时间的函数声明

// Aging 机制相关函数
void apply_aging(void);
void reset_priority_to_base(struct proc *p);
void update_wait_time(struct proc *p);

// 互斥自旋锁结构体
struct spinlock {
    unsigned int locked;         // 锁的状态：0表示未锁定, 1表示锁定
    char *name;        // 锁的名称 
    struct cpu *cpu;   // 持有该锁的CPU
};

void initlock(struct spinlock *lk, char *name);
void acquire(struct spinlock *lk);
void release(struct spinlock *lk);
extern struct spinlock ptable_lock;
