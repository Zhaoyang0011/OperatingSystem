#include <spinlock.h>

void spinlock_init(spinlock_t *spinlock) {
    spinlock->lock = 0;
}

// 加锁函数
void spin_lock(spinlock_t *lock) {
    __asm__ __volatile__("1:         \n"
                         "lock; xchg  %0, %1 \n"
                         "cmpl   $0, %0      \n"
                         "jnz    2f      \n"
                         ".section .spinlock.text,"
                         "\"ax\""
                         "\n"                    // 重新定义一个代码段所以jnz 2f下面并不是
                         "2:         \n"         // cmpl $0,%1 事实上下面的代码不会常常执行,
                         "cmpl   $0, %1      \n" // 这是为了不在cpu指令高速缓存中填充无用代码
                         "jne    2b      \n"
                         "jmp    1b      \n"
                         ".previous      \n"
            :
            : "r"(1), "m"(*lock));
    return;
}

// 解锁函数
void spin_unlock(spinlock_t *lock) {
    __asm__ __volatile__("movl   $0, %0\n" // 解锁把lock内存中的值设为0就行
            :
            : "m"(*lock));
}

void spinlock_cli(spinlock_t *lock, cpuflg_t *cpuflg) {
    __asm__ __volatile__("pushfq             \n\t"
                         "cli                \n\t"
                         "popq %0            \n\t"

                         "1:                 \n\t"
                         "lock; xchg  %1, %2 \n\t"
                         "cmpl   $0,%1       \n\t"
                         "jnz    2f          \n\t"
                         ".section .spinlock.text,"
                         "\"ax\""
                         "\n\t"                    // 重新定义一个代码段所以jnz 2f下面并不是
                         "2:                 \n\t" // cmpl $0,%1 事实上下面的代码不会常常执行,
                         "cmpl   $0,%2       \n\t" // 这是为了不在cpu指令高速缓存中填充无用代码
                         "jne    2b          \n\t"
                         "jmp    1b          \n\t"
                         ".previous          \n\t"
            : "=m"(*cpuflg)
            : "r"(1), "m"(*lock));
    return;
}

void spinunlock_sti(spinlock_t *lock, cpuflg_t *cpuflg) {
    __asm__ __volatile__("movl   $0, %0\n\t"
                         "pushq %1 \n\t"
                         "popfq \n\t"
            :
            : "m"(*lock), "m"(*cpuflg));
    return;
}