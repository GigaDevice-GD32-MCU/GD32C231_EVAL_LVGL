/*!
    \file    main.c
    \brief   running led

    \version 2026-02-06, V1.2.0, demo for gd32c2x1
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32c2x1.h"
#include "systick.h"

#include "lcd_driver.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "gd32c231c_eval.h"
#include "gd32c2x1_it.h"
#include "ui.h"

/*
 * PA0 (WakeUp) — 按下高电平（SET）：进入 / 退出页面
 * PA4 (UserKey)— 按下低电平（RESET）：切换到下一页
 */
#define BTN_DEBOUNCE_MS  50U

int main(void)
{
    systick_config();
    lcd_init();

    /* 初始化 LED1~4（不初始化不能操作 GPIO）*/
    gd_eval_led_init(LED1);
    gd_eval_led_init(LED2);
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);
    gd_eval_led_off(LED1);
    gd_eval_led_off(LED2);
    gd_eval_led_off(LED3);
    gd_eval_led_off(LED4);

    /* PA0 使用 EXTI 上升沿；PA4 保持 GPIO 轮询 */
    gd_eval_key_init(KEY_WAKEUP, KEY_MODE_EXTI);
    gd_eval_key_init(KEY_USER,   KEY_MODE_GPIO);  /* PA4 */

    lv_init();
    lv_port_disp_init();
    ui_init();

    uint32_t pa0_last_at = 0U, pa4_changed_at = 0U;
    uint8_t pa4_sample = 0U;
    uint8_t pa4_stable = 0U;

    while(1) {
        uint32_t now = lv_tick_get();

        /* PA4 (UserKey, 低有效) — 切换页面 */
        uint8_t pa4_cur = (gd_eval_key_state_get(KEY_USER) == RESET) ? 1U : 0U;
        if(pa4_cur != pa4_sample) {
            pa4_sample = pa4_cur;
            pa4_changed_at = now;
        }
        if(pa4_sample != pa4_stable && (now - pa4_changed_at) >= BTN_DEBOUNCE_MS) {
            pa4_stable = pa4_sample;
            if(pa4_stable) {
                ui_next_tab();
            }
        }

        /* PA0 (WakeUp, 高有效) — EXTI 只置位，主循环执行页面切换 */
        if(g_pa0_key_request) {
            g_pa0_key_request = 0U;
            if((now - pa0_last_at) >= BTN_DEBOUNCE_MS) {
                pa0_last_at = now;
                ui_enter_exit();
            }
        }

        lv_timer_handler();
        delay_1ms(1);
    }
}
