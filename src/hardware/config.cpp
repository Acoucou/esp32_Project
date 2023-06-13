#include "config.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eFEX fex = TFT_eFEX(&tft);

static lv_disp_draw_buf_t draw_buf;    // 定义显示器变量
static lv_color_t buf[TFT_WIDTH * 35]; // 定义刷新缓存

// 你的热点
const char *ssid = "CouCou";
const char *password = "12345678";

// 获取时间
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 3600;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();                                        // 使能写功能
  tft.setAddrWindow(area->x1, area->y1, w, h);             // 设置填充区域
  tft.pushColors((uint16_t *)&color_p->full, w * h, true); // 写入颜色缓存和缓存大小
  tft.endWrite();                                          // 关闭写功能

  lv_disp_flush_ready(disp); // 调用区域填充颜色函数
}

void lv_disp_init()
{
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 35);
  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;

  lv_disp_drv_register(&disp_drv);

  // lv_color_t c = lv_palette_main(LV_PALETTE_LIGHT_BLUE);
  // lv_color_t c1 = lv_palette_main(LV_PALETTE_YELLOW);
  // lv_disp_t *display = NULL;
  // lv_theme_t * th = lv_theme_default_init(display,  /*Use the DPI, size, etc from this display*/ 
  //                                       c, c1,   /*Primary and secondary palette*/
  //                                       false,    /*Light or dark mode*/ 
  //                                       &lv_font_montserrat_14); /*Small, normal, large fonts*/
                                      
  // lv_disp_set_theme(display, th); /*Assign the theme to the display*/
}

void tft_init()
{
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  fex.listSPIFFS();
  tft.setRotation(1);
}

void wifi_init()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected: " + WiFi.localIP());
}

void all_init(void)
{
  Serial.begin(115200);

  camera_init();

  tft_init();

  // mqtt_init();

  lv_disp_init();

  lv_port_indev_init(); // keyboard

  wifi_init();

  /*
    初始化时间
  */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // 更新天气
  lv_app_weather_update();

  
}
