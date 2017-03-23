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

#include <notification.h>

#ifndef __INDICATOR_TICKER_H__
#define __INDICATOR_TICKER_H__

/**
 * @file ticker.h
 */

/**
 * @defgroup ticker Ticker
 */

/**
 * @addtogroup ticker
 * @{
 */

/**
 * @brief Enumeration for animated icon type
 */
typedef enum _indicator_animated_icon_type {
	INDICATOR_ANIMATED_ICON_NONE = -1,
	INDICATOR_ANIMATED_ICON_DOWNLOAD = 1,
	INDICATOR_ANIMATED_ICON_UPLOAD,
	INDICATOR_ANIMATED_ICON_INSTALL,
} indicator_animated_icon_type;

/**
 * @brief Definition for ticker data structure
 */
typedef struct ticker {
	Evas_Object *textblock; /**< text block for notification content*/
	Evas_Object *icon;
	int current_page;	/**< Current page*/

	notification_h noti;

	char *pkgname;
	int current_line;
} ticker_info_s;

/**
 * @brief Initializes ticker module.
 *
 * @remarks Ticker module is registered for receiving notifications
 *
 * @param[in] data the app data
 *
 * @return 0 on success, negative value on failure
 */
extern int ticker_init(void *data);

/**
 * @brief Deinitializes ticker module.
 *
 * @param[in] data the app data
 *
 * @return 0 on success, negative value on failure
 */
extern int ticker_fini(void *data);


/**
 * @}
 */

#endif /* __INDICATOR_TICKER_H__ */
