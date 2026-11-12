/**
 * @file  ui_memory_usage.h
 * @brief Memory Usage 页面接口
 */

#ifndef UI_MEMORY_USAGE_H
#define UI_MEMORY_USAGE_H

#include "lvgl.h"

/**
 * @brief 在 parent 容器中创建 Memory Usage 页面内容
 * @param parent lv_tabview_add_tab() 返回的 tab 容器
 */
void ui_memory_usage_create(lv_obj_t *parent);
void ui_memory_usage_destroy(void);

#endif /* UI_MEMORY_USAGE_H */
