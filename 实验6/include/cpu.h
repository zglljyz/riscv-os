#pragma once
// 包含 proc.h 以获取 struct proc 和 struct context 的完整定义
#include "proc.h"

#define NCPU 1      // 系统中的CPU核心数

// Per-CPU 状态
struct cpu {
  struct proc *proc;      // 当前在此CPU上运行的进程
  struct context context; // 调度器上下文
  int noff;               // 禁用中断的深度
  int intena;             // 在 push_off 之前的中断启用状态
};

// 声明全局的 cpus 数组
extern struct cpu cpus[NCPU];
