/**
 * @file  ui_rgb_wave.c
 * @brief RGB565 color band animation drawn directly on the LCD
 */
#include "ui_rgb_wave.h"
#include "lcd_driver.h"

#define WAVE_BANDS      12U
#define WAVE_TOP        36U
#define WAVE_HEIGHT     246U
#define WAVE_BAND_WIDTH 20U
#define WAVE_UPDATE_BANDS 2U

static lv_timer_t *s_timer = NULL;
static uint8_t s_phase = 0U;
static uint8_t s_draw_band = 0U;
static uint8_t s_reverse = 0U;

static const uint16_t s_colors[12] = {
    0xF800U, 0xFC00U, 0xFFE0U, 0x87E0U,
    0x07E0U, 0x07FFU, 0x03FFU, 0x001FU,
    0x781FU, 0xF81FU, 0xF817U, 0xF800U
};

static void wave_draw(void)
{
    uint8_t band;
    for(band = 0U; band < WAVE_BANDS; band++) {
        uint8_t color_index = (uint8_t)((band + s_phase) % WAVE_BANDS);
        lcd_fill((uint16_t)band * WAVE_BAND_WIDTH, WAVE_TOP,
                 WAVE_BAND_WIDTH, WAVE_HEIGHT, s_colors[color_index]);
    }
}

static void wave_timer_cb(lv_timer_t *timer)
{
    uint8_t index;
    (void)timer;
    if(s_reverse) {
        s_phase = (s_phase == 0U) ? (WAVE_BANDS - 1U) : (uint8_t)(s_phase - 1U);
    } else {
        s_phase = (uint8_t)((s_phase + 1U) % WAVE_BANDS);
    }
    for(index = 0U; index < WAVE_UPDATE_BANDS; index++) {
        uint8_t color_index = (uint8_t)((s_draw_band + s_phase) % WAVE_BANDS);
        lcd_fill((uint16_t)s_draw_band * WAVE_BAND_WIDTH, WAVE_TOP,
                 WAVE_BAND_WIDTH, WAVE_HEIGHT, s_colors[color_index]);
        s_draw_band = (uint8_t)((s_draw_band + 1U) % WAVE_BANDS);
    }
}

void ui_rgb_wave_key(void)
{
    s_reverse ^= 1U;
}

void ui_rgb_wave_create(lv_obj_t *parent)
{
    lv_obj_t *label;

    label = lv_label_create(parent);
    lv_label_set_text(label, "RGB WAVE");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 8);

    label = lv_label_create(parent);
    lv_label_set_text(label, "PA4:Reverse  PA0:Back");
    lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x666666), 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);

    s_phase = 0U;
    s_draw_band = 0U;
    s_reverse = 0U;
    lv_refr_now(NULL);
    wave_draw();
    s_timer = lv_timer_create(wave_timer_cb, 100U, NULL);
}

void ui_rgb_wave_destroy(void)
{
    if(s_timer != NULL) {
        lv_timer_del(s_timer);
        s_timer = NULL;
    }
}