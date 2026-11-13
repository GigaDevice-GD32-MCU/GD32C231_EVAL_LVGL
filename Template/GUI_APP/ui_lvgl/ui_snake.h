/**
 * @file  ui_snake.h
 * @brief Snake 贪吃蛇游戏页面接口
 */

#ifndef UI_SNAKE_H
#define UI_SNAKE_H

#include "lvgl.h"

void ui_snake_create(lv_obj_t *parent);
void ui_snake_destroy(void);
void ui_snake_key(void);  /* PA4 按下：转向 */

#endif /* UI_SNAKE_H */
