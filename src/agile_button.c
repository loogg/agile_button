/**
 * @file    agile_button.c
 * @brief   Agile Button 软件包源文件
 * @author  马龙伟 (2544047213@qq.com)
 * @version 1.1.1
 * @date    2021-12-29
 *
 @verbatim
    使用：
    如果未使能 PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT:
    1. agile_btn_env_init 初始化环境
    2. 创建一个线程，周期调用 agile_btn_process，建议周期时间不要太长

    - agile_btn_create / agile_btn_init 创建 / 初始化对象
    - agile_btn_set_elimination_time 更改消抖时间，可忽略
    - agile_btn_set_hold_cycle_time 更改持续按下触发周期时间，可忽略
      该操作也可在运行过程中执行
    - agile_btn_set_event_cb 设置事件触发回调
    - agile_btn_start 启动运行
    - agile_btn_stop 运行过程中强制停止

 @endverbatim
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#include <agile_button.h>

/** @defgroup RT_Thread_DBG_Configuration RT-Thread DBG Configuration
 * @{
 */

/** @name RT-Thread DBG 功能配置
 * @{
 */
#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME "agile_button"
#ifdef PKG_AGILE_BUTTON_DEBUG
#define DBG_LEVEL DBG_LOG
#else
#define DBG_LEVEL DBG_INFO
#endif
#include <rtdbg.h>
/**
 * @}
 */

/**
 * @}
 */

#ifdef PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT

/** @defgroup AGILE_BUTTON_Thread_Auto_Init Agile Button Thread Auto Init
 * @{
 */

/** @defgroup AGILE_BUTTON_Thread_Auto_Init_Configuration Agile Button Thread Auto Init Configuration
 * @{
 */

/** @name Agile Button 自动初始化线程配置
 * @{
 */
#ifndef PKG_AGILE_BUTTON_THREAD_STACK_SIZE
#define PKG_AGILE_BUTTON_THREAD_STACK_SIZE 256 /**< Agile Button 线程堆栈大小 */
#endif

#ifndef PKG_AGILE_BUTTON_THREAD_PRIORITY
#define PKG_AGILE_BUTTON_THREAD_PRIORITY RT_THREAD_PRIORITY_MAX - 4 /**< Agile Button 线程优先级 */
#endif
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT */

/** @defgroup AGILE_BUTTON_Private_Constants Agile Button Private Constants
 * @{
 */
#define AGILE_BUTTON_ELIMINATION_TIME_DEFAULT  15   /**< 按键消抖默认时间 15ms */
#define AGILE_BUTTON_TWO_INTERVAL_TIME_DEFAULT 500  /**< 两次按键按下间隔超过500ms清零重复计数 */
#define AGILE_BUTTON_HOLD_CYCLE_TIME_DEFAULT   1000 /**< 按键按下后持续调用回调函数的周期 */
/**
 * @}
 */

/** @defgroup AGILE_BUTTON_Private_Macros Agile Button Private Macros
 * @{
 */

/**
 * @brief   获取按键引脚电平状态
 * @param   btn Agile Button 对象指针
 * @return  引脚电平
 */
#define AGILE_BUTTON_PIN_STATE(btn) rt_pin_read(btn->pin)

/**
 * @brief   调用按键事件的回调函数
 * @param   btn Agile Button 对象指针
 * @param   event 事件类型
 * @return  无
 */
#define AGILE_BUTTON_EVENT_CB(btn, event) \
    do {                                  \
        RT_ASSERT(btn);                   \
        if (btn->event_cb[event]) {       \
            btn->event_cb[event](btn);    \
        }                                 \
    } while (0)

/**
 * @}
 */

/** @defgroup AGILE_BUTTON_Private_Variables Agile Button Private Variables
 * @{
 */
ALIGN(RT_ALIGN_SIZE)
static rt_slist_t _slist_head = RT_SLIST_OBJECT_INIT(_slist_head); /**< Agile Button 链表头节点 */
static struct rt_mutex _mtx;                                       /**< Agile Button 互斥锁 */
static uint8_t _is_init = 0;                                       /**< Agile Button 初始化完成标志 */

