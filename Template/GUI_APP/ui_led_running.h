/**
 * @file  ui_led_running.h
 * @brief LED Running 页面接口
 */

#ifndef UI_LED_RUNNING_H
#define UI_LED_RUNNING_H

#include "lvgl.h"

/**
 * @brief 在 parent 容器中创建 LED Running 页面内容
 * @param parent lv_tabview_add_tab() 返回的 tab 容器
 */
void ui_led_running_create(lv_obj_t *parent);
void ui_led_running_destroy(void);
/* PA4 按下时由 ui_next_tab 转发调用 */
void ui_led_running_key(void);

#endif /* UI_LED_RUNNING_H */
