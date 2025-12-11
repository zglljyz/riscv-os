#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "vm.h"
#include "string.h"
#include "cpu.h"

// 外部函数声明
void printf(const char *fmt, ...);
void *alloc_page(void);
void free_page(void *pa);
void swtch(struct context *, struct context *);
pagetable_t uvmcreate();
int uvmcopy(pagetable_t, pagetable_t, uint64_t);
void uvmfree(pagetable_t, uint64_t);
void usertrapret(void);
static void freeproc(struct proc *p);
void forkret(void);

// 全局变量
struct proc procs[NPROC];
struct cpu cpus[NCPU]; 
static int nextpid = 1;

//全局进程表锁，用于保护 procs 数组和相关状态
struct spinlock ptable_lock;


// spinlock实现
// 启用中断
static void intr_on() {
    w_sstatus(r_sstatus() | SSTATUS_SIE);
}

// 禁用中断
static void intr_off() {
    w_sstatus(r_sstatus() & ~SSTATUS_SIE);
}

// 禁用中断，并保存之前的状态
static void push_off(void) {
    int old = r_sstatus();
    intr_off();
    if (mycpu()->noff == 0)
        mycpu()->intena = old & SSTATUS_SIE;
    mycpu()->noff += 1;
}

// 恢复之前的中断状态
static void pop_off(void) {
    struct cpu *c = mycpu();
    if (r_sstatus() & SSTATUS_SIE)
        while(1); // panic:中断不应被启用
    if (c->noff < 1)
        while(1); // panic:嵌套计数错误
    c->noff -= 1;
    if (c->noff == 0 && c->intena)
        intr_on();
}

