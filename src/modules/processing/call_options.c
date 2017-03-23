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

#include <call-manager/call-manager.h>
#include <call-manager/call-manager-extension.h>
#include <call-manager/call-manager-ext.h>
#include "common.h"
#include "indicator.h"
#include "main.h"
#include "modules.h"
#include "icon.h"
#include "util.h"
#include "log.h"

#define MODULE_NAME_MUTE "call_options_mute"
#define ICON_PRIORITY_MUTE	INDICATOR_PRIORITY_MINICTRL2

#define MODULE_NAME_SPEAKER "call_options_speaker"
#define ICON_PRIORITY_SPEAKER	INDICATOR_PRIORITY_MINICTRL3

static int register_call_options_mute_module(void *data);
static int register_call_options_speaker_module(void *data);

static int unregister_call_options_mute_module(void);
static int unregister_call_options_speaker_module(void);

icon_s call_options_mute = {
	.type = INDICATOR_IMG_ICON,
	.name = MODULE_NAME_MUTE,
	.priority = ICON_PRIORITY_MUTE,
	.always_top = EINA_FALSE,
	.exist_in_view = EINA_FALSE,
	.img_obj = {"Call/b03_call_mute.png",},
	.obj_exist = EINA_FALSE,
	.area = INDICATOR_ICON_AREA_MINICTRL,
	.init = register_call_options_mute_module,
	.fini = unregister_call_options_mute_module,
	.minictrl_control = NULL, /* mctrl_monitor_cb */
};

icon_s call_options_speaker = {
	.type = INDICATOR_IMG_ICON,
	.name = MODULE_NAME_SPEAKER,
	.priority = ICON_PRIORITY_SPEAKER,
	.always_top = EINA_FALSE,
	.exist_in_view = EINA_FALSE,
	.img_obj = {"Call/b03_call_speaker_on.png",},
	.obj_exist = EINA_FALSE,
	.area = INDICATOR_ICON_AREA_MINICTRL,
	.init = register_call_options_speaker_module,
	.fini = unregister_call_options_speaker_module,
	.minictrl_control = NULL, /* mctrl_monitor_cb */
};

static cm_client_h cm_handle;
static int init_cnt = 0;

static void mute_status_cb(cm_mute_status_e mute_status, void *user_data)
{
	if (mute_status == CM_MUTE_STATUS_ON)
		icon_show(&call_options_mute);

	else if (mute_status == CM_MUTE_STATUS_OFF)
		icon_hide(&call_options_mute);
}

static void audio_status_cb(cm_audio_state_type_e audio_state, void *user_data)
{
	if (audio_state == CM_AUDIO_STATE_SPEAKER_E)
		icon_show(&call_options_speaker);

	else
		icon_hide(&call_options_speaker);
}

static int call_manager_init(void)
{
	if (init_cnt == 0) {
		int ret = cm_init(&cm_handle);
		retvm_if(ret != CM_ERROR_NONE, FAIL, "cm_init failed[%d]: %s", ret, get_error_message(ret));
	}

	init_cnt++;

	return OK;
}

static int call_manager_deinit(void)
{
	if (init_cnt == 1) {
		int ret = cm_deinit(cm_handle);
		retvm_if(ret != CM_ERROR_NONE, FAIL, "cm_deinit failed[%d]: %s", ret, get_error_message(ret));
	}

	init_cnt--;

	return OK;
}

static int register_call_options_mute_module(void *data)
{
	int ret;

	ret = call_manager_init();
	retvm_if(ret == FAIL, FAIL, "call_manager_init[%d]: %s", ret, get_error_message(ret));

	ret = cm_set_mute_status_cb(cm_handle, mute_status_cb, data);
	retvm_if(ret != CM_ERROR_NONE, FAIL, "cm_mute_status_cb failed[%d]: %s", ret, get_error_message(ret));

	return OK;
}

static int register_call_options_speaker_module(void *data)
{
	int ret;

	ret = call_manager_init();
	retvm_if(ret == FAIL, FAIL, "call_manager_init[%d]: %s", ret, get_error_message(ret));

	ret = cm_set_audio_state_changed_cb(cm_handle, audio_status_cb, data);
	retvm_if(ret != CM_ERROR_NONE, FAIL, "cm_set_audio_state_changed_cb[%d]: %s", ret, get_error_message(ret));

	return OK;
}

static int unregister_call_options_mute_module(void)
{
	int ret;

	ret = cm_unset_mute_status_cb(cm_handle);
	if (ret != CM_ERROR_NONE)
		_E("cm_unset_mute_status_cb[%d]: %s", ret, get_error_message(ret));

	ret = call_manager_deinit();
	retvm_if(ret == FAIL, FAIL, "call_manager_deinit[%d]: %s", ret, get_error_message(ret));

	return OK;
}

static int unregister_call_options_speaker_module(void)
{
	int ret;

	ret = cm_unset_audio_state_changed_cb(cm_handle);
	if (ret != CM_ERROR_NONE)
		_E("cm_unset_audio_state_changed_cb[%d]: %s", ret, get_error_message(ret));

	ret = call_manager_deinit();
	retvm_if(ret == FAIL, FAIL, "call_manager_deinit[%d]: %s", ret, get_error_message(ret));

	return OK;
}
