/*
 * Copyright 2016  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <app.h>
#include <unistd.h>
#include <app_manager.h>
#include <signal.h>
#include <feedback.h>
#include <notification.h>
#include <app_preference.h>
#include <wifi.h>
#include <device/display.h>
#include <device/callback.h>
#include <system_settings.h>
#include <runtime_info.h>
#include <vconf.h>

#include "common.h"
#include "box.h"
#include "icon.h"
#include "main.h"
#include "indicator_gui.h"
#include "modules.h"
#include "util.h"
#include "toast_popup.h"
#include "tts.h"
#include "log.h"
#include "indicator.h"
#include "ticker.h"
#include "bg_color.h"

#define GRP_NAME "indicator"
#define WIN_TITLE "Illume Indicator"
#define VCONF_PHONE_STATUS "memory/startapps/sequence"

#define BUS_NAME       "org.tizen.system.deviced"
#define PATH_NAME    "/Org/Tizen/System/DeviceD/Display"
#define INTERFACE_NAME BUS_NAME".display"
#define MEMBER_NAME	"LCDOn"

#define MP_APP_ID "org.tizen.music-player-lite"
#define FMRADIO_APP_ID "org.tizen.fm-radio-lite"
#define VR_APP_ID "org.tizen.voicerecorder-lite"

#define STR_ATOM_MV_INDICATOR_GEOMETRY          "_E_MOVE_INDICATOR_GEOMETRY"

#define HIBERNATION_ENTER_NOTI	"HIBERNATION_ENTER"
#define HIBERNATION_LEAVE_NOTI	"HIBERNATION_LEAVE"

#define UNLOCK_ENABLED	0
#define TIMEOUT			5

#define ERROR_MESSAGE_LEN 256

Evas_Coord_Point indicator_press_coord = {0,0};
Ecore_Timer *clock_timer;
int is_transparent = 0;
int current_angle = 0;
int current_state = 0;

static struct _s_info {
	Ecore_Timer *listen_timer;
} s_info = {
	.listen_timer = NULL,
};


static indicator_error_e _start_indicator(void *data);
static indicator_error_e _terminate_indicator(void *data);

static void _indicator_low_bat_cb(app_event_info_h event_info, void *data);
static void _indicator_lang_changed_cb(app_event_info_h event_info, void *data);
static void _indicator_region_changed_cb(app_event_info_h event_info, void *data);
static void _indicator_window_delete_cb(void *data, Evas_Object * obj, void *event);


static void _indicator_low_bat_cb(app_event_info_h event_info, void *data)
{
}

static void _indicator_lang_changed_cb(app_event_info_h event_info, void *data)
{
	modules_lang_changed(data);

	char *language_set = vconf_get_str(VCONFKEY_LANGSET);
	if (language_set) {
		_D("Lanugaged changed: %s", language_set);
		elm_language_set(language_set);
		free(language_set);
	}
}

static void _indicator_region_changed_cb(app_event_info_h event_info, void *data)
{
	modules_region_changed(data);
}

static void _indicator_window_delete_cb(void *data, Evas_Object * obj, void *event)
{
	ret_if(!data);

	_terminate_indicator((struct appdata *)data);
}

#define SIGNAL_NAME_LEN 30
static void _indicator_notify_pm_state_cb(device_callback_e type, void *value, void *user_data)
{
	display_state_e state;

	ret_if(!user_data);

	if (type != DEVICE_CALLBACK_DISPLAY_STATE)
		return;

	state = (display_state_e)value;

	switch (state) {
	case DISPLAY_STATE_SCREEN_OFF:
		if (clock_timer != NULL) {
			ecore_timer_del(clock_timer);
			clock_timer = NULL;
		}
		icon_set_update_flag(0);
		box_noti_ani_handle(0);
		break;
	case DISPLAY_STATE_SCREEN_DIM:
		icon_set_update_flag(0);
		box_noti_ani_handle(0);
		break;
	case DISPLAY_STATE_NORMAL:
		if (!icon_get_update_flag()) {
			icon_set_update_flag(1);
			box_noti_ani_handle(1);
			modules_wake_up(user_data);
		}
		break;
	default:
		break;
	}
}

static void _indicator_lock_status_cb(system_settings_key_e key, void *data)
{
	static int lockstate = 0;
	extern int clock_mode;
	int val = -1;

	ret_if(!data);

	int err = system_settings_get_value_int(SYSTEM_SETTINGS_KEY_LOCK_STATE, &val);
	if (err != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_get_value_int failed: %s", get_error_message(err));
		return;
	}

	if (val == lockstate) return;
	lockstate = val;

	switch (val) {
	case SYSTEM_SETTINGS_LOCK_STATE_UNLOCK:
		if (!clock_mode) util_signal_emit(data,"clock.font.12","indicator.prog");
		else util_signal_emit(data,"clock.font.24","indicator.prog");
		break;
	case SYSTEM_SETTINGS_LOCK_STATE_LAUNCHING_LOCK:
	case SYSTEM_SETTINGS_LOCK_STATE_LOCK:
		util_signal_emit(data,"clock.invisible","indicator.prog");
		break;
	default:
		break;
	}
}

static void _indicator_ecore_evas_msg_parent_handle(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
	ret_if(!data);

#ifdef _SUPPORT_SCREEN_READER
	if (msg_domain == MSG_DOMAIN_CONTROL_ACCESS) {
		struct appdata *ad = (struct appdata *)ecore_evas_data_get(ee,"indicator_app_data");

		ret_if(!ad);

		Elm_Access_Action_Info *action_info;
		Evas_Object* win = NULL;
		action_info = data;

		win = ad->win.win;

		if (msg_id == ELM_ACCESS_ACTION_ACTIVATE) {
			elm_access_action(win, action_info->action_type,action_info);
		} else if (msg_id == ELM_ACCESS_ACTION_HIGHLIGHT_NEXT) {
			action_info->highlight_cycle = EINA_TRUE;
			elm_access_action(win,action_info->action_type,action_info);
		} else if (msg_id == ELM_ACCESS_ACTION_HIGHLIGHT_PREV) {
			action_info->highlight_cycle = EINA_TRUE;
			elm_access_action(win,action_info->action_type,action_info);
		} else if (msg_id == ELM_ACCESS_ACTION_UNHIGHLIGHT) {
			elm_access_action(win,action_info->action_type,action_info);
		} else if (msg_id == ELM_ACCESS_ACTION_READ) {
			elm_access_action(win,action_info->action_type,action_info);
		}
	}
#endif /* _SUPPORT_SCREEN_READER */
}

