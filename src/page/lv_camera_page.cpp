#include "lv_page_common.h"

extern bool camera_flag;
extern unsigned char page_idx;

void lv_app_load_camera_scr(void)
{
    page_idx = 0;
    camera_flag = true;
}
