/**
 * @file  ui_snake.c
 * @brief 贪吃蛇游戏 — 直接 LCD 绘制，不使用 LVGL 控件渲染游戏区域
 *        PA4: 顺时针转向   PA0: 退出
 *        游戏区域: 16×20 格 (每格 15×16 像素 = 240×320)
 */
#include "ui_snake.h"
#include "lcd_driver.h"

/* ── 游戏网格参数 ── */
#define GRID_W      16U
#define GRID_H      18U     /* 留2行给顶部分数显示 */
#define CELL_W      15U
#define CELL_H      15U
#define GRID_Y_OFF  32U     /* 顶部偏移(像素)，给分数留空 */

/* ── 蛇最大长度 ── */
#define SNAKE_MAX   32U

/* ── 方向 ── */
#define DIR_UP      0U
#define DIR_RIGHT   1U
#define DIR_DOWN    2U
#define DIR_LEFT    3U

/* ── 颜色 ── */
#define COL_BG      BLACK
#define COL_SNAKE   GREEN
#define COL_HEAD    YELLOW
#define COL_FOOD    RED
#define COL_BORDER  GRAY2

/* ── 游戏状态 ── */
static uint8_t s_snake_x[SNAKE_MAX];
static uint8_t s_snake_y[SNAKE_MAX];
static uint8_t s_len      = 0U;
static uint8_t s_dir      = DIR_RIGHT;
static uint8_t s_food_x   = 0U;
static uint8_t s_food_y   = 0U;
static uint16_t s_score   = 0U;
static uint8_t s_running  = 0U;    /* 1=运行中 0=游戏结束 */

static lv_obj_t  *s_lbl_score = NULL;
static lv_obj_t  *s_lbl_info  = NULL;
static lv_timer_t *s_timer    = NULL;

/* ── 简单伪随机 ── */
static uint16_t s_rng = 12345U;
static uint8_t rng_next(uint8_t max)
{
    s_rng = (uint16_t)(s_rng * 25173U + 13849U);
    return (uint8_t)(s_rng % max);
}

/* ── 绘制一个格子 ── */
static void draw_cell(uint8_t gx, uint8_t gy, uint16_t color)
{
    uint16_t px = (uint16_t)gx * CELL_W;
    uint16_t py = GRID_Y_OFF + (uint16_t)gy * CELL_H;
    lcd_fill(px, py, CELL_W, CELL_H, color);
}

/* ── 刷新分数 ── */
static void refresh_score(void)
{
    char buf[16];
    char *p = buf;
    char tmp[6];
    uint8_t n = 0U;
    uint16_t v = s_score;
    const char *s = "Score:";
    while(*s) *p++ = *s++;
    if(v == 0U) { *p++ = '0'; }
    else {
        while(v > 0U) { tmp[n++] = (char)('0' + (uint8_t)(v % 10U)); v /= 10U; }
        while(n > 0U) { *p++ = tmp[--n]; }
    }
    *p = '\0';
    if(s_lbl_score) lv_label_set_text(s_lbl_score, buf);
}

/* ── 放置食物 ── */
static void place_food(void)
{
    uint8_t i, collision;
    do {
        s_food_x = rng_next((uint8_t)GRID_W);
        s_food_y = rng_next((uint8_t)GRID_H);
        collision = 0U;
        for(i = 0; i < s_len; i++) {
            if(s_snake_x[i] == s_food_x && s_snake_y[i] == s_food_y) {
                collision = 1U;
                break;
            }
        }
    } while(collision);
    draw_cell(s_food_x, s_food_y, COL_FOOD);
}

/* ── 绘制整条蛇 ── */
static void draw_snake_full(void)
{
    uint8_t i;
    for(i = 0; i < s_len; i++) {
        draw_cell(s_snake_x[i], s_snake_y[i], (i == 0) ? COL_HEAD : COL_SNAKE);
    }
}