static void _register_event_handler_both(win_info *win, void *data)
{
	Ecore_Evas *ee;

	ret_if(!win);

	ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win->win));

	evas_object_smart_callback_add(win->win,"delete,request", _indicator_window_delete_cb, data);
	ecore_evas_callback_msg_parent_handle_set(ee, _indicator_ecore_evas_msg_parent_handle);
	ecore_evas_data_set(ee,"indicator_app_data",data);
}


static void register_event_handler(void *data)
{
	struct appdata *ad = data;

	ret_if(!data);

	ad->active_indi_win = -1;

	_register_event_handler_both(&(ad->win),data);

	int err = device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE, _indicator_notify_pm_state_cb, ad);
	if (err != DEVICE_ERROR_NONE) {
		_E("device_add_callback failed: %s", get_error_message(err));
	}

	if (util_system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCK_STATE, _indicator_lock_status_cb, ad)) {
		_E("util_system_settings_set_changed_cb failed");
	}

}

static void _unregister_event_handler_both(win_info *win)
{
	ret_if(!win);

	evas_object_smart_callback_del(win->win, "delete-request", _indicator_window_delete_cb);
}

static int unregister_event_handler(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	retv_if(!data, 0);

	_unregister_event_handler_both(&(ad->win));

	device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE, _indicator_notify_pm_state_cb);
	util_system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCK_STATE, _indicator_lock_status_cb);

	Ecore_Event_Handler *hdl = NULL;
	EINA_LIST_FREE(ad->evt_handlers, hdl) {
		if (hdl) ecore_event_handler_del(hdl);
	}


	return OK;
}

