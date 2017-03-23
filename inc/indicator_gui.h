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

#ifndef __INDICATOR_GUI_H__
#define __INDICATOR_GUI_H__

/* DISPLAY */
#define INDICATOR_HEIGHT 40

#define ICON_PADDING		3

#define PADDING_WIDTH		10
#define LEFT_PADDING_WIDTH	4
#define RIGHT_PADDING_WIDTH	4
#define INDICATOR_PADDING_H 3

#define DEFAULT_ICON_PADDING 3

#define DEFAULT_ICON_HEIGHT	34
#define DEFAULT_ICON_WIDTH	34

#define BATTERY_DIGIT_WIDTH 12
#define BATTERY_TWO_DIGITS_WIDTH 25
#define BATTERY_FULL_WIDTH 30

#define BATTERY_DIGIT_HEIGHT 17

#define MESSAGE_PADDING 12
#define MESSAGE_ICON_WIDTH 34
#define MESSAGE_ICON_HEIGHT 34
#define MESSAGE_ICON_PADDING 12

#define INDI_DEFAULT_BG_COLOR 30 30 30 255
#define INDI_DEFAULT_BG_TRANSLUCENT 0 0 0 125
#define INDI_DEFAULT_BG_TRANSPARENT 0 0 0 0
#define INDI_DEFAULT_BG_TEST 255 0 0 150


/* FONT */
#define ICON_FONT_NAME		"Tizen"
#define ICON_FONT_STYLE		"Medium"
#define ICON_FONT_SIZE		20
#define FONT_COLOR		255, 255, 255, 255

#define INDICATOR_HIDE_TRIGER_H 30

#define DOWN_ANI_TIMER 0.3

#define INDICATOR_ICON_OPACITY 204	//80%

#define DEFAULT_LABEL_STRING	"<font_size=%d><color=#%02x%02x%02x%02x>%s</color></font_size>"
#define CUSTOM_LABEL_STRING		"<font=%s><style=%s><font_size=%d>%s</font_size></style></font>"

#ifndef _INDICATOR_REMOVE_SEARCH
#define _INDICATOR_REMOVE_SEARCH
#endif
#endif /*__INDICATOR_GUI_H__*/
