#pragma once

#include "lvgl.h"
#include "demos/lv_demos.h"

#include "LVGL_Driver.h"
#include "SD_SPI.h"
#include "Wireless.h"

#define EXAMPLE1_LVGL_TICK_PERIOD_MS  1000


void Lvgl_Example1(void);
void Lvgl_Example1_close(void);
void Lvgl_Example1_hide(void);
void Lvgl_Example1_show(void);
void Lvgl_Set_Screen_Black(void);