static void _create_layout(struct appdata *ad, const char *file, const char *group)
{
	ad->win.layout = elm_layout_add(ad->win.win);
	ret_if(!ad->win.layout);

	if (EINA_FALSE == elm_layout_file_set(ad->win.layout, file, group)) {
		_E("Failed to set file of layout");
		evas_object_del(ad->win.layout);
		return;
	}

	evas_object_size_hint_min_set(ad->win.layout, ad->win.w, ad->win.h);
	/* FIXME */
	evas_object_size_hint_weight_set(ad->win.layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win.win, ad->win.layout);
	evas_object_move(ad->win.layout, 0, 0);
	evas_object_show(ad->win.layout);
}

static void _create_box(win_info *win)
{
	ret_if(!win);

	/* First, clear layout */
	box_fini(win);

	box_init(win);

	return;
}

static Eina_Bool _render_force(void *data)
{
	elm_win_render(data);
	// below do not work - why??
	// evas_render(evas_object_evas_get(data));
	return EINA_FALSE;
}

static void _indicator_service_cb(void *data, tzsh_indicator_service_h service,
		int angle, int opacity)
{
	win_info *info = data;

	switch (angle) {
		case 0:
		case 180:
			_D("Enable indicator portrait mode: %d %d", info->port_w, INDICATOR_HEIGHT);
			evas_object_resize(info->win, info->port_w, INDICATOR_HEIGHT);
			break;
		case 90:
		case 270:
			_D("Enable indicator landscape mode: %d %d", info->land_w, INDICATOR_HEIGHT);
			evas_object_resize(info->win, info->land_w, INDICATOR_HEIGHT);
			break;
		default:
			_E("Unahandled rotation value");
			return;
	}
	info->angle = angle;
	// this line below is making indicator to appear, however there are some
	// rendering artifcats, so we delay rendering
	// elm_win_render(info->win);
	ecore_timer_add(0.1, _render_force, info->win);
}

int indicator_tzsh_init(win_info *info)
{
	info->tzsh = tzsh_create(TZSH_TOOLKIT_TYPE_EFL);
	if (!info->tzsh) {
		_E("tzsh_create failed for TZSH_TOOLKIT_TYPE_EFL");
		return -1;
	}
	info->service = tzsh_indicator_service_create(info->tzsh, elm_win_window_id_get(info->win));
	if (!info->service) {
		_E("tzsh_indicator_service_create failed");
		tzsh_destroy(info->tzsh);
		info->tzsh = NULL;
		return -1;
	}
	int err = tzsh_indicator_service_property_change_cb_set(info->service,
			_indicator_service_cb, info);
	if (err != TZSH_ERROR_NONE) {
		_E("tzsh_indicator_service_property_change_cb_set failed[%d]: %s",
				err, get_error_message(err));
		tzsh_indicator_service_destroy(info->service);
		tzsh_destroy(info->tzsh);
		info->tzsh = NULL;
		info->service = NULL;
		return -1;
	}

	_D("Successfully created tzsh indicator service");
	return 0;
}

void indicator_tzsh_shutdown(win_info *info)
{
	if (info->service) tzsh_indicator_service_destroy(info->service);
	if (info->tzsh) tzsh_destroy(info->tzsh);
}

