#include "light.h"

#define PIN 0
#define NOISE 50 // 噪音底线
#define NUMPIXELS 14

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void sound_light(){
  int ADC_VALUE[100];
  int last_VALUE = 0;
  int ADC_sort[10];
  int light_ADC = 0;

  int ADC_compute = 0;
  for (int i = 0; i < 2; i++)
  {
    ADC_compute += analogRead(A10);
  }
  last_VALUE = ADC_compute >> 1;

  //Serial.println(last_VALUE);
  
  int num = last_VALUE;

  // 平均值滤波，10个，上面
  // light_ADC = last_VALUE;
  // light_ADC = abs(last_VALUE - 1500);

  // light_ADC = (light_ADC <= NOISE) ? 0 : (light_ADC - 30);

  //Serial.println(light_ADC);

  // num = light_ADC / 8;
  // if (num >= 30)
  //   num = 10 + (num >> 3);
  // else if (num >= 14)
  //   num = 10 + (num >> 2);

  num = constrain(num, 850, 1300); 
  num = map(num, 850, 1300, 1, 14); 
  
  for (int i = 0; i < num; i++)
  {
    last_VALUE = i * 30;
    if (i <= 8)
    {
      pixels.setPixelColor(i, pixels.Color(last_VALUE, 255 - last_VALUE, 0)); // Moderately bright green color.
      pixels.show();                                              // This sends the updated pixel color to the hardware.
      delay(10);
    }
    else
    {
      pixels.setPixelColor(i, pixels.Color(50, 10, 255 - last_VALUE)); // Moderately bright green color.
      pixels.show();                                                     // This sends the updated pixel color to the hardware.
      delay(10);
    }
  }
  // delay(10);
  for (int i = num; i >= 0; i--)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Moderately bright green color.
    pixels.show();                                  // This sends the updated pixel color to the hardware.
    delay(1);
  }
}

// 彩虹沿整个带状循环
void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 1*65536; firstPixelHue += 256) {
    for(int i=0; i<pixels.numPixels(); i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show(); // 更新当前带内容
    delay(wait);  // 暂停wait毫秒
  }
}

void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // 第一个像素从红色开始(色调0)
  for(int a=0; a<30; a++) {  // 重复30次
    for(int b=0; b<3; b++) { 
      pixels.clear();         //   将RAM中的所有像素设置为0(关闭)
      for(int c=b; c<pixels.numPixels(); c += 3) {
        int  hue   = firstPixelHue + c * 65536L / pixels.numPixels();
        uint32_t color = pixels.gamma32(pixels.ColorHSV(hue));
        pixels.setPixelColor(c, color); 
      }
      pixels.show();              
      delay(wait);               
      firstPixelHue += 65536 / 90; 
    }
  }
}

