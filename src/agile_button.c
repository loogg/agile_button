/*************************************************
 All rights reserved.
 File name:     agile_button.c
 Description:   button软件包源码
 History:
 1. Version:      v1.0.0
    Date:         2019-10-12
    Author:       Longwei Ma
    Modification: 新建版本
*************************************************/

#include <agile_button.h>

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME    "agile_button"
#ifdef PKG_AGILE_BUTTON_DEBUG
#define DBG_LEVEL           DBG_LOG
#else
#define DBG_LEVEL           DBG_INFO
#endif
#include <rtdbg.h>

// agile_button 线程堆栈大小
#ifndef PKG_AGILE_BUTTON_THREAD_STACK_SIZE
#define PKG_AGILE_BUTTON_THREAD_STACK_SIZE 256
#endif

// agile_button 线程优先级
#ifndef PKG_AGILE_BUTTON_THREAD_PRIORITY
#define PKG_AGILE_BUTTON_THREAD_PRIORITY RT_THREAD_PRIORITY_MAX - 4
#endif

// 按键消抖默认时间 15ms
#define AGILE_BUTTON_ELIMINATION_TIME_DEFAULT    15

// 两次按键按下间隔超过500ms清零重复计数
#define AGILE_BUTTON_TWO_INTERVAL_TIME_DEFAULT   500

// 按键按下后持续调用回调函数的周期
#define AGILE_BUTTON_HOLD_CYCLE_TIME_DEFAULT     1000

// 获取按键引脚电平状态
#define AGILE_BUTTON_PIN_STATE(btn)  rt_pin_read(btn->pin)

// 调用按键事件的回调函数
#define AGILE_BUTTON_EVENT_CB(btn, event) do { \
    RT_ASSERT(btn); \
    if(btn->event_cb[event]) { \
        btn->event_cb[event](btn); \
    } \
} while(0)

// agile_button 单向链表
static rt_slist_t agile_btn_list = RT_SLIST_OBJECT_INIT(agile_btn_list);
// agile_button 互斥锁
static rt_mutex_t lock_mtx = RT_NULL;
// agile_button 初始化完成标志
static uint8_t is_initialized = 0;