/* ── 游戏 tick ── */
static void snake_tick_cb(lv_timer_t *t)
{
    
    uint8_t nx, ny, i;
    (void)t;

    if(!s_running) return;

    /* 计算新头部位置 */
    nx = s_snake_x[0];
    ny = s_snake_y[0];
    switch(s_dir) {
        case DIR_UP:    ny = (ny == 0U) ? (uint8_t)(GRID_H - 1U) : (uint8_t)(ny - 1U); break;
        case DIR_DOWN:  ny = (uint8_t)((ny + 1U) % GRID_H); break;
        case DIR_LEFT:  nx = (nx == 0U) ? (uint8_t)(GRID_W - 1U) : (uint8_t)(nx - 1U); break;
        case DIR_RIGHT: nx = (uint8_t)((nx + 1U) % GRID_W); break;
        default: break;
    }

    /* 碰撞检测：撞自己 */
    for(i = 0; i < s_len; i++) {
        if(s_snake_x[i] == nx && s_snake_y[i] == ny) {
            /* Game Over */
            s_running = 0U;
            if(s_lbl_info) {
                lv_label_set_text(s_lbl_info, "GAME OVER! PA0:Exit");
                lv_obj_set_style_text_color(s_lbl_info, lv_color_hex(0xFF0000), 0);
            }
            return;
        }
    }

    /* 吃到食物？ */
    if(nx == s_food_x && ny == s_food_y) {
        /* 增长 */
        if(s_len < SNAKE_MAX) {
            /* 尾部后移一格腾出空间 */
            for(i = s_len; i > 0U; i--) {
                s_snake_x[i] = s_snake_x[i - 1U];
                s_snake_y[i] = s_snake_y[i - 1U];
            }
            s_len++;
        } else {
            /* 已满，正常移动 */
            draw_cell(s_snake_x[s_len - 1U], s_snake_y[s_len - 1U], COL_BG);
            for(i = s_len - 1U; i > 0U; i--) {
                s_snake_x[i] = s_snake_x[i - 1U];
                s_snake_y[i] = s_snake_y[i - 1U];
            }
        }
        s_snake_x[0] = nx;
        s_snake_y[0] = ny;
        s_score++;
        refresh_score();
        place_food();
    } else {
        /* 正常移动：擦尾，移体，画头 */
        draw_cell(s_snake_x[s_len - 1U], s_snake_y[s_len - 1U], COL_BG);
        for(i = s_len - 1U; i > 0U; i--) {
            s_snake_x[i] = s_snake_x[i - 1U];
            s_snake_y[i] = s_snake_y[i - 1U];
        }
        s_snake_x[0] = nx;
        s_snake_y[0] = ny;
    }

    /* 画蛇头和原第二节(变色) */
    draw_cell(s_snake_x[0], s_snake_y[0], COL_HEAD);
    if(s_len > 1U) {
        draw_cell(s_snake_x[1], s_snake_y[1], COL_SNAKE);
    }
    /* 始终重绘食物，防止被 LVGL 覆盖 */
    draw_cell(s_food_x, s_food_y, COL_FOOD);
}

/* ── PA4 按键：顺时针转向 ── */
void ui_snake_key(void)
{
    if(s_running) {
        s_dir = (uint8_t)((s_dir + 1U) % 4U);
    }
}

/* ── 创建游戏页面 ── */
void ui_snake_create(lv_obj_t *parent)
{
    uint8_t i;

    /* 分数 label (LVGL) */
    s_lbl_score = lv_label_create(parent);
    lv_label_set_text(s_lbl_score, "Score:0");
    lv_obj_set_style_text_color(s_lbl_score, lv_color_hex(0x00FF88), 0);
    lv_obj_align(s_lbl_score, LV_ALIGN_TOP_LEFT, 5, 5);

    /* 提示 label */
    s_lbl_info = lv_label_create(parent);
    lv_label_set_text(s_lbl_info, "PA4:Turn PA0:Exit");
    lv_obj_set_style_text_font(s_lbl_info, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(s_lbl_info, lv_color_hex(0x666666), 0);
    lv_obj_align(s_lbl_info, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_refr_now(NULL);
    lcd_fill(0, GRID_Y_OFF, 240, (uint16_t)(GRID_H * CELL_H), COL_BG);

    /* 初始化蛇 (长度3，居中向右) */
    s_len = 3U;
    s_dir = DIR_RIGHT;
    s_score = 0U;
    s_running = 1U;
    s_rng = (uint16_t)(lv_tick_get() & 0xFFFFU);

    for(i = 0; i < s_len; i++) {
        s_snake_x[i] = (uint8_t)(GRID_W / 2U - i);
        s_snake_y[i] = (uint8_t)(GRID_H / 2U);
    }

    draw_snake_full();
    place_food();

    /* 游戏定时器：200ms/tick */
    s_timer = lv_timer_create(snake_tick_cb, 200U, NULL);
}

/* ── 销毁游戏页面 ── */
void ui_snake_destroy(void)
{
    if(s_timer != NULL) {
        lv_timer_del(s_timer);
        s_timer = NULL;
    }
    s_lbl_score = NULL;
    s_lbl_info = NULL;
    s_running = 0U;
}
