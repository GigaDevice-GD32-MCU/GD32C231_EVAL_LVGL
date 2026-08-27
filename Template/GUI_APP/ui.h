/**
 * @file  ui.h
 * @brief 全局UI配置、样式和翻译接口
 */

#ifndef UI_H
#define UI_H

#include "lvgl.h"

/* 初始化所有屏幕和UI对象，须在 lv_port_disp_init() 之后调用 */
void ui_init(void);

/* PA4 按下：切换到下一个页面选项 */
void ui_next_tab(void);

/* PA0 按下：进入当前选中页面，或从页面返回菜单 */
void ui_enter_exit(void);

#endif /* UI_H */
