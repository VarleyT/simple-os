# simple-os

A simple `RTOS` example, this demonstration aims to provide an easy and effective introduction to the concept of `RTOS`. 
The project is modified based on [os.h](https://github.com/adamheinrich/os.h).

一个简单的 `RTOS` 示例，这个示例旨在提供一个简单而有效的 `RTOS` 概念入门。
该项目基于 [os.h](https://github.com/adamheinrich/os.h) 进行了修改。

# Quick Start 快速开始

```C

// Task Stack 任务的栈空间
uint8_t stack_task_hello[512];

// Function Definition 函数定义
OS_FUNC_PROTO(task_hello, params)
{
    while (1)
    {
        printf("hello world!\r\n");
        os_delay(100);
    }
}

// Initialize the OS. 初始化OS
ret = os_init();
// if (!ret) ...

// Initialize task. 初始化任务
ret = os_task_init(task_hello, NULL, stack_task_hello, sizeof(stack_task_hello));
// if (!ret) ...

// Start task scheduling. 开始任务调度
ret = os_start();
// if (!ret) ...

```