// 初始化锁
void initlock(struct spinlock *lk, char *name) {
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

// 获取锁
void acquire(struct spinlock *lk) {
    push_off(); // 获取锁前禁用中断，防止死锁
    while (__atomic_test_and_set(&lk->locked, __ATOMIC_ACQUIRE));
    lk->cpu = mycpu();
}

// 释放锁
void release(struct spinlock *lk) {
    lk->cpu = 0;
    __atomic_clear(&lk->locked, __ATOMIC_RELEASE);
    pop_off(); // 释放锁后恢复中断状态
}


//CPU 和进程初始化函数

//初始化CPU结构体
void cpu_init(void) {
    for (int i = 0; i < NCPU; i++) {
        cpus[i].noff = 0;
        cpus[i].intena = 0;
    }
}

// 初始化进程表和全局锁
void proc_init(void) {
    initlock(&ptable_lock, "ptable");
    for (int i = 0; i < NPROC; i++) {
        procs[i].state = UNUSED;
    }
}

// 获取当前CPU的指针
struct cpu* mycpu(void) {
    return &cpus[0]; 
}

// 获取当前CPU上运行进程的指针
struct proc* myproc(void) {
    struct cpu *c = mycpu();
    return c->proc;
}


//核心进程管理函数
// 释放一个进程的资源
static void freeproc(struct proc *p) {
    // 调用者必须持有 ptable_lock
    if(p->trapframe) free_page((void*)p->trapframe);
    if(p->kstack) free_page((void*)p->kstack);
    p->trapframe = 0;
    p->kstack = 0;

    if(p->pagetable) uvmfree(p->pagetable, p->sz);
    p->pagetable = 0;
    p->sz = 0;
    
    p->pid = 0;
    p->parent = 0;
    p->name[0] = 0;
    p->chan = 0;
    p->killed = 0;
    p->xstate = 0;
    p->state = UNUSED;
}

// 分配一个新进程
struct proc* allocproc(void) {
    struct proc *p;

    acquire(&ptable_lock); // 获取进程表锁

    for (p = procs; p < &procs[NPROC]; p++) {
        if (p->state == UNUSED) {
            goto found;
        }
    }
    release(&ptable_lock);
    return 0;

found:
    p->pid = nextpid++;
    p->state = USED;
    release(&ptable_lock); // 释放锁，允许其他操作

    // 分配内核栈和陷阱帧 (这些操作可能会休眠，不应在持锁时进行)
    if ((p->kstack = (uint64_t)alloc_page()) == 0) {
        // 分配失败，需要重新获取锁来清理
        acquire(&ptable_lock);
        freeproc(p);
        release(&ptable_lock);
        return 0;
    }

    if ((p->trapframe = (struct trapframe *)alloc_page()) == 0) {
        acquire(&ptable_lock);
        freeproc(p);
        release(&ptable_lock);
        return 0;
    }

    // 初始化上下文，使其返回到 forkret
    memset(&p->context, 0, sizeof(p->context));
    p->context.ra = (uint64_t)forkret;
    p->context.sp = p->kstack + PGSIZE;

    // 实验五：初始化优先级调度相关字段
    p->priority = PRIORITY_NORMAL;  // 默认普通优先级
    p->base_priority = PRIORITY_NORMAL;  // 初始化基础优先级
    p->time_slice = 100;           // 默认时间片 100ms (假设单位)
    p->cpu_time = 0;               // 初始化CPU使用时间
    p->last_run = 0;               // 初始化上次运行时间
    p->wait_start = 0;             // 初始化等待开始时间
    p->total_wait = 0;             // 初始化总等待时间

    return p;
}



//调度与同步函数
// 用户进程首次进入内核的返回点
void forkret(void) {
    struct proc *p = myproc();

    // 首次进入用户空间前，需要释放调度器持有的锁
    release(&ptable_lock);
    // printf("[forkret] Released ptable_lock\n");

    // 启用中断
    intr_on();
    // printf("[forkret] Enabled interrupts\n");

    // xv6方式: 调用prepare_return后，通过函数指针调用userret
    extern char trampoline[];
    extern char userret[];

    // printf("[forkret] Got process pid=%d\n", p->pid);

    // 准备返回用户模式 - 按xv6的prepare_return()方式设置
    intr_off();
    // printf("[forkret] Disabled interrupts\n");

    // 设置stvec指向uservec
    extern char uservec[];
    uint64_t uservec_va = TRAMPOLINE + ((uint64_t)uservec - (uint64_t)trampoline);
    w_stvec(uservec_va);
    // printf("[forkret] Set stvec to 0x%lx\n", uservec_va);

    // 填充trapframe的kernel字段
    p->trapframe->kernel_satp = r_satp();  // 使用当前的satp!
    p->trapframe->kernel_sp = p->kstack + 4096;
    extern void usertrap();
    p->trapframe->kernel_trap = (uint64_t)usertrap;
    p->trapframe->kernel_hartid = 0;
    // printf("[forkret] Filled trapframe kernel fields, kernel_satp=0x%lx\n", p->trapframe->kernel_satp);

    // 设置sstatus和sepc
    uint64_t x = r_sstatus();
    x &= ~SSTATUS_SPP;  // 清除SPP (进入U-mode)
    x |= SSTATUS_SPIE;  // 设置SPIE (U-mode中断使能)
    x |= SSTATUS_SUM;   // 设置SUM - 允许S-mode访问U-mode页面（对于trampoline是必须的！）
    w_sstatus(x);
    w_sepc(p->trapframe->epc);
    // printf("[forkret] Set sstatus=0x%lx (with SUM), sepc=0x%lx\n", x, p->trapframe->epc);

    // 计算userret的虚拟地址
    uint64_t userret_va = TRAMPOLINE + ((uint64_t)userret - (uint64_t)trampoline);
    // printf("[forkret] userret physical: 0x%lx, virtual: 0x%lx\n", (uint64_t)userret, userret_va);

    // 准备user_satp
    uint64_t user_satp = MAKE_SATP(p->pagetable);
    // printf("[forkret] user_satp=0x%lx, pagetable=0x%lx\n", user_satp, (uint64_t)p->pagetable);

    // printf("[forkret] About to call userret at VA 0x%lx with satp=0x%lx\n", userret_va, user_satp);

    // 在调用前验证映射
    // printf("[forkret] Current satp=0x%lx\n", r_satp());

    // DEBUG: 验证userret_va是否可访问
    extern pte_t *walk(pagetable_t, uint64_t, int);
    pagetable_t kpt = (pagetable_t)((r_satp() & 0xFFFFFFFFFFF) << 12);
    pte_t *pte = walk(kpt, userret_va, 0);
    if (pte == 0 || (*pte & PTE_V) == 0) {
        // printf("[forkret] ERROR: userret_va 0x%lx is not mapped in kernel page table!\n", userret_va);
        while(1);
    }
    // printf("[forkret] userret_va is mapped, PTE=0x%lx\n", *pte);

    // DEBUG: 检查用户页表中TRAMPOLINE的映射
    pagetable_t upt = p->pagetable;
    pte_t *upte = walk(upt, userret_va, 0);
    if (upte == 0 || (*upte & PTE_V) == 0) {
        // printf("[forkret] ERROR: userret_va not mapped in USER page table!\n");
        while(1);
    }
    // printf("[forkret] userret_va in USER PT: PTE=0x%lx\n", *upte);

    // 比较物理地址
    uint64_t kernel_pa = ((*pte) >> 10) << 12;
    uint64_t user_pa = ((*upte) >> 10) << 12;
    // printf("[forkret] Kernel PA: 0x%lx, User PA: 0x%lx\n", kernel_pa, user_pa);
    if (kernel_pa != user_pa) {
        // printf("[forkret] ERROR: Physical addresses don't match!\n");
        while(1);
    }

    // DEBUG: 尝试读取userret_va处的指令
    // printf("[forkret] Attempting to read instruction at userret_va...\n");
    uint32_t *instr_ptr = (uint32_t*)userret_va;
    uint32_t first_instr = *instr_ptr;
    // printf("[forkret] First instruction at userret_va: 0x%x\n", first_instr);

    // printf("[forkret] Calling userret now...\n");

    // 在跳转前添加更多调试信息
    // printf("[forkret] About to jump to 0x%lx\n", userret_va);
    // printf("[forkret] Current PC should be around 0x%lx\n", (uint64_t)&&debug_label);
    debug_label:

    // xv6方式: 使用虚拟地址跳转到 userret，这是一个不会返回的函数
    // userret会切换页表、恢复用户寄存器，然后sret到用户模式
    // 重要：userret永远不会返回，所以我们使用内联汇编并标记为unreachable

    // 添加一个fence指令确保前面的所有内存操作完成
    asm volatile("fence" ::: "memory");

    asm volatile(
        "mv a0, %0\n"       // 将user_satp加载到a0
        "jr %1\n"           // 跳转到userret_va (不保存返回地址)
        :
        : "r"(user_satp), "r"(userret_va)
        : "a0"
    );

    // 永远不会到达这里，但添加__builtin_unreachable()告诉编译器
    __builtin_unreachable();
}

// 切换到调度器，不返回
// 调用者必须持有 ptable_lock
void sched(void) {
    struct cpu *c = mycpu();
    struct proc *p = myproc();
    
    // 保存上下文并切换到调度器
    swtch(&p->context, &c->context);
}

// 实验五：优先级调度器（替代原来的轮转调度器）
void scheduler(void) {
    struct proc *p;
    struct cpu *c = mycpu();
    static uint64_t aging_counter = 0;  // 用于定期执行aging

    c->proc = 0;
    for (;;) {
        intr_on(); // 在寻找进程前启用中断

        acquire(&ptable_lock); // 获取进程表锁

        // 定期执行aging机制
        aging_counter++;
        if (aging_counter % AGING_INTERVAL == 0) {
            apply_aging();
        }

        // 更新所有进程的等待时间
        for (struct proc *proc = procs; proc < &procs[NPROC]; proc++) {
            if (proc->state != UNUSED) {
                update_wait_time(proc);
            }
        }

        // 查找最高优先级的可运行进程
        p = find_highest_priority_proc();

        if (p != 0) {
            // 找到了可运行的进程
            // printf("[scheduler] Found process %d, switching to it\n", p->pid);
            p->state = RUNNING;
            c->proc = p;

            // 更新进程运行信息
            p->last_run = get_time();

            // 当进程开始运行时，重置优先级为基础优先级（如果被aging提升过）
            if (p->priority < p->base_priority) {
                reset_priority_to_base(p);
            }

            // printf("Scheduling process %d (priority %d)\n", p->pid, p->priority);

            // 切换到进程 p, swtch后锁会传给p，p调用sched后会传回
            // printf("[scheduler] About to call swtch() to process %d\n", p->pid);
            swtch(&c->context, &p->context);
            // printf("[scheduler] Returned from swtch() from process %d\n", p->pid);

            // 进程已经切换回来，更新其时间信息
            update_time_slice(p);

            c->proc = 0;
        } else {
            // 没有找到可运行的进程
            static int debug_count = 0;
            if (debug_count < 5) {
                // printf("[scheduler] No RUNNABLE process found\n");
                debug_count++;
            }
        }

        release(&ptable_lock); // 如果没有可运行的进程，释放锁并重试
    }
}

// 进程主动放弃CPU
void yield(void) {
    struct proc *p = myproc();
    acquire(&ptable_lock);
    p->state = RUNNABLE;
    p->wait_start = get_time();  // 设置等待开始时间
    sched();
    release(&ptable_lock);
}

// 睡眠
// 调用者必须持有 lk。lk 通常是 ptable_lock。
void sleep(void *chan, struct spinlock *lk) {
    struct proc *p = myproc();

    // 必须持有锁，并且锁不能为 NULL
    if(lk == 0) while(1);

    // 如果传入的锁不是 ptable_lock，需要先获取 ptable_lock
    if(lk != &ptable_lock) {
        acquire(&ptable_lock);
        release(lk);
    }

    // 原子地将进程置为睡眠状态并切换
    p->chan = chan;
    p->state = SLEEPING;

    // 切换到调度器，sched() 不会释放 ptable_lock
    sched();

    // 醒来后，清除通道
    p->chan = 0;

    // 如果传入的锁不是 ptable_lock，重新获取原来的锁
    if(lk != &ptable_lock) {
        release(&ptable_lock);
        acquire(lk);
    }
}

// 唤醒在特定通道上睡眠的所有进程
// 调用者必须持有 ptable_lock
void wakeup(void *chan) {
    struct proc *p;
    for (p = procs; p < &procs[NPROC]; p++) {
        if (p->state == SLEEPING && p->chan == chan) {
            p->state = RUNNABLE;
        }
    }
}


// 系统调用实现
// 进程退出
void exit(int status) {
    struct proc *p = myproc();

    // ... 在这里可以添加关闭文件等清理操作 ...

    acquire(&ptable_lock);

    // 唤醒父进程
    wakeup(p->parent);

    p->xstate = status;
    p->state = ZOMBIE;
    
    // 跳转到调度器，永不返回。sched()不释放锁。
    sched();

    // 这一行永远不会被执行
    while(1); // panic
}

// 等待子进程退出并收集其状态
int wait(uint64_t addr) {
    struct proc *p = myproc();
    struct proc *child;
    int have_kids, pid;

    acquire(&ptable_lock); // 获取全局进程表锁

    for (;;) {
        have_kids = 0;
        for (child = procs; child < &procs[NPROC]; child++) {
            if (child->parent == p) {
                have_kids = 1;
                if (child->state == ZOMBIE) {
                    pid = child->pid;
                    // TODO: copyout status to user space
                    freeproc(child); // freeproc 必须在持锁时调用
                    release(&ptable_lock); // 释放锁并返回
                    return pid;
                }
            }
        }

        // 如果没有子进程或者本进程被杀死，则返回错误
        if (!have_kids || p->killed) {
            release(&ptable_lock);
            return -1;
        }

        // 等待子进程退出。sleep 会原子地释放 ptable_lock
        sleep(p, &ptable_lock);
    }
}

// 创建一个子进程
int fork(void) {
    struct proc *np;
    struct proc *p = myproc();

    // 检查父进程是否是内核线程
    if (p->pagetable == 0) {
        return -1;
    }

    // 分配新进程
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // 为子进程创建用户页表和内存
    if ((np->pagetable = uvmcreate()) == 0) {
        acquire(&ptable_lock);
        freeproc(np);
        release(&ptable_lock);
        return -1;
    }

    // 关键修复：更新子进程的TRAPFRAME映射，指向子进程自己的trapframe
    extern pte_t *walk(pagetable_t, uint64_t, int);
    pte_t *pte = walk(np->pagetable, TRAPFRAME, 0);
    if (pte != 0) {
        *pte = PA2PTE((uint64_t)np->trapframe) | PTE_R | PTE_W | PTE_V;
    }

    if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0) {
        acquire(&ptable_lock);
        freeproc(np);
        release(&ptable_lock);
        return -1;
    }
    np->sz = p->sz;

    // 复制父进程的陷阱帧
    *(np->trapframe) = *(p->trapframe);
    // 子进程的 fork 返回值为 0
    np->trapframe->a0 = 0;

    np->parent = p;

    acquire(&ptable_lock);

    // 实验五：子进程继承父进程的优先级
    np->priority = p->priority;

    np->state = RUNNABLE;
    np->wait_start = get_time();  // 设置等待开始时间，确保轮转调度正常工作
    release(&ptable_lock);

    return np->pid;
}

