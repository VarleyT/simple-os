#include "os.h"
#include "log.h"

#define LOG_LVL LOG_LVL_DEBUG
#include "log.h"

#define APP_ASSERT(val)                        \
    do {                                       \
        if (!(val)) {                          \
            LOG_E("error! val = %d\r\n", val); \
            return -1;                         \
        }                                      \
    } while (0)

OS_FUNC_PROTO(task_a, params);
OS_FUNC_PROTO(task_b, params);
OS_FUNC_PROTO(task_c, params);

static uint8_t stack_a[512];
static uint8_t stack_b[512];
static uint8_t stack_c[512];

void BSP_Init(void) {
    USART1_Init();
}

int main() {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    BSP_Init();
    LOG_D("system start\r\n");

    bool bRet;
    bRet = os_init();
    APP_ASSERT(bRet);

    bRet = os_task_init(task_a, NULL, stack_a, sizeof(stack_a));
    APP_ASSERT(bRet);
    bRet = os_task_init(task_b, NULL, stack_b, sizeof(stack_b));
    APP_ASSERT(bRet);
    bRet = os_task_init(task_c, NULL, stack_c, sizeof(stack_c));
    APP_ASSERT(bRet);

    bRet = os_start();
    APP_ASSERT(bRet);

    while (1) {
        LOG_E("error!\r\n");
    }

    return 0;
}

OS_FUNC_PROTO(task_a, params) {
    while (1) {
        __disable_irq();
        LOG_D("hello\r\n");
        __enable_irq();
        os_delay(1000);
    }
}

OS_FUNC_PROTO(task_b, params) {
    while (1) {
        __disable_irq();
        LOG_D("world\r\n");
        __enable_irq();
        os_delay(1000);
    }
}

OS_FUNC_PROTO(task_c, params) {
    while (1) {
        __disable_irq();
        LOG_D("hello world!\r\n");
        __enable_irq();
        os_delay(1000);
    }
}
