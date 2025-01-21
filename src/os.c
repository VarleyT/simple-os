/*
 * This file is part of os.h.
 *
 * Copyright (C) 2016 Adam Heinrich <adam@adamh.cz>
 *
 * Os.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Os.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with os.h.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <os.h>
#include <string.h>

enum os_task_status {
    OS_TASK_STATUS_IDLE = 1,
    OS_TASK_STATUS_ACTIVE,
};

struct os_task {
    /* 堆栈指针 (sp) 必须是第一个元素，
	  	因为它与结构本身位于同一地址（这使得可以从 PendSV_Handler 的程序集实现中安全地定位它）。
	   	编译器可能会在其他结构元素之间添加填充。 */
    volatile uint32_t sp;
    void (*handler)(void *params);
    void *params;
    volatile enum os_task_status status;
};

static enum state {
    STATE_DEFAULT = 1,
    STATE_INITIALIZED,
    STATE_TASKS_INITIALIZED,
    STATE_STARTED,
} state = STATE_DEFAULT;

static struct task_table {
    struct os_task tasks[OS_CONFIG_MAX_TASKS];
    volatile size_t current_task;
    size_t size;
} task_table;

volatile struct os_task *os_curr_task;
volatile struct os_task *os_next_task;
static volatile uint32_t g_system_ticks = 0;

void SysTick_Handler(void);

static void task_finished(void) {
    /* 当某些任务处理程序返回时调用此函数。 */

    volatile unsigned int i = 0;
    while (1) {
        i++;
    }
}

bool os_init(void) {
    if (state != STATE_DEFAULT)
        return false;

    memset(&task_table, 0, sizeof(task_table));
    state = STATE_INITIALIZED;

    return true;
}

bool os_task_init(void (*handler)(void *params), void *task_params, uint32_t *stack, size_t stack_size) {
    if (state != STATE_INITIALIZED && state != STATE_TASKS_INITIALIZED)
        return false;

    if (task_table.size >= OS_CONFIG_MAX_TASKS)
        return false;

    if ((stack_size % sizeof(uint32_t)) != 0)  /* TODO: 使用断言? */
        return false;

    uint32_t stack_offset = (stack_size / sizeof(uint32_t));
    if (stack_offset <= 16)
        return false;

    /* 初始化任务结构体，将SP设置为栈顶减去16个字（64字节），为存储16个寄存器留出空间： */
    struct os_task *task = &task_table.tasks[task_table.size];
    task->handler = handler;
    task->params = task_params;
    task->sp = (uint32_t) (stack + stack_offset - 16);
    task->status = OS_TASK_STATUS_IDLE;

    /* 保存将在异常返回时恢复的寄存器初始值：
   		- XPSR：默认值（0x01000000）
   		- PC：指向处理函数（LSB屏蔽，因为如果异常返回时pc<0> == '1'，行为是不可预测的）
   		- LR：指向处理函数返回时要调用的函数
   		- R0：指向处理函数的参数 */
    stack[stack_offset - 1] = 0x01000000;
    stack[stack_offset - 2] = (uint32_t) handler & ~0x01UL;
    stack[stack_offset - 3] = (uint32_t) &task_finished;
    stack[stack_offset - 8] = (uint32_t) task_params;

#ifdef OS_CONFIG_DEBUG
    size_t base = (task_table.size + 1) * 1000;
    stack[stack_offset - 4] = base + 12; /* R12 */
    stack[stack_offset - 5] = base + 3;  /* R3  */
    stack[stack_offset - 6] = base + 2;  /* R2  */
    stack[stack_offset - 7] = base + 1;  /* R1  */
    /* stack[stack_offset-8] 为 R0 */
    stack[stack_offset - 9] = base + 11;  /* R11 */
    stack[stack_offset - 10] = base + 10; /* R10 */
    stack[stack_offset - 11] = base + 9;  /* R9  */
    stack[stack_offset - 12] = base + 8;  /* R8  */
    stack[stack_offset - 13] = base + 7;  /* R7  */
    stack[stack_offset - 14] = base + 6;  /* R6  */
    stack[stack_offset - 15] = base + 5;  /* R5  */
    stack[stack_offset - 16] = base + 4;  /* R4  */
#endif                                    /* OS_CONFIG_DEBUG */

    state = STATE_TASKS_INITIALIZED;
    task_table.size++;

    return true;
}

