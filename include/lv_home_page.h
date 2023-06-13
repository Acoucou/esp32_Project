#ifndef __LV_HOME_PAGE_H
#define __LV_HOME_PAGE_H


#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

void lv_app_load_home_scr(void);

void lv_user_weather_update(lv_obj_t *home_scr);
void lv_user_time_update(lv_obj_t *home_scr);

#endif