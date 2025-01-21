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

#ifndef OS_CONFIG_H
#define OS_CONFIG_H

/* 配置并包含CMSIS库（或供应商的已配置和包含CMSIS的MCU头文件） */
#include "stm32f4xx.h"

/* 任务的最大数量 */
#define OS_CONFIG_MAX_TASKS	10

/* 任务函数原型 */
#define OS_FUNC_PROTO(name, param) static void name##(void *##param)

/* 系统节拍频率 Hz */
#define OS_TICK (100)

/* 启用调试 */
// #define OS_CONFIG_DEBUG

#endif /* OS_CONFIG_H */
