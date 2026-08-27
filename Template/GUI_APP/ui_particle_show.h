/**
 * @file  ui_particle_show.h
 * @brief Snowfall visual page interface
 */

#ifndef UI_PARTICLE_SHOW_H
#define UI_PARTICLE_SHOW_H

#include "lvgl.h"

void ui_particle_show_create(lv_obj_t *parent);
void ui_particle_show_destroy(void);
void ui_particle_show_key(void);

#endif /* UI_PARTICLE_SHOW_H */