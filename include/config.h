#ifndef __CONFIG_H
#define __CONFIG_H

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <TFT_eFEX.h>
#include <WiFi.h>

#include "lvgl.h"
#include "lv_port_indev.h"

#include "OV2640.h"
#include "camera.h"
#include "light.h"
#include "mqtt.h"
#include "lv_page_common.h"

void wifi_init();
void tft_init();
void lv_disp_init();
void all_init(void);


#endif 
