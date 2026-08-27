/**
 * @file  ui_led_running.c
 * @brief LED Running 页面 — LED1~4 跑马灯
 *        PA4: 开关跑马灯   PA0: 退出页面（灯继续跑）
 *        lv_timer 在首次进入时创建，永不删除
 *        退出时只清标签指针，定时器和 LED 状态保留
 */
#include "ui_led_running.h"
#include "gd32c231c_eval.h"

/* ── LED 索引映射 ── */
static const uint8_t s_led_map[4] = { LED1, LED2, LED3, LED4 };

/* ── 持久状态（页面进出不重置）── */
static lv_timer_t *s_led_timer  = NULL;
static uint8_t     s_running    = 0U;  /* 0=停止  1=运行 */
static uint8_t     s_led_idx    = 0U;  /* 当前亮灯 0~3 */

/* ── 页面内标签（退出时清 NULL）── */
static lv_obj_t *s_lbl_led[4]   = { NULL, NULL, NULL, NULL };
static lv_obj_t *s_lbl_status   = NULL;

/* ── 刷新屏幕上的 LED 状态显示 ── */
static void refresh_labels(void)
{
    uint8_t i;
    if(s_lbl_status == NULL) return;  /* 页面已退出，不操作 */
    lv_label_set_text(s_lbl_status, s_running ? "Running" : "Stopped");
    lv_obj_set_style_text_color(s_lbl_status,
        s_running ? lv_color_hex(0x00FF88) : lv_color_hex(0x888888), 0);
    for(i = 0; i < 4U; i++) {
        if(s_lbl_led[i] == NULL) return;
        lv_obj_set_style_text_color(s_lbl_led[i],
            (s_running && i == s_led_idx)
                ? lv_color_hex(0xFFFF00)
                : lv_color_hex(0x444444), 0);
    }
}

/* ── 定时器回调：每 250ms 推进一格 ── */
static void led_timer_cb(lv_timer_t *t)
{
    (void)t;
    if(!s_running) return;
    /* 关当前，推进，开下一个 */
    gd_eval_led_off(s_led_map[s_led_idx]);
    s_led_idx = (uint8_t)((s_led_idx + 1U) % 4U);
    gd_eval_led_on(s_led_map[s_led_idx]);
    refresh_labels();
}

/* ── PA4 按键：切换运行 / 停止 ── */
void ui_led_running_key(void)
{
    s_running ^= 1U;
    if(s_running) {
        /* 启动：点亮当前格 */
        gd_eval_led_on(s_led_map[s_led_idx]);
    } else {
        /* 停止：关掉当前格 */
        gd_eval_led_off(s_led_map[s_led_idx]);
    }
    refresh_labels();
}

void ui_led_running_create(lv_obj_t *parent)
{
    uint8_t i;
    static const lv_coord_t led_yoff[4] = { -60, -20, 20, 60 };
    static const char * const led_name[4] = { "LED1", "LED2", "LED3", "LED4" };
    lv_obj_t *lbl;

    /* 标题 */
    lbl = lv_label_create(parent);
    lv_label_set_text(lbl, "LED Runner");
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 20);

    /* LED 指示标签 */
    for(i = 0; i < 4U; i++) {
        s_lbl_led[i] = lv_label_create(parent);
        lv_label_set_text(s_lbl_led[i], led_name[i]);
        lv_obj_align(s_lbl_led[i], LV_ALIGN_CENTER, 0, led_yoff[i]);
    }

    /* 状态标签 */
    s_lbl_status = lv_label_create(parent);
    lv_obj_align(s_lbl_status, LV_ALIGN_BOTTOM_MID, 0, -30);

    /* 提示 */
    lbl = lv_label_create(parent);
    lv_label_set_text(lbl, "PA4:Start/Stop  PA0:Back");
    lv_obj_set_style_text_font(lbl, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x666666), 0);
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -10);

    /* 定时器：只在首次进入时创建，之后复用 */
    if(s_led_timer == NULL) {
        s_led_timer = lv_timer_create(led_timer_cb, 250U, NULL);
    }

    /* 立即同步显示当前状态 */
    refresh_labels();
}

void ui_led_running_destroy(void)
{
    uint8_t i;
    /* 只清标签指针：定时器继续运行，LED 状态保留 */
    for(i = 0; i < 4U; i++) { s_lbl_led[i] = NULL; }
    s_lbl_status = NULL;
}
