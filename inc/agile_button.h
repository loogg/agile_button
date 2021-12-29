/**
 * @file    agile_button.h
 * @brief   Agile Button 软件包头文件
 * @author  马龙伟 (2544047213@qq.com)
 * @version 1.1.1
 * @date    2021-12-29
 *
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 Ma Longwei.
 * All rights reserved.</center></h2>
 *
 */

#ifndef __PKG_AGILE_BUTTON_H
#define __PKG_AGILE_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>

/** @defgroup AGILE_BUTTON_Exported_Types Agile Button Exported Types
 * @{
 */

/**
 * @brief   Agile Button 对象事件
 */
enum agile_btn_event {
    BTN_PRESS_DOWN_EVENT = 0, /**< 按下事件 */
    BTN_HOLD_EVENT,           /**< 持续按下有效事件 */
    BTN_PRESS_UP_EVENT,       /**< 弹起事件 */
    BTN_CLICK_EVENT,          /**< 点击事件 */
    BTN_EVENT_SUM             /**< 事件总数目 */
};

/**
 * @brief   Agile Button 对象状态
 */
enum agile_btn_state {
    BTN_STATE_NONE_PRESS = 0, /**< 未按下状态 */
    BTN_STATE_CHECK_PRESS,    /**< 抖动检查状态 */
    BTN_STATE_PRESS_DOWN,     /**< 按下状态 */
    BTN_STATE_PRESS_HOLD,     /**< 持续按下状态 */
    BTN_STATE_PRESS_UP,       /**< 弹起状态 */
};

typedef struct agile_btn agile_btn_t; /**< Agile Button 结构体 */

/**
 * @brief   Agile Button 结构体
 */
struct agile_btn {
    uint8_t active;                                    /**< 激活标志 */
    uint8_t repeat_cnt;                                /**< 按键重按计数 */
    uint8_t elimination_time;                          /**< 按键消抖时间(单位ms,默认15ms) */
    enum agile_btn_event event;                        /**< 按键对象事件 */
    enum agile_btn_state state;                        /**< 按键对象状态 */
    uint32_t hold_time;                                /**< 按键按下持续时间(单位ms) */
    uint32_t prev_hold_time;                           /**< 缓存 hold_time 变量 */
    uint32_t hold_cycle_time;                          /**< 按键按下后持续调用回调函数的周期(单位ms,默认1s) */
    uint32_t pin;                                      /**< 按键引脚 */
    uint32_t active_logic;                             /**< 有效电平(PIN_HIGH/PIN_LOW) */
    rt_tick_t tick_timeout;                            /**< 超时时间 */
    void (*event_cb[BTN_EVENT_SUM])(agile_btn_t *btn); /**< 按键对象事件回调函数 */
    rt_slist_t slist;                                  /**< 单向链表节点 */
};

/**
 * @}
 */

/** @addtogroup AGILE_BUTTON_Exported_Functions
 * @{
 */
#ifdef RT_USING_HEAP
agile_btn_t *agile_btn_create(uint32_t pin, uint32_t active_logic, uint32_t pin_mode);
int agile_btn_delete(agile_btn_t *btn);
#endif

int agile_btn_init(agile_btn_t *btn, uint32_t pin, uint32_t active_logic, uint32_t pin_mode);
int agile_btn_start(agile_btn_t *btn);
int agile_btn_stop(agile_btn_t *btn);
int agile_btn_set_elimination_time(agile_btn_t *btn, uint8_t elimination_time);
int agile_btn_set_hold_cycle_time(agile_btn_t *btn, uint32_t hold_cycle_time);
int agile_btn_set_event_cb(agile_btn_t *btn, enum agile_btn_event event, void (*event_cb)(agile_btn_t *btn));
void agile_btn_process(void);
void agile_btn_env_init(void);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __PKG_AGILE_BUTTON_H */
