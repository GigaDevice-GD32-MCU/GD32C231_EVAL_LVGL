/**
 * @file lv_port_disp.h
 * @brief LVGL display port header for GD32C231C-EVAL
 */

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#include "lvgl.h"

void lv_port_disp_init(void);
void lv_port_disp_flush_done(void);  /* 由 DMA_Channel2_IRQHandler 调用 */

#endif /* LV_PORT_DISP_H */