#ifdef PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT
static struct rt_thread _thread;                                  /**< Agile Button 线程控制块 */
static uint8_t _thread_stack[PKG_AGILE_BUTTON_THREAD_STACK_SIZE]; /**< Agile Button 线程堆栈 */
#endif
/**
 * @}
 */

/** @defgroup AGILE_BUTTON_Private_Functions Agile Button Private Functions
 * @{
 */

/**
 * @brief   计算按键按下持续时间
 * @param   btn Agile Button 对象指针
 */
static void agile_btn_cal_hold_time(agile_btn_t *btn)
{
    RT_ASSERT(btn);

    if (rt_tick_get() < btn->tick_timeout) {
        btn->hold_time = RT_TICK_MAX - btn->tick_timeout + rt_tick_get();
    } else {
        btn->hold_time = rt_tick_get() - btn->tick_timeout;
    }
    btn->hold_time = btn->hold_time * (1000 / RT_TICK_PER_SECOND);
}

/**
 * @}
 */

/** @defgroup AGILE_BUTTON_Exported_Functions Agile Button Exported Functions
 * @{
 */

#ifdef RT_USING_HEAP

/**
 * @brief   创建 Agile Button 对象
 * @param   pin 按键引脚
 * @param   active_logic 有效电平
 * @param   pin_mode 引脚模式
 * @return  !=RT_NULL:Agile Button 对象指针; RT_NULL:异常
 */
agile_btn_t *agile_btn_create(uint32_t pin, uint32_t active_logic, uint32_t pin_mode)
{
    if (!_is_init) {
        LOG_E("Please call agile_btn_env_init first.");
        return RT_NULL;
    }

    agile_btn_t *btn = (agile_btn_t *)rt_malloc(sizeof(agile_btn_t));
    if (btn == RT_NULL)
        return RT_NULL;

    rt_memset(btn, 0, sizeof(agile_btn_t));
    btn->active = 0;
    btn->repeat_cnt = 0;
    btn->elimination_time = AGILE_BUTTON_ELIMINATION_TIME_DEFAULT;
    btn->event = BTN_EVENT_SUM;
    btn->state = BTN_STATE_NONE_PRESS;
    btn->hold_time = 0;
    btn->prev_hold_time = 0;
    btn->hold_cycle_time = AGILE_BUTTON_HOLD_CYCLE_TIME_DEFAULT;
    btn->pin = pin;
    btn->active_logic = active_logic;
    btn->tick_timeout = rt_tick_get();
    rt_slist_init(&(btn->slist));

    rt_pin_mode(pin, pin_mode);

    return btn;
}

/**
 * @brief   删除 Agile Button 对象
 * @param   btn Agile Button 对象指针
 * @return  RT_EOK:成功
 */
int agile_btn_delete(agile_btn_t *btn)
{
    RT_ASSERT(btn);

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    rt_slist_remove(&_slist_head, &(btn->slist));
    btn->slist.next = RT_NULL;
    rt_mutex_release(&_mtx);
    rt_free(btn);

    return RT_EOK;
}

#endif /* RT_USING_HEAP */

/**
 * @brief   初始化 Agile Button 对象
 * @param   btn Agile Button 对象指针
 * @param   pin 按键引脚
 * @param   active_logic 有效电平
 * @param   pin_mode 引脚模式
 * @return  RT_EOK:成功; !=RT_EOK:异常
 */
int agile_btn_init(agile_btn_t *btn, uint32_t pin, uint32_t active_logic, uint32_t pin_mode)
{
    RT_ASSERT(btn);

    if (!_is_init) {
        LOG_E("Please call agile_btn_env_init first.");
        return -RT_ERROR;
    }

    rt_memset(btn, 0, sizeof(agile_btn_t));
    btn->active = 0;
    btn->repeat_cnt = 0;
    btn->elimination_time = AGILE_BUTTON_ELIMINATION_TIME_DEFAULT;
    btn->event = BTN_EVENT_SUM;
    btn->state = BTN_STATE_NONE_PRESS;
    btn->hold_time = 0;
    btn->prev_hold_time = 0;
    btn->hold_cycle_time = AGILE_BUTTON_HOLD_CYCLE_TIME_DEFAULT;
    btn->pin = pin;
    btn->active_logic = active_logic;
    btn->tick_timeout = rt_tick_get();
    rt_slist_init(&(btn->slist));

    rt_pin_mode(pin, pin_mode);

    return RT_EOK;
}

