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

#define ICON_PRIORITY	INDICATOR_PRIORITY_FIXED8
#define MODULE_NAME		"wifi-direct"

#include <vconf.h>

#include "common.h"
#include "icon.h"
#include "log.h"
#include "util.h"

static int register_wifi_direct_module(void *data);
static int unregister_wifi_direct_module(void);

icon_s wifi_direct = {
	.type = INDICATOR_IMG_ICON,
	.name = MODULE_NAME,
	.priority = ICON_PRIORITY,
	.always_top = EINA_FALSE,
	.exist_in_view = EINA_FALSE,
	.img_obj = {0,},
	.obj_exist = EINA_FALSE,
	.area = INDICATOR_ICON_AREA_FIXED,
	.init = register_wifi_direct_module,
	.fini = unregister_wifi_direct_module,
};

static const char *_wifi_direct_get_icon_for_state(int vconf_wifi_key_value)
{
	switch (vconf_wifi_key_value)
	{
		case VCONFKEY_WIFI_DIRECT_CONNECTED:
		case VCONFKEY_WIFI_DIRECT_GROUP_OWNER:
			return "Bluetooth, NFC, GPS/b03_wi_fi_direct_on_connected.png";
		case VCONFKEY_WIFI_DIRECT_ACTIVATED:
		case VCONFKEY_WIFI_DIRECT_DISCOVERING:
		case VCONFKEY_WIFI_DIRECT_DEACTIVATING:
		case VCONFKEY_WIFI_DIRECT_ACTIVATING:
		case VCONFKEY_WIFI_DIRECT_CONNECTING:
		case VCONFKEY_WIFI_DIRECT_DISCONNECTING:
			return "Bluetooth, NFC, GPS/b03_wi-fi_direct_on_not_connected.png";
		case VCONFKEY_WIFI_DIRECT_DEACTIVATED:
			return NULL;
	}
	return NULL;
}

static void _wifi_direct_update_icon(int state)
{
	switch (state) {
		case VCONFKEY_WIFI_DIRECT_DEACTIVATED:
			_D("Wifi-Direct off");
			icon_hide(&wifi_direct);
			util_signal_emit(wifi_direct.ad, "indicator.wifidirect.hide", "indicator.prog");
			break;
		default:
			wifi_direct.img_obj.data = _wifi_direct_get_icon_for_state(state);
			icon_show(&wifi_direct);
			util_signal_emit(wifi_direct.ad, "indicator.wifidirect.show", "indicator.prog");
			_D("Wifi-Direct on: %s", wifi_direct.img_obj.data);
	}
}

static void _wifi_direct_state_changed_cb(keynode_t *node, void *user_data)
{
	_wifi_direct_update_icon(node->value.i);
}

static int register_wifi_direct_module(void *data)
{
	int state;

	wifi_direct.ad = data;

	int err = vconf_notify_key_changed(VCONFKEY_WIFI_DIRECT_STATE,
			_wifi_direct_state_changed_cb, data);
	if (err != 0) {
		_E("vconf_notify_key_changed failed");
		return FAIL;
	}

	if (vconf_get_int(VCONFKEY_WIFI_DIRECT_STATE, &state) != 0) {
		_E("vconf_get_int failed");
		unregister_wifi_direct_module();
		return FAIL;
	};

	_wifi_direct_update_icon(state);
	return OK;
}

static int unregister_wifi_direct_module(void)
{
	vconf_ignore_key_changed(VCONFKEY_WIFI_DIRECT_STATE, _wifi_direct_state_changed_cb);
	return OK;
}
