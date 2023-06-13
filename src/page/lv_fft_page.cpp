#include "lv_page_common.h"
#include <Arduino.h>

lv_obj_t * chart;
lv_chart_series_t * fft_ser;
extern bool fft_flag;
extern unsigned char page_idx;

void lv_app_load_fft_scr(void)
{
    page_idx = 4;
    
    chart = lv_chart_create(NULL);
    lv_obj_set_size(chart, 240, 240);
    lv_chart_set_type(chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(chart, 50);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(chart, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_MAIN);
    lv_obj_set_style_line_color(chart, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_chart_set_div_line_count(chart, 5, 5);

    lv_obj_t *exit_btn = lv_btn_create(chart);
    lv_obj_remove_style_all(exit_btn);
    lv_obj_set_size(exit_btn, 1, 1);
    lv_obj_set_align(exit_btn, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_event_cb(exit_btn, (lv_event_cb_t)app_exit_event_handler, LV_EVENT_PRESSED, NULL);


    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0 ,100);
    
    fft_ser = lv_chart_add_series(chart, lv_palette_lighten(LV_PALETTE_GREEN, 2), LV_CHART_AXIS_PRIMARY_Y);
    
    fft_flag = true;
    lv_scr_load_anim(chart, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, true);
}