/**
 * @brief   启动 Agile Button 对象
 * @param   btn Agile Button 对象指针
 * @return  RT_EOK:成功; !=RT_OK:异常
 */
int agile_btn_start(agile_btn_t *btn)
{
    RT_ASSERT(btn);

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    if (btn->active) {
        rt_mutex_release(&_mtx);
        return -RT_ERROR;
    }
    btn->repeat_cnt = 0;
    btn->event = BTN_EVENT_SUM;
    btn->state = BTN_STATE_NONE_PRESS;
    btn->hold_time = 0;
    btn->prev_hold_time = 0;
    btn->tick_timeout = rt_tick_get();
    rt_slist_append(&_slist_head, &(btn->slist));
    btn->active = 1;
    rt_mutex_release(&_mtx);

    return RT_EOK;
}

/**
 * @brief   停止 Agile Button 对象
 * @param   btn Agile Button 对象指针
 * @return  RT_EOK:成功
 */
int agile_btn_stop(agile_btn_t *btn)
{
    RT_ASSERT(btn);

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    if (!btn->active) {
        rt_mutex_release(&_mtx);
        return RT_EOK;
    }
    rt_slist_remove(&_slist_head, &(btn->slist));
    btn->slist.next = RT_NULL;
    btn->active = 0;
    rt_mutex_release(&_mtx);

    return RT_EOK;
}

/**
 * @brief   设置按键消抖时间
 * @param   btn Agile Button 对象指针
 * @param   elimination_time 消抖时间(单位ms)
 * @return  RT_EOK:成功
 */
int agile_btn_set_elimination_time(agile_btn_t *btn, uint8_t elimination_time)
{
    RT_ASSERT(btn);

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    btn->elimination_time = elimination_time;
    rt_mutex_release(&_mtx);

    return RT_EOK;
}

/**
 * @brief   设置按键按下后 BTN_HOLD_EVENT 事件回调函数的周期
 * @param   btn Agile Button 对象指针
 * @param   hold_cycle_time 周期时间(单位ms)
 * @return  RT_EOK:成功
 */
int agile_btn_set_hold_cycle_time(agile_btn_t *btn, uint32_t hold_cycle_time)
{
    RT_ASSERT(btn);

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    btn->hold_cycle_time = hold_cycle_time;
    rt_mutex_release(&_mtx);

    return RT_EOK;
}

/**
 * @brief   设置按键事件回调函数
 * @param   btn Agile Button 对象指针
 * @param   event 事件类型
 * @param   event_cb 事件回调函数
 * @return  RT_EOK:成功; !=RT_OK:异常
 */
int agile_btn_set_event_cb(agile_btn_t *btn, enum agile_btn_event event, void (*event_cb)(agile_btn_t *btn))
{
    RT_ASSERT(btn);

    if (event >= BTN_EVENT_SUM)
        return -RT_ERROR;
    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    btn->event_cb[event] = event_cb;
    rt_mutex_release(&_mtx);

    return RT_EOK;
}

/**
 * @brief   处理所有 Agile Button 对象
 * @note    如果使能 PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT, 这个函数将被自动初始化线程 5ms 周期调用。
 *          用户调用需要创建一个线程并将这个函数放入 while (1) {} 中。
 */