bool os_start(void) {
    if (state != STATE_TASKS_INITIALIZED)
        return false;

    NVIC_SetPriority(PendSV_IRQn, 0xff);  /* 最低优先级 */
    NVIC_SetPriority(SysTick_IRQn, 0x00); /* 最高优先级 */

    /* 启动SysTick定时器： */
    uint32_t ret_val = SysTick_Config(SystemCoreClock / OS_TICK);
    if (ret_val != 0) {
        return false;
    }

    /* 开始第一个任务： */
    os_curr_task = &task_table.tasks[task_table.current_task];
    state = STATE_STARTED;

    __set_PSP(os_curr_task->sp + 64); /* 将 PSP 设置为任务堆栈的顶部 */
    __set_CONTROL(0x03);              /* 切换到使用PSP的非特权线程模式 */
    __ISB();                          /* 更改CONTROL后执行ISB（推荐） */

    os_curr_task->handler(os_curr_task->params);

    return true;
}

void os_delay(uint32_t ms) {
    uint32_t ticks = os_get_ticks() + ms / (1000 / OS_TICK);
    while (os_get_ticks() < ticks) {
        ;
    }
}

__INLINE uint32_t os_get_ticks(void) {
    return g_system_ticks;
}

void SysTick_Handler(void) {
    os_curr_task = &task_table.tasks[task_table.current_task];
    os_curr_task->status = OS_TASK_STATUS_IDLE;

    /* 选择下一个任务： */
    task_table.current_task++;
    if (task_table.current_task >= task_table.size)
        task_table.current_task = 0;

    os_next_task = &task_table.tasks[task_table.current_task];
    os_next_task->status = OS_TASK_STATUS_ACTIVE;

    g_system_ticks++;

    /* 触发执行实际上下文切换的 PendSV： */
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

// clang-format off
__asm void PendSV_Handler(void) {
	extern os_curr_task
	extern os_next_task

	/* 禁用中断： */
	cpsid	i

	/*
	NVIC硬件将异常帧保存到堆栈中：
	+------+
	|      | <- 中断前的堆栈指针（原始堆栈指针）
	| xPSR |
	|  PC  |
	|  LR  |
	|  R12 |
	|  R3  |
	|  R2  |
	|  R1  |
	|  R0  | <- 进入中断后的 SP 堆栈指针（原始堆栈指针 + 32 字节）
	+------+

	由软件保存的寄存器（PendSV_Handler）：
	+------+
	|  R11 |
	|  R10 |
	|  R9  |
	|  R8  |
	|  R7  |
	|  R6  |
	|  R5  |
	|  R4  | <- 保存的 SP 堆栈指针（原始堆栈指针 + 64 字节）
	+------+
	*/

	/* 将寄存器R4-R11（32字节）保存到当前PSP（进程堆栈指针）并使PSP指向最后一个堆栈寄存器（R4）：
		- MRS/MSR指令用于加载/保存特殊寄存器。 */
	mrs	r0, psp
	subs	r0, #32
	stmia	r0,{r4-r11}

	/* 保存当前任务的堆栈指针（SP）： */
	ldr	r2, =os_curr_task
	ldr	r1, [r2]
	str	r0, [r1]

	/* 加载下一个任务的堆栈指针： */
	ldr	r2, =os_next_task
	ldr	r1, [r2]
	ldr	r0, [r1]

	/* 从新的 PSP 加载寄存器 R4-R11（32 字节），并使 PSP 指向异常堆栈帧的末尾。 NVIC硬件在异常返回后会恢复剩余的寄存器： */
	ldmia	r0!,{r4-r11}
	msr	psp, r0

	/* EXC_RETURN - PSP 线程模式： */
	ldr r0, =0xFFFFFFFD

	/* 使能中断： */
	cpsie	i

	bx	r0
}
// clang-format on
