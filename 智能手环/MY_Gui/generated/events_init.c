/*
* Copyright 2024 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"


static void battery_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_SCREEN_LOADED:
	{
		//Write animation: battery_setting_info move in x direction
		lv_anim_t battery_setting_info_anim_x;
		lv_anim_init(&battery_setting_info_anim_x);
		lv_anim_set_var(&battery_setting_info_anim_x, guider_ui.battery_setting_info);
		lv_anim_set_time(&battery_setting_info_anim_x, 800);
		lv_anim_set_delay(&battery_setting_info_anim_x, 0);
		lv_anim_set_exec_cb(&battery_setting_info_anim_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
		lv_anim_set_values(&battery_setting_info_anim_x, lv_obj_get_x(guider_ui.battery_setting_info), 10);
		lv_anim_set_path_cb(&battery_setting_info_anim_x, &lv_anim_path_bounce);
		battery_setting_info_anim_x.repeat_cnt = 1;
		lv_anim_start(&battery_setting_info_anim_x);
		//Write animation: battery_setting_info move in y direction
		lv_anim_t battery_setting_info_anim_y;
		lv_anim_init(&battery_setting_info_anim_y);
		lv_anim_set_var(&battery_setting_info_anim_y, guider_ui.battery_setting_info);
		lv_anim_set_time(&battery_setting_info_anim_y, 800);
		lv_anim_set_delay(&battery_setting_info_anim_y, 0);
		lv_anim_set_exec_cb(&battery_setting_info_anim_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
		lv_anim_set_values(&battery_setting_info_anim_y, lv_obj_get_y(guider_ui.battery_setting_info), 20);
		lv_anim_set_path_cb(&battery_setting_info_anim_y, &lv_anim_path_bounce);
		battery_setting_info_anim_y.repeat_cnt = 1;
		lv_anim_start(&battery_setting_info_anim_y);
		break;
	}
	default:
		break;
	}
}
void events_init_battery(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->battery, battery_event_handler, LV_EVENT_ALL, NULL);
}

void events_init(lv_ui *ui)
{

}