/**
* Name:                   agile_btn_create
* Brief:                  创建按键对象
* Input:
*   @pin:                 按键引脚
*   @active_logic:        有效电平
*   @pin_mode:            引脚模式
* Output:                 !=RT_NULL:agile_btn对象指针
*                         RT_NULL:异常
*/
agile_btn_t *agile_btn_create(rt_base_t pin, rt_base_t active_logic, rt_base_t pin_mode)
{
    if (!is_initialized)
    {
        LOG_E("Agile button haven't initialized!");
        return RT_NULL;
    }
    agile_btn_t *btn = (agile_btn_t *)rt_malloc(sizeof(agile_btn_t));
    if(btn ==RT_NULL)
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
* Name:                   agile_btn_delete
* Brief:                  删除按键对象
* Input:
*   @btn:                 按键对象
* Output:                 RT_EOK:成功
*/
int agile_btn_delete(agile_btn_t *btn)
{
    RT_ASSERT(btn);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    rt_slist_remove(&(agile_btn_list), &(btn->slist));
    btn->slist.next = RT_NULL;
    rt_mutex_release(lock_mtx);
    rt_free(btn);
    return RT_EOK;
}

/**
* Name:                   agile_btn_start
* Brief:                  启动按键
* Input:
*   @btn:                 按键对象
* Output:                 RT_EOK:成功
*                         !=RT_OK:异常
*/
int agile_btn_start(agile_btn_t *btn)
{
    RT_ASSERT(btn);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    if(btn->active)
    {
        rt_mutex_release(lock_mtx);
        return -RT_ERROR;
    }
    btn->repeat_cnt = 0;
    btn->event = BTN_EVENT_SUM;
    btn->state = BTN_STATE_NONE_PRESS;
    btn->hold_time = 0;
    btn->prev_hold_time = 0;
    btn->tick_timeout = rt_tick_get();
    rt_slist_append(&(agile_btn_list), &(btn->slist));
    btn->active = 1;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
* Name:                   agile_btn_stop
* Brief:                  停止按键
* Input:
*   @btn:                 按键对象
* Output:                 RT_EOK:成功
*/
int agile_btn_stop(agile_btn_t *btn)
{
    RT_ASSERT(btn);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    if(!btn->active)
    {
        rt_mutex_release(lock_mtx);
        return RT_EOK;
    }
    rt_slist_remove(&(agile_btn_list), &(btn->slist));
    btn->slist.next = RT_NULL;
    btn->active = 0;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
* Name:                   agile_btn_set_elimination_time
* Brief:                  设置按键消抖时间
* Input:
*   @btn:                 按键对象
*   @elimination_time:    消抖时间(单位ms)
* Output:                 RT_EOK:成功
*/
int agile_btn_set_elimination_time(agile_btn_t *btn, uint8_t elimination_time)
{
    RT_ASSERT(btn);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    btn->elimination_time = elimination_time;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
* Name:                   agile_btn_set_hold_cycle_time
* Brief:                  设置按键按下后BTN_HOLD_EVENT事件回调函数的周期
* Input:
*   @btn:                 按键对象
*   @hold_cycle_time:     周期时间(单位ms)
* Output:                 RT_EOK:成功
*/
int agile_btn_set_hold_cycle_time(agile_btn_t *btn, uint32_t hold_cycle_time)
{
    RT_ASSERT(btn);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    btn->hold_cycle_time = hold_cycle_time;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
* Name:                   agile_btn_set_event_cb
* Brief:                  设置按键事件回调函数
* Input:
*   @btn:                 按键对象
*   @event:               事件
*   @event_cb:            回调函数
* Output:                 RT_EOK:成功
*                         !=RT_OK:异常
*/
int agile_btn_set_event_cb(agile_btn_t *btn, enum agile_btn_event event, void (*event_cb)(agile_btn_t *btn))
{
    RT_ASSERT(btn);
    if(event >= BTN_EVENT_SUM)
        return -RT_ERROR;
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    btn->event_cb[event] = event_cb;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
* Name:             agile_btn_cal_hold_time
* Brief:            计算按键按下持续时间
* Input:
*   @btn:           按键对象
* Output:           none
*/
static void agile_btn_cal_hold_time(agile_btn_t *btn)
{
    RT_ASSERT(btn);
    if(rt_tick_get() < btn->tick_timeout)
    {
        btn->hold_time = RT_TICK_MAX - btn->tick_timeout + rt_tick_get();
    }
    else
    {
        btn->hold_time = rt_tick_get() - btn->tick_timeout;
    }
    btn->hold_time = btn->hold_time * (1000 / RT_TICK_PER_SECOND);
}

/**
* Name:             btn_process
* Brief:            agile_button线程
* Input:
*   @parameter:     线程参数
* Output:           none
*/
static void btn_process(void *parameter)
{
    rt_slist_t *node;
    while (1)
    {
        rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
        rt_slist_for_each(node, &(agile_btn_list))
        {
            agile_btn_t *btn = rt_slist_entry(node, agile_btn_t, slist);
            switch (btn->state)
            {
            case BTN_STATE_NONE_PRESS:
            {
                if (AGILE_BUTTON_PIN_STATE(btn) == btn->active_logic)
                {
                    btn->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(btn->elimination_time);
                    btn->state = BTN_STATE_CHECK_PRESS;
                }
                else
                {
                    // 2次按下中间间隔过大，清零重按计数
                    if (btn->repeat_cnt)
                    {
                        if ((rt_tick_get() - btn->tick_timeout) < (RT_TICK_MAX / 2))
                        {
                            btn->repeat_cnt = 0;
                        }
                    }
                }
            }
            break;
            case BTN_STATE_CHECK_PRESS:
            {
                if (AGILE_BUTTON_PIN_STATE(btn) == btn->active_logic)
                {
                    if ((rt_tick_get() - btn->tick_timeout) < (RT_TICK_MAX / 2))
                    {
                        btn->state = BTN_STATE_PRESS_DOWN;
                    }
                }
                else
                {
                    btn->state = BTN_STATE_NONE_PRESS;
                }
            }
            break;
            case BTN_STATE_PRESS_DOWN:
            {
                btn->hold_time = 0;
                btn->prev_hold_time = 0;
                btn->repeat_cnt++;
                btn->event = BTN_PRESS_DOWN_EVENT;
                AGILE_BUTTON_EVENT_CB(btn, btn->event);

                btn->tick_timeout = rt_tick_get();
                btn->state = BTN_STATE_PRESS_HOLD;
            }
            break;
            case BTN_STATE_PRESS_HOLD:
            {
                if (AGILE_BUTTON_PIN_STATE(btn) == btn->active_logic)
                {
                    agile_btn_cal_hold_time(btn);
                    if (btn->hold_time - btn->prev_hold_time >= btn->hold_cycle_time)
                    {
                        btn->event = BTN_HOLD_EVENT;
                        AGILE_BUTTON_EVENT_CB(btn, btn->event);
                        btn->prev_hold_time = btn->hold_time;
                    }
                }
                else
                {
                    btn->state = BTN_STATE_PRESS_UP;
                }
            }
            break;
            case BTN_STATE_PRESS_UP:
            {
                btn->event = BTN_PRESS_UP_EVENT;
                AGILE_BUTTON_EVENT_CB(btn, btn->event);
                btn->event = BTN_CLICK_EVENT;
                AGILE_BUTTON_EVENT_CB(btn, btn->event);

                btn->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(AGILE_BUTTON_TWO_INTERVAL_TIME_DEFAULT);
                btn->state = BTN_STATE_NONE_PRESS;
            }
            break;
            default:
                break;
            }
        }
        rt_mutex_release(lock_mtx);
        rt_thread_mdelay(5);
    }
}

/**
* Name:             agile_btn_init
* Brief:            agile_button初始化
* Input:            none
* Output:           RT_EOK:成功
*                   !=RT_EOK:失败
*/
static int agile_btn_init(void)
{
    rt_thread_t tid = RT_NULL;
    lock_mtx = rt_mutex_create("btn_mtx", RT_IPC_FLAG_FIFO);
    if (lock_mtx == RT_NULL)
    {
        LOG_E("Agile button initialize failed! lock_mtx create failed!");
        return -RT_ENOMEM;
    }

    tid = rt_thread_create("agile_btn", btn_process, RT_NULL,
                           PKG_AGILE_BUTTON_THREAD_STACK_SIZE, PKG_AGILE_BUTTON_THREAD_PRIORITY, 100);
    if (tid == RT_NULL)
    {
        LOG_E("Agile button initialize failed! thread create failed!");
        rt_mutex_delete(lock_mtx);
        return -RT_ENOMEM;
    }
    rt_thread_startup(tid);
    is_initialized = 1;
    return RT_EOK;
}
INIT_APP_EXPORT(agile_btn_init);
