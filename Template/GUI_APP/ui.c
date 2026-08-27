/**
 * @file  ui.c
 * @brief 页面管理：菜单选择 + 五个全屏页面
 *        PA4 → ui_next_tab()   切换当前高亮选项
 *        PA0 → ui_enter_exit() 进入选中页面 / 返回菜单
 */

#include "ui.h"
#include "ui_memory_usage.h"
#include "ui_LED_Running.h"
#include "ui_snake.h"
#include "ui_rgb_wave.h"
#include "ui_particle_show.h"

/* ── 页面数量 ── */
#define PAGE_COUNT  5U

/* ── 状态 ── */
static uint8_t s_page      = 0U;   /* 0=Memory 1=LED 2=Snake 3=RGB Wave 4=Particles */
static uint8_t s_in_detail = 0U;   /* 0=菜单中  1=页面详情中 */

/* ── LVGL 对象 ── */
static lv_obj_t *s_menu_cont;
static lv_obj_t *s_menu_items[PAGE_COUNT];
static lv_obj_t *s_page_cont = NULL;

/* ── 页面名称 ── */
static const char * const s_names[PAGE_COUNT] = {
    "Memory Usage", "LED_Running", "Snake", "RGB Wave", "Snowfall"
};

/* 菜单项纵向偏移（相对屏幕中心，单位 px）*/
static const int16_t s_ypos[PAGE_COUNT] = { -70, -35, 0, 35, 70 };

/* 刷新菜单高亮：选中项绿色，其余白色 */
static void menu_refresh(void)
{
    uint8_t i;
    for(i = 0; i < PAGE_COUNT; i++) {
        lv_obj_set_style_text_color(s_menu_items[i],
            (i == s_page) ? lv_color_hex(0x00FF00) : lv_color_white(), 0);
    }
}

void ui_init(void)
{
    uint8_t i;
    lv_obj_t *scr = lv_scr_act();

    /* 屏幕黑底（黑色 = 背景键色，由 disp_flush 替换为渐变） */
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* ── 菜单容器 ── */
    s_menu_cont = lv_obj_create(scr);
    lv_obj_set_size(s_menu_cont, 240, 320);
    lv_obj_set_pos(s_menu_cont, 0, 0);
    lv_obj_set_style_bg_color(s_menu_cont, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_menu_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_menu_cont, 0, 0);
    lv_obj_set_style_pad_all(s_menu_cont, 0, 0);
    lv_obj_set_style_radius(s_menu_cont, 0, 0);
    lv_obj_clear_flag(s_menu_cont, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title = lv_label_create(s_menu_cont);
    lv_label_set_text(title, "SELECT PAGE");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    /* 菜单项 */
    for(i = 0; i < PAGE_COUNT; i++) {
        s_menu_items[i] = lv_label_create(s_menu_cont);
        lv_label_set_text(s_menu_items[i], s_names[i]);
        lv_obj_align(s_menu_items[i], LV_ALIGN_CENTER, 0, s_ypos[i]);
    }
    menu_refresh();

    /* 底部提示 */
    lv_obj_t *hint = lv_label_create(s_menu_cont);
    lv_label_set_text(hint, "PA4:Switch  PA0:Enter");
    lv_obj_set_style_text_font(hint, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x666666), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void ui_next_tab(void)
{
    if(s_in_detail) {
        if(s_page == 1U) { ui_led_running_key(); }
        else if(s_page == 2U) { ui_snake_key(); }
        else if(s_page == 3U) { ui_rgb_wave_key(); }
        else if(s_page == 4U) { ui_particle_show_key(); }
        return;
    }
    s_page = (uint8_t)((s_page + 1U) % PAGE_COUNT);
    menu_refresh();
}

void ui_enter_exit(void)
{
    if(!s_in_detail) {
        lv_obj_t *scr = lv_scr_act();
        /* Snake / RGB Wave / Snowfall 直接用 lcd_fill 写屏，绘图区之外由 LVGL 背景填充。
         * 纯黑会被 disp_flush 的键色替换成渐变，导致上下露出主界面背景，
         * 故这三页改用"近黑"(RGB565 下与纯黑同色)，避开键色替换。 */
        lv_color_t page_bg = (s_page >= 2U) ? lv_color_hex(0x000008) : lv_color_black();

        s_page_cont = lv_obj_create(scr);
        lv_obj_set_size(s_page_cont, 240, 320);
        lv_obj_set_pos(s_page_cont, 0, 0);
        lv_obj_set_style_bg_color(s_page_cont, page_bg, 0);
        lv_obj_set_style_bg_opa(s_page_cont, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(s_page_cont, 0, 0);
        lv_obj_set_style_pad_all(s_page_cont, 0, 0);
        lv_obj_set_style_radius(s_page_cont, 0, 0);
        lv_obj_clear_flag(s_page_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(s_menu_cont, LV_OBJ_FLAG_HIDDEN);
        switch(s_page) {
            case 0U: ui_memory_usage_create(s_page_cont); break;
            case 1U: ui_led_running_create(s_page_cont);  break;
            case 2U: ui_snake_create(s_page_cont);        break;
            case 3U: ui_rgb_wave_create(s_page_cont);     break;
            default: ui_particle_show_create(s_page_cont); break;
        }
        s_in_detail = 1U;
    } else {
        switch(s_page) {
            case 0U: ui_memory_usage_destroy(); break;
            case 1U: ui_led_running_destroy();  break;
            case 2U: ui_snake_destroy();        break;
            case 3U: ui_rgb_wave_destroy();     break;
            default: ui_particle_show_destroy(); break;
        }
        lv_obj_del(s_page_cont);
        s_page_cont = NULL;
        lv_obj_clear_flag(s_menu_cont, LV_OBJ_FLAG_HIDDEN);
        s_in_detail = 0U;
    }
}
