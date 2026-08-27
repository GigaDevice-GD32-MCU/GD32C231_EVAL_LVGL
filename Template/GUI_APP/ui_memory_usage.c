/**
 * @file  ui_memory_usage.c
 * @brief Memory Usage 页面 — 实时监控：堆占用率 / 堆剩余 / FPS / 运行时长
 *        注意：无动画，避免退出时动画回调访问已释放对象导致崩溃
 */
#include "ui_memory_usage.h"

static lv_obj_t  *s_row[4] = {NULL, NULL, NULL, NULL};
static lv_timer_t *s_timer = NULL;

/* uint32_t 转字符串，返回末尾指针 */
static char *u32s(uint32_t v, char *p)
{
    char tmp[10];
    uint8_t n = 0U;
    if(v == 0U) { *p++ = '0'; return p; }
    while(v > 0U) { tmp[n++] = (char)('0' + (uint8_t)(v % 10U)); v /= 10U; }
    while(n > 0U) { *p++ = tmp[--n]; }
    return p;
}

static void mem_update_cb(lv_timer_t *t)
{
    static lv_mem_monitor_t mon;  /* static：不占运行时栈 */
    char buf[24];
    char *p;
    (void)t;

    /* 页面已退出时不操作，防释放后访问 */
    if(s_row[0] == NULL) return;

    lv_mem_monitor(&mon);

    /* Heap: XX% */
    p = buf;
    { const char *s = "Heap: "; while(*s) *p++ = *s++; }
    p = u32s((uint32_t)mon.used_pct, p); *p++ = '%'; *p = '\0';
    lv_label_set_text(s_row[0], buf);

    /* Free: XXXXb */
    p = buf;
    { const char *s = "Free: "; while(*s) *p++ = *s++; }
    p = u32s(mon.free_size, p); *p++ = 'b'; *p = '\0';
    lv_label_set_text(s_row[1], buf);

    /* Frag: XX% */
    p = buf;
    { const char *s = "Frag: "; while(*s) *p++ = *s++; }
    p = u32s((uint32_t)mon.frag_pct, p); *p++ = '%'; *p = '\0';
    lv_label_set_text(s_row[2], buf);

    /* Up:   XXXs */
    p = buf;
    { const char *s = "Up:   "; while(*s) *p++ = *s++; }
    p = u32s(lv_tick_get() / 1000UL, p); *p++ = 's'; *p = '\0';
    lv_label_set_text(s_row[3], buf);
}

void ui_memory_usage_create(lv_obj_t *parent)
{
    lv_obj_t *lbl;
    uint8_t i;
    static const uint32_t colors[4] = {
        0x00FF88UL, 0x44AAFFULL, 0xFFCC00UL, 0xFF8844UL
    };
    static const lv_coord_t yoff[4] = { -55, -15, 25, 65 };

    lbl = lv_label_create(parent);
    lv_label_set_text(lbl, "Memory Usage");
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 20);

    for(i = 0; i < 4U; i++) {
        s_row[i] = lv_label_create(parent);
        lv_label_set_text(s_row[i], "...");
        lv_obj_set_style_text_color(s_row[i], lv_color_hex(colors[i]), 0);
        lv_obj_align(s_row[i], LV_ALIGN_CENTER, 0, yoff[i]);
    }

    lbl = lv_label_create(parent);
    lv_label_set_text(lbl, "PA0: Back");
    lv_obj_set_style_text_font(lbl, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x666666), 0);
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -10);

    /* 启动定时器，1s 刷新一次，不在此处直接调用回调 */
    s_timer = lv_timer_create(mem_update_cb, 1000, NULL);
}

void ui_memory_usage_destroy(void)
{
    uint8_t i;
    /* 顺序：先删 timer（禁止回调），再清 NULL（防释放后访问） */
    if(s_timer != NULL) {
        lv_timer_del(s_timer);
        s_timer = NULL;
    }
    for(i = 0; i < 4U; i++) { s_row[i] = NULL; }
}
