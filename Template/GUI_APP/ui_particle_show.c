/**
 * @file  ui_particle_show.c
 * @brief Lightweight falling-snow animation drawn directly on the LCD
 */
#include "ui_particle_show.h"
#include "lcd_driver.h"

#define PARTICLE_COUNT 12U
#define PARTICLE_TOP   38U
#define PARTICLE_BOTTOM 278U

typedef struct {
    uint8_t x;
    uint16_t y;
    uint8_t speed;
    uint8_t radius;
    uint8_t branch_style;
    int8_t drift;
    uint16_t color;
} particle_t;

static particle_t s_particles[PARTICLE_COUNT];
static lv_timer_t *s_timer = NULL;
static uint16_t s_seed = 1U;
static uint8_t s_fast = 0U;

static const uint16_t s_colors[4] = { 0xFFFFU, 0xDFFBU, 0x9E7FU, 0x7DFFU };

static uint16_t random_next(void)
{
    s_seed = (uint16_t)(s_seed * 25173U + 13849U);
    return s_seed;
}

static void particle_reset(particle_t *particle, uint8_t above_screen)
{
    particle->radius = (uint8_t)(2U + (random_next() % 3U));
    particle->x = (uint8_t)(particle->radius + (random_next() % (240U - 2U * particle->radius)));
    particle->y = above_screen ? (uint16_t)(PARTICLE_TOP - (random_next() % 32U)) : PARTICLE_TOP;
    particle->speed = (uint8_t)(1U + (random_next() % 3U));
    particle->branch_style = (uint8_t)(random_next() % 3U);
    particle->drift = (int8_t)(random_next() % 3U) - 1;
    particle->color = s_colors[random_next() % 4U];
}

static void particle_draw(const particle_t *particle, uint16_t color)
{
    uint8_t arm;
    uint16_t start_x = (uint16_t)(particle->x - particle->radius);
    uint16_t start_y = (uint16_t)(particle->y - particle->radius);
    uint16_t diameter = (uint16_t)particle->radius * 2U + 1U;

    lcd_fill(start_x, particle->y, diameter, 1U, color);
    lcd_fill(particle->x, start_y, 1U, diameter, color);

    if(particle->branch_style != 0U) {
        for(arm = 1U; arm <= particle->radius; arm++) {
            lcd_fill((uint16_t)(particle->x - arm), (uint16_t)(particle->y - arm), 1U, 1U, color);
            lcd_fill((uint16_t)(particle->x + arm), (uint16_t)(particle->y - arm), 1U, 1U, color);
            lcd_fill((uint16_t)(particle->x - arm), (uint16_t)(particle->y + arm), 1U, 1U, color);
            lcd_fill((uint16_t)(particle->x + arm), (uint16_t)(particle->y + arm), 1U, 1U, color);
        }
    }
    if(particle->branch_style == 2U && particle->radius > 2U) {
        arm = (uint8_t)(particle->radius - 1U);
        lcd_fill((uint16_t)(particle->x - arm), (uint16_t)(particle->y - arm), 3U, 1U, color);
        lcd_fill((uint16_t)(particle->x - arm), (uint16_t)(particle->y + arm), 3U, 1U, color);
    }
}

static void particle_timer_cb(lv_timer_t *timer)
{
    uint8_t index;
    uint8_t multiplier = s_fast ? 2U : 1U;
    (void)timer;

    for(index = 0U; index < PARTICLE_COUNT; index++) {
        if(s_particles[index].y >= PARTICLE_TOP) {
            particle_draw(&s_particles[index], BLACK);
        }
        s_particles[index].y = (uint16_t)(s_particles[index].y + s_particles[index].speed * multiplier);
        if(s_particles[index].drift < 0 && s_particles[index].x > s_particles[index].radius) {
            s_particles[index].x--;
        } else if(s_particles[index].drift > 0 && s_particles[index].x < (uint8_t)(239U - s_particles[index].radius)) {
            s_particles[index].x++;
        }
        if(s_particles[index].y >= PARTICLE_BOTTOM) {
            particle_reset(&s_particles[index], 1U);
        }
        if(s_particles[index].y >= PARTICLE_TOP) {
            particle_draw(&s_particles[index], s_particles[index].color);
        }
    }
}

void ui_particle_show_key(void)
{
    s_fast ^= 1U;
}

void ui_particle_show_create(lv_obj_t *parent)
{
    uint8_t index;
    lv_obj_t *label;

    label = lv_label_create(parent);
    lv_label_set_text(label, "SNOWFALL");
    lv_obj_set_style_text_color(label, lv_color_hex(0xBFEFFF), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 8);

    label = lv_label_create(parent);
    lv_label_set_text(label, "PA4:Blizzard  PA0:Back");
    lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x666666), 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_refr_now(NULL);
    lcd_fill(0U, PARTICLE_TOP, 240U, 242U, BLACK);
    s_seed = (uint16_t)(lv_tick_get() | 1U);
    s_fast = 0U;
    for(index = 0U; index < PARTICLE_COUNT; index++) {
        particle_reset(&s_particles[index], 1U);
    }
    s_timer = lv_timer_create(particle_timer_cb, 40U, NULL);
}

void ui_particle_show_destroy(void)
{
    if(s_timer != NULL) {
        lv_timer_del(s_timer);
        s_timer = NULL;
    }
}