//===========================================
// 实验五：优先级调度实现
//===========================================

// 获取当前时间（从硬件 cycle 计数器读取）
uint64_t get_time(void) {
    uint64_t cycles;
    asm volatile("csrr %0, cycle" : "=r"(cycles));
    return cycles;
}

// 设置进程优先级
void set_priority(struct proc *p, int priority) {
    if (priority < 0 || priority > MAX_PRIORITY) {
        return; // 无效优先级
    }

    p->priority = priority;
    p->base_priority = priority;  // 设置基础优先级

    // 根据优先级调整时间片
    switch (priority) {
        case PRIORITY_HIGH:
            p->time_slice = 200;  // 高优先级进程获得更长时间片
            break;
        case PRIORITY_NORMAL:
            p->time_slice = 100;  // 普通时间片
            break;
        case PRIORITY_LOW:
            p->time_slice = 50;   // 低优先级进程时间片较短
            break;
        case PRIORITY_IDLE:
            p->time_slice = 10;   // 空闲进程最短时间片
            break;
    }
}

// Aging 机制：提升等待时间过长的进程优先级
void apply_aging(void) {
    uint64_t current_time = get_time();

    for (struct proc *p = procs; p < &procs[NPROC]; p++) {
        if (p->state == RUNNABLE && p->priority > PRIORITY_HIGH) {
            uint64_t wait_time = current_time - p->wait_start;

            // 如果等待时间超过阈值，提升优先级
            if (wait_time > AGING_THRESHOLD) {
                p->priority--;  // 提升优先级（数值越小优先级越高）
                p->wait_start = current_time;  // 重置等待开始时间
                printf("Aging: boosted process %d priority to %d\n", p->pid, p->priority);

                // 不能超过最高优先级
                if (p->priority < PRIORITY_HIGH) {
                    p->priority = PRIORITY_HIGH;
                }
            }
        }
    }
}

