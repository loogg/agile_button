#include <agile_button.h>
#include <drv_gpio.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#define WK_UP_KEY_PIN  GET_PIN(A, 0)
#define KEY0_PIN       GET_PIN(C, 1)
#define KEY1_PIN       GET_PIN(C, 13)

static agile_btn_t *wk_up_key = RT_NULL;
static agile_btn_t *key0 = RT_NULL;
static agile_btn_t *key1 = RT_NULL;

static void btn_click_event_cb(agile_btn_t *btn)
{
    rt_kprintf("[button click event] pin:%d   repeat:%d, hold_time:%d\r\n", btn->pin, btn->repeat_cnt, btn->hold_time);
}

static void btn_hold_event_cb(agile_btn_t *btn)
{
    rt_kprintf("[button hold event] pin:%d   hold_time:%d\r\n", btn->pin, btn->hold_time);
}

static void key_create(void)
{
    if(wk_up_key == RT_NULL)
    {
        wk_up_key = agile_btn_create(WK_UP_KEY_PIN, PIN_HIGH, PIN_MODE_INPUT_PULLDOWN);
        agile_btn_set_event_cb(wk_up_key, BTN_CLICK_EVENT, btn_click_event_cb);
        agile_btn_set_event_cb(wk_up_key, BTN_HOLD_EVENT, btn_hold_event_cb);
        agile_btn_start(wk_up_key);
    }

    if(key0 == RT_NULL)
    {
        key0 = agile_btn_create(KEY0_PIN, PIN_LOW, PIN_MODE_INPUT_PULLUP);
        agile_btn_set_event_cb(key0, BTN_CLICK_EVENT, btn_click_event_cb);
        agile_btn_set_event_cb(key0, BTN_HOLD_EVENT, btn_hold_event_cb);
        agile_btn_start(key0);
    }

    if(key1 == RT_NULL)
    {
        key1 = agile_btn_create(KEY1_PIN, PIN_LOW, PIN_MODE_INPUT_PULLUP);
        agile_btn_set_event_cb(key1, BTN_CLICK_EVENT, btn_click_event_cb);
        agile_btn_set_event_cb(key1, BTN_HOLD_EVENT, btn_hold_event_cb);
        agile_btn_start(key1);
    }
}

static void key_delete(void)
{
    if(wk_up_key)
    {
        agile_btn_delete(wk_up_key);
        wk_up_key = RT_NULL;
    }

    if(key0)
    {
        agile_btn_delete(key0);
        key0 = RT_NULL;
    }

    if(key1)
    {
        agile_btn_delete(key1);
        key1 = RT_NULL;
    }
}

#ifdef RT_USING_FINSH
MSH_CMD_EXPORT(key_create, create key);
MSH_CMD_EXPORT(key_delete, delete key);
#endif