static Eina_Bool _indicator_listen_timer_cb(void* data)
{
	win_info *win = data;

	retv_if(!win, ECORE_CALLBACK_CANCEL);


	if (!elm_win_socket_listen(win->win , INDICATOR_SERVICE_NAME, 0, EINA_FALSE)) {
		_E("failed to elm_win_socket_listen() %x", win->win);
		return ECORE_CALLBACK_RENEW;
	} else {
		_D("listen success");
		s_info.listen_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
}

static void _create_window(struct appdata *ad)
{
	Evas_Object *dummy_win = NULL;

	_D("Create window");

	ad->win.win = elm_win_add(NULL, "indicator", ELM_WIN_SOCKET_IMAGE);
	ret_if(!(ad->win.win));

	dummy_win = elm_win_add(NULL, "indicator_dummy", ELM_WIN_BASIC);
	if (dummy_win) {
		elm_win_screen_size_get(dummy_win, NULL, NULL, &ad->win.port_w, &ad->win.land_w);
		evas_object_del(dummy_win);
		_D("Dummy window w, h (%d, %d)", ad->win.port_w, ad->win.land_w);
	} else {
		_E("Critical error. Cannot create dummy window");
	}

	if (!elm_win_socket_listen(ad->win.win , INDICATOR_SERVICE_NAME, 0, EINA_FALSE)) {
		_E("Failed 1st to elm_win_socket_listen() %x", ad->win.win);

		if (s_info.listen_timer != NULL) {
			ecore_timer_del(s_info.listen_timer);
			s_info.listen_timer = NULL;
		}
		s_info.listen_timer = ecore_timer_add(3, _indicator_listen_timer_cb, &(ad->win));
	}

	elm_win_alpha_set(ad->win.win , EINA_TRUE);
	/* FIXME */
	elm_win_borderless_set(ad->win.win , EINA_TRUE);
	evas_object_size_hint_fill_set(ad->win.win , EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->win.win , 1.0, 0.5);

	evas_object_resize(ad->win.win, ad->win.port_w, INDICATOR_HEIGHT);
	_D("w,h(%d,%d)", ad->win.port_w, INDICATOR_HEIGHT);

	evas_object_show(ad->win.win);
	if (indicator_tzsh_init(&ad->win))
		_E("indicator_tzsh_init failed. Indicator support for device rotation will not be available.");
}

static void _create_base_gui(void* data)
{
	struct appdata *ad = data;

	ret_if(!ad);

	_D("Start to create base gui");

	_create_window(ad);

	/* FIXME */
	ad->win.h = INDICATOR_HEIGHT;
	ad->win.w = ad->win.port_w;
	ad->win.evas = evas_object_evas_get(ad->win.win);

	_D("win_size = Original(%d, %d), Scaled(%lf, %lf)", ad->win.port_w, ad->win.h, ELM_SCALE_SIZE(ad->win.port_w), ELM_SCALE_SIZE(ad->win.h));

	_create_layout(ad, util_get_res_file_path(EDJ_FILE), GRP_NAME);
	_create_box(&(ad->win));


#if 0 /* For test */
	Evas_Object *rect = evas_object_rectangle_add(ad->win.evas);
	ret_if(!rect);
	evas_object_resize(rect, 720, 52);
	evas_object_color_set(rect, 0, 0, 255, 255);
	evas_object_show(rect);
	evas_object_layer_set(rect, -256);
#endif
	ad->win.data = data;

	return;
}

static void _init_win_info(void * data)
{
	struct appdata *ad = data;

	ret_if(!ad);

	memset(&(ad->win),0x00,sizeof(win_info));
}

static void _init_tel_info(void * data)
{
	struct appdata *ad = data;

	ret_if(!ad);

	memset(&(ad->tel_info), 0x00, sizeof(telephony_info));
}

static indicator_error_e _start_indicator(void *data)
{
	retv_if(!data, INDICATOR_ERROR_INVALID_PARAMETER);

	_init_win_info(data);
	_init_tel_info(data);

	/* Create indicator window */
	_create_base_gui(data);

	return INDICATOR_ERROR_NONE;
}

static indicator_error_e _terminate_indicator(void *data)
{
	struct appdata *ad = data;

	retv_if(!ad, INDICATOR_ERROR_INVALID_PARAMETER);

	modules_fini(data);
	unregister_event_handler(ad);

	box_fini(&(ad->win));

	if (ad->win.evas)
		evas_image_cache_flush(ad->win.evas);

	if (ad->win.layout) {
		evas_object_del(ad->win.layout);
		ad->win.layout = NULL;
	}

	if (ad->win.win) {
		evas_object_del(ad->win.win);
		ad->win.win = NULL;
	}


	if (ad)
		free(ad);

	elm_exit();

	return INDICATOR_ERROR_NONE;
}

static void _signal_handler(int signum, siginfo_t *info, void *unused)
{
    ui_app_exit();
}

static bool app_create(void *data)
{
	struct appdata *ad = data;
	int ret;

	retv_if(!ad, false);

	elm_app_base_scale_set(2.6);

	/* Signal handler */
	struct sigaction act;
	memset(&act, 0x00, sizeof(struct sigaction));
	act.sa_sigaction = _signal_handler;
	act.sa_flags = SA_SIGINFO;

	ret = sigemptyset(&act.sa_mask);
	if (ret < 0) {
		char error_message[ERROR_MESSAGE_LEN] = {0,};
		strerror_r(errno, error_message, ERROR_MESSAGE_LEN);
		_E("Failed to sigemptyset[%s]", error_message);
	}
	ret = sigaddset(&act.sa_mask, SIGTERM);
	if (ret < 0) {
		char error_message[ERROR_MESSAGE_LEN] = {0,};
		strerror_r(errno, error_message, ERROR_MESSAGE_LEN);
		_E("Failed to sigaddset[%s]", error_message);
	}
	ret = sigaction(SIGTERM, &act, NULL);
	if (ret < 0) {
		char error_message[ERROR_MESSAGE_LEN] = {0,};
		strerror_r(errno, error_message, ERROR_MESSAGE_LEN);
		_E("Failed to sigaction[%s]", error_message);
	}

	ret = _start_indicator(ad);
	if (ret != INDICATOR_ERROR_NONE) {
		_D("Failed to create a new window!");
	}

	/* Set nonfixed-list size for display */
	modules_init_first(ad);

	if (ad->win.win) {
		elm_win_activate(ad->win.win);
	}
	evas_object_show(ad->win.layout);
	evas_object_show(ad->win.win);

	return true;
}

static void app_terminate(void *data)
{
	struct appdata *ad = data;
	modules_fini(data);
	ticker_fini(ad);
	indicator_toast_popup_fini();
	message_port_unregister();
#ifdef _SUPPORT_SCREEN_READER2
	indicator_service_tts_fini(data);
#endif

	unregister_event_handler(ad);

	feedback_deinitialize();

	box_fini(&(ad->win));
	evas_image_cache_flush(ad->win.evas);
	evas_object_del(ad->win.layout);
	indicator_tzsh_shutdown(&ad->win);
	evas_object_del(ad->win.win);

	_D("INDICATOR IS TERMINATED");
}

static void app_pause(void *data)
{
}

static void app_resume(void *data)
{
}

static void app_service(app_control_h service, void *data)
{
	struct appdata *ad = data;

	_D("INDICATOR IS STARTED");

	register_event_handler(ad);
	modules_init(data);
#ifdef _SUPPORT_SCREEN_READER
	modules_register_tts(data);
#endif
	feedback_initialize();
	indicator_toast_popup_init(data);
	message_port_register(data);
	if (INDICATOR_ERROR_NONE != ticker_init(ad)) {
		_E("Ticker cannot initialize");
	}
#ifdef _SUPPORT_SCREEN_READER2
	indicator_service_tts_init(data);
#endif
	_indicator_lock_status_cb(SYSTEM_SETTINGS_KEY_LOCK_STATE, data);
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	int ret = 0;

	_D("Start indicator");

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_service;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, _indicator_low_bat_cb, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, _indicator_lang_changed_cb, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, _indicator_region_changed_cb, NULL);

	memset(&ad, 0x0, sizeof(struct appdata));

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		_E("app_main() is failed. err = %d", ret);
	}

	return ret;
}

/* End of file */
