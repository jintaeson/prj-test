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
#include <app_preference.h>

#include "common.h"
#include "indicator.h"
#include "icon.h"
#include "modules.h"
#include "main.h"
#include "log.h"
#include "util.h"
#include "box.h"

#define ICON_PRIORITY	INDICATOR_PRIORITY_NOTI_MIN
#define MODULE_NAME		"more_notify"

#define PART_NAME_MORE_NOTI "elm.swallow.more_noti"

static int register_more_notify_module(void *data);
static int unregister_more_notify_module(void);

icon_s more_notify = {
	.name = MODULE_NAME,
	.priority = ICON_PRIORITY,
	.always_top = EINA_TRUE,
	.exist_in_view = EINA_FALSE,
	.img_obj.data = "Notify/b03_notify_more.png",
	.obj_exist = EINA_FALSE,
	.area = INDICATOR_ICON_AREA_MORE_NOTI,
	.init = register_more_notify_module,
	.fini = unregister_more_notify_module
};

enum {
	MUSIC_PLAY,
	MUSIC_PAUSED,
};

static void set_app_state(void *data)
{
	more_notify.ad = data;
}

void indicator_more_notify_icon_change(Eina_Bool show)
{
	_D("Show 'more notify' icon:%s", (show) ? "true" : "false");

	struct appdata *ad = more_notify.ad;
	retm_if(!ad, "Invalid parameter!");

	if (show) {
		util_signal_emit(ad, "indicator.more_noti.show", "indicator.prog");
		_D();
	}
	else {
		util_signal_emit(ad, "indicator.more_noti.hide", "indicator.prog");
		_D();
	}

	return;
}

static void icon_create_and_swallow(icon_s *icon, const char *part_name)
{
	struct appdata *ad = (struct appdata *)icon->ad;
	ret_if(!ad);

	icon_add(&ad->win, icon);
	ret_if(!icon->img_obj.obj);

	edje_object_part_swallow(elm_layout_edje_get(ad->win.layout), part_name, icon->img_obj.obj);
}

static int register_more_notify_module(void *data)
{
	_D("register_more_notify_module");

	retvm_if(!data, FAIL, "Invalid parameter!");

	set_app_state(data);

	icon_create_and_swallow(&more_notify, PART_NAME_MORE_NOTI);

	return OK;
}


static int unregister_more_notify_module(void)
{
	struct appdata *ad = (struct appdata *)more_notify.ad;

	icon_del(&more_notify);
	edje_object_part_unswallow(ad->win.layout, more_notify.img_obj.obj);

	return OK;
}