void agile_btn_process(void)
{
    rt_slist_t *node;

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);
    rt_slist_for_each(node, &_slist_head)
    {
        agile_btn_t *btn = rt_slist_entry(node, agile_btn_t, slist);
        switch (btn->state) {
        case BTN_STATE_NONE_PRESS: {
            if (AGILE_BUTTON_PIN_STATE(btn) == btn->active_logic) {
                btn->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(btn->elimination_time);
                btn->state = BTN_STATE_CHECK_PRESS;
            } else {
                /* 2次按下中间间隔过大，清零重按计数 */
                if (btn->repeat_cnt) {
                    if ((rt_tick_get() - btn->tick_timeout) < (RT_TICK_MAX / 2)) {
                        btn->repeat_cnt = 0;
                    }
                }
            }
        } break;
        case BTN_STATE_CHECK_PRESS: {
            if (AGILE_BUTTON_PIN_STATE(btn) == btn->active_logic) {
                if ((rt_tick_get() - btn->tick_timeout) < (RT_TICK_MAX / 2)) {
                    btn->state = BTN_STATE_PRESS_DOWN;
                }
            } else {
                btn->state = BTN_STATE_NONE_PRESS;
            }
        } break;
        case BTN_STATE_PRESS_DOWN: {
            btn->hold_time = 0;
            btn->prev_hold_time = 0;
            btn->repeat_cnt++;
            btn->event = BTN_PRESS_DOWN_EVENT;
            AGILE_BUTTON_EVENT_CB(btn, btn->event);

            btn->tick_timeout = rt_tick_get();
            btn->state = BTN_STATE_PRESS_HOLD;
        } break;
        case BTN_STATE_PRESS_HOLD: {
            if (AGILE_BUTTON_PIN_STATE(btn) == btn->active_logic) {
                agile_btn_cal_hold_time(btn);
                if (btn->hold_time - btn->prev_hold_time >= btn->hold_cycle_time) {
                    btn->event = BTN_HOLD_EVENT;
                    AGILE_BUTTON_EVENT_CB(btn, btn->event);
                    btn->prev_hold_time = btn->hold_time;
                }
            } else {
                btn->state = BTN_STATE_PRESS_UP;
            }
        } break;
        case BTN_STATE_PRESS_UP: {
            btn->event = BTN_PRESS_UP_EVENT;
            AGILE_BUTTON_EVENT_CB(btn, btn->event);
            btn->event = BTN_CLICK_EVENT;
            AGILE_BUTTON_EVENT_CB(btn, btn->event);

            btn->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(AGILE_BUTTON_TWO_INTERVAL_TIME_DEFAULT);
            btn->state = BTN_STATE_NONE_PRESS;
        } break;
        default:
            break;
        }
    }
    rt_mutex_release(&_mtx);
}

/**
 * @brief   Agile Button 环境初始化
 * @note    使用其他 API 之前该函数必须被调用。
 *          如果使能 PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT, 这个函数将被自动调用。
 */
void agile_btn_env_init(void)
{
    if (_is_init)
        return;

    rt_mutex_init(&_mtx, "btn_mtx", RT_IPC_FLAG_FIFO);

    _is_init = 1;
}

/**
 * @}
 */

#ifdef PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT

/** @addtogroup AGILE_BUTTON_Thread_Auto_Init
 * @{
 */

/** @defgroup AGILE_BUTTON_Thread_Auto_Init_Functions Agile Button Thread Auto Init Functions
 * @{
 */

/**
 * @brief   Agile Button 内部线程函数入口
 * @param   parameter 线程参数
 */
static void agile_btn_auto_thread_entry(void *parameter)
{
    while (1) {
        agile_btn_process();
        rt_thread_mdelay(5);
    }
}

/**
 * @brief   Agile Button 内部线程初始化
 * @return  RT_EOK:成功
 */
static int agile_btn_auto_thread_init(void)
{
    agile_btn_env_init();

    rt_thread_init(&_thread,
                   "agbtn",
                   agile_btn_auto_thread_entry,
                   RT_NULL,
                   &_thread_stack[0],
                   sizeof(_thread_stack),
                   PKG_AGILE_BUTTON_THREAD_PRIORITY,
                   100);

    rt_thread_startup(&_thread);

    return RT_EOK;
}
INIT_APP_EXPORT(agile_btn_auto_thread_init);

/**
 * @}
 */

/**
 * @}
 */

#endif /* PKG_AGILE_BUTTON_USING_THREAD_AUTO_INIT */
