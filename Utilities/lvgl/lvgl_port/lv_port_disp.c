/**
 * @file lv_port_disp.c
 * @brief LVGL display port for GD32C231C-EVAL (2.2" SPI LCD, 240x320, ILI9341)
 */

#include "lv_port_disp.h"
#include "lcd_driver.h"

/* Display resolution */
#define DISP_HOR_RES    240
#define DISP_VER_RES    320

/* Display buffer - 3 lines to balance performance and LVGL heap usage */
#define DISP_BUF_LINES  3
static lv_color_t disp_buf_1[DISP_HOR_RES * DISP_BUF_LINES];
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t      disp_drv;

/* DMA 异步刷新：保存 drv 指针，DMA 完成后通知 LVGL */
static lv_disp_drv_t *s_flush_drv = NULL;

/* ────────── 背景渐变 + 透视网格（程序化生成，不占用图片数据） ──────────
 * 效果：四角双线性插值的对角渐变 —— 左上浅蓝 / 右上亮青 / 左下浅蓝紫 / 右下浅紫，
 *       叠加下半部向消失点汇聚的透视网格线，模拟 GD32 宣传图的观感。
 * 原理：LVGL 侧背景保持纯黑，flush 时把黑色像素替换为按坐标计算的颜色。
 *       前景（文字/图形）为非黑色，原样保留。
 * 平滑：RGB565 色阶太少会有色带，用 4x4 Bayer 有序抖动打散量化误差。
 * 性能：沿 x 用 8.8 定点增量累加（DDA），内循环每像素仅 3 次加法，无乘除。
 */
#define BG_GRAD_ENABLE      1
#define BG_GRID_ENABLE      1
#define BG_KEY_COLOR        0x0000U     /* 被替换的键色：纯黑 */
#define BG_MAX_LEVEL        248         /* 通道上限，留出抖动/提亮余量 */

/* 四角颜色 (8bit RGB)，各通道需 <= BG_MAX_LEVEL
 * 色相取自 GD32 宣传图：左上深蓝 → 右上亮青光晕 → 左下蓝紫 → 右下亮紫 */
static const uint8_t s_corner[4][3] = {
    {  10U,  20U,  38U },   /* 0 = 左上：深蓝   (参考 #14284B × 0.5) */
    {  56U, 110U, 118U },   /* 1 = 右上：青光晕 (参考 #6FDCEC × 0.5) */
    {  23U,  21U,  47U },   /* 2 = 左下：蓝紫   (参考 #2E2A5E × 0.5) */
    {  81U,  48U, 100U },   /* 3 = 右下：紫     (参考 #A15FC8 × 0.5) */
};

/* 4x4 Bayer 抖动矩阵，值域 0~15（16 字节常量） */
static const uint8_t s_bayer4[16] = {
     0U,  8U,  2U, 10U,
    12U,  4U, 14U,  6U,
     3U, 11U,  1U,  9U,
    15U,  7U, 13U,  5U
};

/* 定点魔数：避免除法。BG_KY ≈ 65536/DISP_VER_RES，BG_KX ≈ 65536/DISP_HOR_RES */
#define BG_KY   ((int32_t)(65536L / DISP_VER_RES))          /* 320 → 204 */
#define BG_KX   ((int32_t)(65536L / DISP_HOR_RES))          /* 240 → 273 */

#if BG_GRID_ENABLE
#define BG_GRID_VX      120         /* 消失点 X */
#define BG_GRID_VY      170         /* 消失点 Y（在网格区上方，竖线在此汇聚） */
#define BG_GRID_TOP     205         /* 网格起始行 */
#define BG_GRID_LINES   11U         /* 竖线条数（奇数，含中线） */
#define BG_GRID_SLOPE   68          /* 相邻竖线斜率步进（8.8 定点） */
#define BG_GRID_LIFT    40          /* 网格线相对背景的提亮量 (8bit) */

/* 横线所在行：透视效果下间距向下递增（16 字节常量） */
static const uint16_t s_grid_rows[8] = { 208U, 214U, 222U, 232U, 245U, 262U, 284U, 312U };

static uint8_t bg_clamp(int32_t v)
{
    return (uint8_t)((v > BG_MAX_LEVEL) ? BG_MAX_LEVEL : v);
}
#endif

