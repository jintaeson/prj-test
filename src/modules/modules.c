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
#include "common.h"
#include "modules.h"

/* Network */
extern icon_s rssi[];
/* Connection */
extern icon_s usb;
extern icon_s wifi;
extern icon_s mobile_hotspot;
extern icon_s conn;
extern icon_s wifi_direct;

/* Processing */
extern icon_s call;
extern icon_s call_divert;
extern icon_s call_options_mute;
extern icon_s call_options_speaker;

/* Information */
extern icon_s ext_storage;
extern icon_s noti;

/* alarm */
extern icon_s useralarm;

/* playing */
extern icon_s fm_radio;
extern icon_s mp3_play;
extern icon_s voice_recorder;

/* Setting */
extern icon_s silent;
extern icon_s bluetooth;
extern icon_s gps;
extern icon_s nfc;
extern icon_s dnd;

/* Clock */
extern icon_s sysclock;

/* Power */
extern icon_s battery;

/* Smart stay */
/* earphone */
extern icon_s earphone;

/* always */
extern icon_s lowmem;
extern icon_s dock;

extern icon_s video_play;
extern icon_s more_notify;



static icon_s *modules[] = {

	/* Clock */
	&sysclock,
	/* Power */
	&battery,
	/* Network */
	&wifi,
	&rssi[0],
	&rssi[1],
	/* Connection */
	&usb,
	&mobile_hotspot,
	&conn,
	&wifi_direct,
	/* Processing */
	&call,
	&call_divert,
	&call_options_mute,
	&call_options_speaker,

	/* Information */
	&ext_storage,
	&noti,
	&useralarm,
	&fm_radio,
	&mp3_play,
	&voice_recorder,

	/* Setting */
	&silent,
	&bluetooth,
	&gps,
	&nfc,
	&dnd,

	/*Smart Stay*/
	&earphone,
	&lowmem,
	&dock,
	&video_play,
	&more_notify,
/* Add your module object here */
	NULL
};

void modules_init(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		list_insert_icon(modules[i]);
		modules[i]->ad = data;
		if (modules[i]->init && !modules[i]->initialized) {
			if (modules[i]->init(data) == OK)
				modules[i]->initialized = EINA_TRUE;
		}
	}
}


void modules_init_first(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; i < 6; i++) {
		list_insert_icon(modules[i]);
		modules[i]->ad = data;
		if (modules[i]->init && !modules[i]->initialized) {
			if (modules[i]->init(data) == OK)
				modules[i]->initialized = EINA_TRUE;
		}
	}
}


void modules_fini(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		if (modules[i]->fini && modules[i]->initialized) {
			if (modules[i]->fini() == OK)
				modules[i]->initialized = EINA_FALSE;
		}
	}
	/* delete modules */
	list_free_all();
}


void modules_lang_changed(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		if (modules[i]->lang_changed && modules[i]->initialized)
			modules[i]->lang_changed(data);
	}
}


void modules_region_changed(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		if (modules[i]->region_changed && modules[i]->initialized)
			modules[i]->region_changed(data);
	}
}


void modules_minictrl_control(int action, const char* name, void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		if (modules[i]->minictrl_control && modules[i]->initialized)
			modules[i]->minictrl_control(action, name, data);
	}
}


void modules_wake_up(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		if (modules[i]->wake_up && modules[i]->initialized)
			modules[i]->wake_up(data);
	}
}


#ifdef _SUPPORT_SCREEN_READER
void modules_register_tts(void *data)
{
	int i;
	/* add items to list */
	for (i = 0; modules[i]; i++) {
		modules[i]->ad = data;
		if (modules[i]->register_tts && modules[i]->initialized)
		{
			modules[i]->register_tts(data);
		}
	}
}
#endif

