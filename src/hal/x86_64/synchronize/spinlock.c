#include <spinlock.h>

void spinlock_init(spinlock_t *spinlock)
{
    spinlock->lock = 0;
}

// 加锁函数
KLINE void spin_lock(spinlock_t *lock)
{
    __asm__ __volatile__("1: \n"
                         "lock; xchg  %0, %1 \n" // 把值为1的寄存器和lock内存中的值进行交换
                         "cmpl   $0, %0 \n"      // 用0和交换回来的值进行比较
                         "jnz    2f \n"          // 不等于0则跳转后面2标号处运行
                         "jmp 3f \n"             // 若等于0则跳转后面3标号处返回
                         "2:         \n"
                         "cmpl   $0, %1  \n" // 用0和lock内存中的值进行比较
                         "jne    2b      \n" // 若不等于0则跳转到前面2标号处运行继续比较
                         "jmp    1b      \n" // 若等于0则跳转到前面1标号处运行，交换并加锁
                         "3:  \n"
                         :
                         : "r"(1), "m"(*lock));
}

// 解锁函数
KLINE void spin_unlock(spinlock_t *lock)
{
    __asm__ __volatile__("movl   $0, %0\n" // 解锁把lock内存中的值设为0就行
                         :
                         : "m"(*lock));
}