// 重置进程优先级为基础优先级
void reset_priority_to_base(struct proc *p) {
    if (p->priority != p->base_priority) {
        p->priority = p->base_priority;
        printf("Reset: process %d priority reset to base %d\n", p->pid, p->base_priority);
    }
}

// 更新进程等待时间
void update_wait_time(struct proc *p) {
    uint64_t current_time = get_time();

    if (p->state == RUNNABLE && p->wait_start == 0) {
        p->wait_start = current_time;  // 开始等待
    } else if (p->state == RUNNING) {
        if (p->wait_start > 0) {
            p->total_wait += current_time - p->wait_start;
            p->wait_start = 0;  // 停止等待计时
        }
    }
}

// 查找最高优先级的可运行进程
struct proc* find_highest_priority_proc(void) {
    struct proc *p;
    struct proc *candidates[NPROC];
    int candidate_count = 0;
    int highest_priority = MAX_PRIORITY + 1;  // 初始化为最低优先级
    static struct proc *last_scheduled = 0;  // 记录上次调度的进程

    // 第一轮：找到最高优先级
    for (p = procs; p < &procs[NPROC]; p++) {
        if (p->state == RUNNABLE) {
            if (p->priority < highest_priority) {
                highest_priority = p->priority;
            }
        }
    }