/* 就地把 buffer 中的键色像素替换为背景 */
static void bg_grad_apply(const lv_area_t *area, uint16_t *px)
{
    int16_t y;

    for(y = area->y1; y <= area->y2; y++) {
        const uint8_t *bay = &s_bayer4[(y & 3) * 4];
        int32_t acc[3];     /* 8.8 定点：当前像素的 R/G/B */
        int32_t stp[3];     /* 8.8 定点：每前进一列的增量 */
        uint8_t ch;
        int16_t x;
#if BG_GRID_ENABLE
        int16_t gx[BG_GRID_LINES];  /* 本行各竖线的 x 坐标（递增） */
        uint8_t gi       = 0U;      /* 扫描到的下一条竖线 */
        uint8_t gn       = 0U;      /* 本行竖线数，0 = 不在网格区 */
        uint8_t row_line = 0U;      /* 本行是否为横线 */
#endif

        for(ch = 0U; ch < 3U; ch++) {
            /* 先沿 y 插出本行左右两端的颜色（8.8 定点） */
            int32_t l = ((int32_t)s_corner[0][ch] << 8) +
                        ((((int32_t)s_corner[2][ch] - (int32_t)s_corner[0][ch]) * y * BG_KY) >> 8);
            int32_t r = ((int32_t)s_corner[1][ch] << 8) +
                        ((((int32_t)s_corner[3][ch] - (int32_t)s_corner[1][ch]) * y * BG_KY) >> 8);
            /* 再算沿 x 的每列增量，并推进到本次刷新区域的起始列 */
            stp[ch] = ((r - l) * BG_KX) >> 16;
            acc[ch] = l + stp[ch] * area->x1;
        }

#if BG_GRID_ENABLE
        if(y >= BG_GRID_TOP) {
            int16_t dy = (int16_t)(y - BG_GRID_VY);
            uint8_t i;
            gn = BG_GRID_LINES;
            /* 竖线从消失点发散：x = VX + slope * dy */
            for(i = 0U; i < BG_GRID_LINES; i++) {
                int32_t k = ((int32_t)i - (int32_t)(BG_GRID_LINES / 2U)) * BG_GRID_SLOPE;
                gx[i] = (int16_t)(BG_GRID_VX + ((k * dy) >> 8));
            }
            for(i = 0U; i < 8U; i++) {
                if(s_grid_rows[i] == (uint16_t)y) { row_line = 1U; break; }
            }
        }
#endif

        /* R/B 是 5bit（量化步长 8）→ 抖动量 d>>1；G 是 6bit（步长 4）→ d>>2 */
        for(x = area->x1; x <= area->x2; x++, px++) {
            if(*px == BG_KEY_COLOR) {
#if BG_GRID_ENABLE
                uint8_t on_line = row_line;
                while((gi < gn) && (gx[gi] < x)) { gi++; }
                if((gi < gn) && (gx[gi] == x)) { on_line = 1U; }

                if(on_line) {
                    *px = lv_color_make(bg_clamp((acc[0] >> 8) + BG_GRID_LIFT),
                                        bg_clamp((acc[1] >> 8) + BG_GRID_LIFT),
                                        bg_clamp((acc[2] >> 8) + BG_GRID_LIFT)).full;
                }
                else
#endif
                {
                    uint8_t d = bay[x & 3];
                    *px = lv_color_make((uint8_t)((acc[0] >> 8) + (d >> 1)),
                                        (uint8_t)((acc[1] >> 8) + (d >> 2)),
                                        (uint8_t)((acc[2] >> 8) + (d >> 1))).full;
                }
            }
            acc[0] += stp[0];
            acc[1] += stp[1];
            acc[2] += stp[2];
        }
    }
}

/**
 * @brief 由 DMA 完成中断调用，通知 LVGL flush 完毕
 */
void lv_port_disp_flush_done(void)
{
    if(s_flush_drv != NULL) {
        lv_disp_flush_ready(s_flush_drv);
        s_flush_drv = NULL;
    }
}

/**
 * @brief Flush callback - 启动 DMA，完成中断后释放 LVGL 缓冲
 */
static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

#if BG_GRAD_ENABLE
    bg_grad_apply(area, (uint16_t *)color_p);
#endif

    s_flush_drv = drv;
    lcd_copy(area->x1, area->y1, w, h, (uint16_t *)color_p);
}

/**
 * @brief Initialize the LVGL display driver
 */
void lv_port_disp_init(void)
{
    /* Initialize draw buffer */
    lv_disp_draw_buf_init(&disp_buf, disp_buf_1, NULL, DISP_HOR_RES * DISP_BUF_LINES);

    /* Initialize display driver */
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &disp_buf;

    /* Register the driver */
    lv_disp_drv_register(&disp_drv);
}
