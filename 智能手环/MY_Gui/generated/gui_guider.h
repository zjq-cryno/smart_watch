/*
* Copyright 2024 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *battery;
	bool battery_del;
	lv_obj_t *battery_setting_info;
	lv_obj_t *battery_label_10;
	lv_obj_t *battery_label_11;
	lv_obj_t *battery_img_1;
	lv_obj_t *battery_label_12;
	lv_obj_t *battery_img_2;
	lv_obj_t *battery_bar_1;
	lv_obj_t *battery_label_13;
	lv_obj_t *battery_img_3;
	lv_obj_t *battery_bar_2;
	lv_obj_t *battery_label_14;
	lv_obj_t *battery_label_15;
	lv_obj_t *battery_label_16;
	lv_obj_t *battery_right;
	lv_obj_t *battery_left;
}lv_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;

void setup_scr_battery(lv_ui *ui);

LV_IMG_DECLARE(_img_bg_2_240x280);
LV_IMG_DECLARE(_img_chart_battery_alpha_197x59);
LV_IMG_DECLARE(_icn_fitness_24x24);
LV_IMG_DECLARE(_icn_travel_24x24);
LV_IMG_DECLARE(_img_arrow_right_alpha_5x22);
LV_IMG_DECLARE(_img_arrow_left_alpha_5x22);

LV_FONT_DECLARE(lv_font_montserratMedium_10)
LV_FONT_DECLARE(lv_font_montserratMedium_14)


#ifdef __cplusplus
}
#endif
#endif