    // 如果没有可运行进程，返回
    if (highest_priority > MAX_PRIORITY) {
        return 0;
    }

    // 第二轮：收集所有同等最高优先级的可运行进程
    for (p = procs; p < &procs[NPROC]; p++) {
        if (p->state == RUNNABLE && p->priority == highest_priority) {
            candidates[candidate_count++] = p;
        }
    }

    // 如果只有一个候选进程，直接返回
    if (candidate_count == 1) {
        last_scheduled = candidates[0];
        return candidates[0];
    }

    // 多个候选进程：实现round-robin
    // 找到上次调度进程的下一个进程
    if (last_scheduled != 0) {
        // 在候选进程中查找上次调度进程
        for (int i = 0; i < candidate_count; i++) {
            if (candidates[i] == last_scheduled) {
                // 找到下一个进程（循环）
                int next_idx = (i + 1) % candidate_count;
                last_scheduled = candidates[next_idx];
                return candidates[next_idx];
            }
        }
    }

    // 如果上次调度的进程不在当前候选列表中，选择第一个
    last_scheduled = candidates[0];
    return candidates[0];
}

// 更新进程的时间片信息
void update_time_slice(struct proc *p) {
    p->cpu_time++;
    p->last_run = get_time();

    // 简单的动态优先级调整：CPU密集型进程降低优先级
    if (p->cpu_time > 500 && p->priority < MAX_PRIORITY) {
        p->priority++;  // 降低优先级
        printf("Process %d priority downgraded to %d\n", p->pid, p->priority);
    }
}
