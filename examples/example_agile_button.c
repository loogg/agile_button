#include <agile_button.h>
#include <stdlib.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

static agile_btn_t *_dbtn = RT_NULL;
static agile_btn_t _sbtn;

static void btn_click_event_cb(agile_btn_t *btn)
{
    rt_kprintf("[button click event] pin:%d   repeat:%d, hold_time:%d\r\n", btn->pin, btn->repeat_cnt, btn->hold_time);
}

static void btn_hold_event_cb(agile_btn_t *btn)
{
    rt_kprintf("[button hold event] pin:%d   hold_time:%d\r\n", btn->pin, btn->hold_time);
}

static void dbtn_create(int argc, char **argv)
{
    int pin = 0;
    int active = 0;

    if (argc < 3) {
        rt_kprintf("dbtn_create      --use dbtn_create [pin] [active]\r\n");
        return;
    }

    pin = atoi(argv[1]);
    active = atoi(argv[2]);

    if (_dbtn) {
        agile_btn_delete(_dbtn);
        _dbtn = RT_NULL;
    }

    if(active == PIN_HIGH)
        _dbtn = agile_btn_create(pin, active, PIN_MODE_INPUT_PULLDOWN);
    else
        _dbtn = agile_btn_create(pin, active, PIN_MODE_INPUT_PULLUP);

    agile_btn_set_event_cb(_dbtn, BTN_CLICK_EVENT, btn_click_event_cb);
    agile_btn_set_event_cb(_dbtn, BTN_HOLD_EVENT, btn_hold_event_cb);
    agile_btn_start(_dbtn);
}

static void dbtn_delete(void)
{
    if (_dbtn) {
        agile_btn_delete(_dbtn);
        _dbtn = RT_NULL;
    }
}

#ifdef RT_USING_FINSH
MSH_CMD_EXPORT(dbtn_create, create btn);
MSH_CMD_EXPORT(dbtn_delete, delete btn);
#endif

static void sbtn_init(int argc, char **argv)
{
    int pin = 0;
    int active = 0;

    if (argc < 3) {
        rt_kprintf("sbtn_init      --use sbtn_init [pin] [active]\r\n");
        return;
    }

    pin = atoi(argv[1]);
    active = atoi(argv[2]);

    agile_btn_stop(&_sbtn);

    if(active == PIN_HIGH)
        agile_btn_init(&_sbtn, pin, active, PIN_MODE_INPUT_PULLDOWN);
    else
        agile_btn_init(&_sbtn, pin, active, PIN_MODE_INPUT_PULLUP);

    agile_btn_set_event_cb(&_sbtn, BTN_CLICK_EVENT, btn_click_event_cb);
    agile_btn_set_event_cb(&_sbtn, BTN_HOLD_EVENT, btn_hold_event_cb);
    agile_btn_start(&_sbtn);
}

#ifdef RT_USING_FINSH
MSH_CMD_EXPORT(sbtn_init, init btn);
#endif
