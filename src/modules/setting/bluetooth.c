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
#include <stdlib.h>
#include <bluetooth.h>
#include <vconf.h>
#include "common.h"
#include "indicator.h"
#include "icon.h"
#include "modules.h"
#include "main.h"
#include "util.h"
#include "log.h"

#define ICON_PRIORITY	INDICATOR_PRIORITY_CONNECTION_SYSTEM_1
#define MODULE_NAME		"bluetooth"
#define TIMER_INTERVAL	0.5

Ecore_Timer *timer_bt = NULL;

static int register_bluetooth_module(void *data);
static int unregister_bluetooth_module(void);
static int wake_up_cb(void *data);
static void show_image_icon(void *data, int index);
static void hide_image_icon(void);
#ifdef _SUPPORT_SCREEN_READER
static char *access_info_cb(void *data, Evas_Object *obj);
#endif


icon_s bluetooth = {
	.type = INDICATOR_IMG_ICON,
	.name = MODULE_NAME,
	.priority = ICON_PRIORITY,
	.always_top = EINA_FALSE,
	.exist_in_view = EINA_FALSE,
	.img_obj = {0,},
	.obj_exist = EINA_FALSE,
	.area = INDICATOR_ICON_AREA_CONNECTION_SYSTEM,
	.init = register_bluetooth_module,
	.fini = unregister_bluetooth_module,
	.wake_up = wake_up_cb,
#ifdef _SUPPORT_SCREEN_READER
	.tts_enable = EINA_TRUE,
	.access_cb = access_info_cb
#endif
};

#define NO_DEVICE			(0x00)
#define HEADSET_CONNECTED	(0x01)
#define DEVICE_CONNECTED	(0x02)
#define DATA_TRANSFER		(0x04)

enum {
	LEVEL_MIN = 0,
	LEVEL_BT_ON = LEVEL_MIN,
	LEVEL_BT_CONNECTED,
	LEVEL_BT_HEADSET,
	LEVEL_MAX
};

static const char *icon_path[LEVEL_MAX] = {
	[LEVEL_BT_ON] = "Bluetooth, NFC, GPS/b03_bt_on_connected.png",
	[LEVEL_BT_CONNECTED] = "Bluetooth, NFC, GPS/b03_bt_activated_on.png",
	[LEVEL_BT_HEADSET] = "Bluetooth, NFC, GPS/b03_bt_on_connected&headset.png",
};

static int updated_while_lcd_off = 0;
static int prevIndex = -1;

static void set_app_state(void* data)
{
	bluetooth.ad = data;
}

static void show_image_icon(void *data, int index)
{
	if (index < LEVEL_MIN || index >= LEVEL_MAX)
		index = LEVEL_MIN;

	if(prevIndex == index)
		return;

	bluetooth.img_obj.data = icon_path[index];
	icon_show(&bluetooth);

	prevIndex = index;
}

static void hide_image_icon(void)
{
	icon_hide(&bluetooth);

	prevIndex = -1;
	util_signal_emit(bluetooth.ad,"indicator.bluetooth.hide","indicator.prog");
}

static void show_bluetooth_icon(void *data, int status)
{
	if (status == NO_DEVICE) {
		show_image_icon(data, LEVEL_BT_ON);
		return;
	}

	if (status & HEADSET_CONNECTED)
		show_image_icon(data, LEVEL_BT_HEADSET);

	else if (status & DEVICE_CONNECTED)
		show_image_icon(data, LEVEL_BT_CONNECTED);

	return;
}


static void indicator_bluetooth_change_cb(keynode_t *node, void *data)
{
	_D("indicator_bluetooth_change_cb");
	int dev = 0;
	int ret = 0;
	int result = NO_DEVICE;
	bt_adapter_state_e adapter_state = BT_ADAPTER_DISABLED;

	retm_if(!data, "Invalid parameter!");

	if (icon_get_update_flag() == 0) {
		updated_while_lcd_off = 1;
		return;
	}
	updated_while_lcd_off = 0;

	ret = bt_adapter_get_state(&adapter_state);
	retm_if(ret != BT_ERROR_NONE, "bt_adapter_get_state failed");
	if (adapter_state != BT_ADAPTER_ENABLED) {  // If adapter_state is NULL. hide_image_icon().
		_D("BT is not enabled.");
		hide_image_icon();
		return;
	}

	ret = vconf_get_int(VCONFKEY_BT_DEVICE, &dev);
	if (ret == OK) {
		DBG("Show BT ICON (BT DEVICE: %d)", dev);

		if (dev == VCONFKEY_BT_DEVICE_NONE) {
			show_bluetooth_icon(data, NO_DEVICE);
			return;
		}
		if ((dev & VCONFKEY_BT_DEVICE_HEADSET_CONNECTED) ||
		    (dev & VCONFKEY_BT_DEVICE_A2DP_HEADSET_CONNECTED)) {
			result = HEADSET_CONNECTED;
			DBG("BT_HEADSET_CONNECTED(%x)", result);
		}
		if ((dev & VCONFKEY_BT_DEVICE_SAP_CONNECTED)
		|| (dev & VCONFKEY_BT_DEVICE_PBAP_CONNECTED)
		|| (dev & VCONFKEY_BT_DEVICE_HID_CONNECTED)
		|| (dev & VCONFKEY_BT_DEVICE_PAN_CONNECTED)) {
			result = (result | DEVICE_CONNECTED);
			DBG("BT_DEVICE_CONNECTED(%x)", result);
		}
		show_bluetooth_icon(data, result);
	} else
		hide_image_icon();
	return;
}

static int wake_up_cb(void *data)
{
	if(updated_while_lcd_off == 0 && bluetooth.obj_exist == EINA_FALSE) {
		return OK;
	}

	indicator_bluetooth_change_cb(NULL, data);
	return OK;
}

#ifdef _SUPPORT_SCREEN_READER
static char *access_info_cb(void *data, Evas_Object *obj)
{
	char *tmp = NULL;
	char buf[256] = {0,};

	switch(prevIndex)
	{
		case LEVEL_BT_ON:
			snprintf(buf, sizeof(buf), "%s, %s", _("IDS_IDLE_BODY_BLUETOOTH_ON"),_("IDS_IDLE_BODY_STATUS_BAR_ITEM"));
			break;
		case LEVEL_BT_CONNECTED:
			snprintf(buf, sizeof(buf), "%s, %s", _("Bluetooth On and Connected"),_("IDS_IDLE_BODY_STATUS_BAR_ITEM"));
			break;
		case LEVEL_BT_HEADSET:
			snprintf(buf, sizeof(buf), "%s, %s", _("Bluetooth On and Connected headset"),_("IDS_IDLE_BODY_STATUS_BAR_ITEM"));
			break;
	}

	tmp = strdup(buf);
	if (!tmp) return NULL;
	return tmp;
}
#endif


static int register_bluetooth_module(void *data)
{
	int ret = -1;

	retvm_if(!data, FAIL, "Invalid parameter!");

	set_app_state(data);

	ret = vconf_notify_key_changed(VCONFKEY_BT_DEVICE, indicator_bluetooth_change_cb, data);
	if(ret != OK) {
		_E("vconf_notify_key_changed failed");
		return FAIL;
	}

	indicator_bluetooth_change_cb(NULL, data);

	return OK;
}

static int unregister_bluetooth_module(void)
{
	_E("unregister_bluetooth_module");
	int ret = 0;

	ret = vconf_ignore_key_changed(VCONFKEY_BT_DEVICE, indicator_bluetooth_change_cb);
	if(ret != OK) _E("vconf_ignore_key_changed failed");

	return ret;
}
