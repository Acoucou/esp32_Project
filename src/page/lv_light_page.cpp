#include "lv_page_common.h"
#include "arduino.h"
#include <Adafruit_NeoPixel.h>

extern unsigned char light_fun;
extern unsigned char page_idx;
extern Adafruit_NeoPixel pixels;

static lv_obj_t *SwitchLed_Switch, *RainbowLed_Switch, *SoundLed_Switch;
static lv_obj_t *led_img;

LV_IMG_DECLARE(led_on);
LV_IMG_DECLARE(led_off);

void SwitchLed_event_handler(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
            lv_img_set_src(led_img, &led_off);
            light_fun = 0;
        }else{
            light_fun = 1;
            lv_img_set_src(led_img, &led_on);
            lv_obj_clear_state(SoundLed_Switch, LV_STATE_CHECKED);
            lv_obj_clear_state(RainbowLed_Switch, LV_STATE_CHECKED);
            // lv_obj_clear_state(SwitchLed_Switch, LV_STATE_CHECKED);
        }
    }
}
void RainbowLed_event_handler(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
            light_fun = 0;
            lv_img_set_src(led_img, &led_off);
        }else{
            light_fun = 2;
            lv_img_set_src(led_img, &led_on);
            lv_obj_clear_state(SoundLed_Switch, LV_STATE_CHECKED);
            //lv_obj_clear_state(RainbowLed_Switch, LV_STATE_CHECKED);
            lv_obj_clear_state(SwitchLed_Switch, LV_STATE_CHECKED);
        }
    }
}
void SoundLed_event_handler(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)){
            light_fun = 0;
            lv_img_set_src(led_img, &led_off);
        }else{
            light_fun = 3;
            lv_img_set_src(led_img, &led_on);
            // lv_obj_clear_state(SoundLed_Switch, LV_STATE_CHECKED);
            lv_obj_clear_state(RainbowLed_Switch, LV_STATE_CHECKED);
            lv_obj_clear_state(SwitchLed_Switch, LV_STATE_CHECKED);
        }
    }
}

void lv_app_load_light_scr(){
    page_idx = 3;

    pixels.begin(); // This initializes the NeoPixel library.

    lv_obj_t* light_scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(light_scr, lv_color_make(0, 0, 0), 0);

    static lv_style_t font_20_label_style;
    lv_style_init(&font_20_label_style);
    lv_style_set_text_font(&font_20_label_style, &myfont);

    lv_obj_t * tital_label = lv_label_create(light_scr);
    lv_label_set_recolor(tital_label, true);//颜色重绘
    lv_obj_add_style(tital_label, &font_20_label_style, 0);
    lv_label_set_text(tital_label, "#0000ff 灯光控制#");
    lv_obj_align(tital_label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * SwitchLed_label = lv_label_create(light_scr);
    lv_label_set_recolor(SwitchLed_label, true);//颜色重绘
    lv_obj_add_style(SwitchLed_label, &font_20_label_style, 0);
    lv_label_set_text(SwitchLed_label, "#ffffff 彩虹灯#");
    lv_obj_align_to(SwitchLed_label, tital_label, LV_ALIGN_LEFT_MID, -60, 50);

    lv_obj_t * RainbowLed_label = lv_label_create(light_scr);
    lv_label_set_recolor(RainbowLed_label, true);//颜色重绘
    lv_obj_add_style(RainbowLed_label, &font_20_label_style, 0);
    lv_label_set_text(RainbowLed_label, "#ffffff 流水灯#");
    lv_obj_align_to(RainbowLed_label, tital_label, LV_ALIGN_LEFT_MID, -60, 100);

    lv_obj_t * SoundLed_label = lv_label_create(light_scr);
    lv_label_set_recolor(SoundLed_label, true);//颜色重绘
    lv_obj_add_style(SoundLed_label, &font_20_label_style, 0);
    lv_label_set_text(SoundLed_label, "#ffffff 律动灯#");
    lv_obj_align_to(SoundLed_label, tital_label, LV_ALIGN_LEFT_MID, -60, 150);

    // 开关
    SwitchLed_Switch = lv_switch_create(light_scr);
    lv_obj_add_event_cb(SwitchLed_Switch, SwitchLed_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_align_to(SwitchLed_Switch, tital_label, LV_ALIGN_LEFT_MID, 10, 50);
    lv_obj_clear_state(SwitchLed_Switch, LV_STATE_CHECKED);

    RainbowLed_Switch = lv_switch_create(light_scr);
    lv_obj_add_event_cb(RainbowLed_Switch, RainbowLed_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_align_to(RainbowLed_Switch, tital_label, LV_ALIGN_LEFT_MID, 10, 100);
    lv_obj_clear_state(RainbowLed_Switch, LV_STATE_CHECKED);

    SoundLed_Switch = lv_switch_create(light_scr);
    lv_obj_add_event_cb(SoundLed_Switch, SoundLed_event_handler, LV_EVENT_PRESSED, NULL);
    lv_obj_align_to(SoundLed_Switch, tital_label, LV_ALIGN_LEFT_MID, 10, 150);
    lv_obj_clear_state(SoundLed_Switch, LV_STATE_CHECKED);

    // 退出
    lv_obj_t* exit_btn = lv_btn_create(light_scr);
    lv_obj_remove_style_all(exit_btn);
    lv_obj_set_size(exit_btn, 1, 1);
    lv_obj_set_align(exit_btn, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_add_event_cb(exit_btn, app_exit_event_handler, LV_EVENT_PRESSED, NULL);

    // 图片
    led_img = lv_img_create(light_scr);
    lv_img_set_src(led_img, &led_off);
    lv_obj_align(led_img, LV_ALIGN_RIGHT_MID, 20, -5);
	
    lv_scr_load_anim(light_scr, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0, true);
}
