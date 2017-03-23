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

#include "indicator.h"
#include "log.h"

Evas_Object *indicator_box_object_get(win_info *win, indicator_icon_area_type type)
{
	switch(type) {
	case INDICATOR_ICON_AREA_SYSTEM:
		return win->_system_box;
	case INDICATOR_ICON_AREA_MINICTRL:
		return win->_minictrl_box;
	case INDICATOR_ICON_AREA_NOTI:
		return win->_noti_box;
	case INDICATOR_ICON_AREA_ALARM:
		return win->_alarm_box;
	case INDICATOR_ICON_AREA_CONNECTION_SYSTEM:
		return win->_connection_system_box;
	default:
		_D("Invalid area type");
		return NULL;
	}
}

Evas_Object **indicator_box_object_ptr_get(win_info *win, indicator_icon_area_type type)
{
	switch(type) {
	case INDICATOR_ICON_AREA_SYSTEM:
		return &win->_system_box;
	case INDICATOR_ICON_AREA_MINICTRL:
		return &win->_minictrl_box;
	case INDICATOR_ICON_AREA_NOTI:
		return &win->_noti_box;
	case INDICATOR_ICON_AREA_ALARM:
		return &win->_alarm_box;
	case INDICATOR_ICON_AREA_CONNECTION_SYSTEM:
		return &win->_connection_system_box;
	default:
		_D("Invalid area type");
		return NULL;
	}
}
