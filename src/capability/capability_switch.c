/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "st_things.h"
#include "log.h"

static const char *PROP_POWER = "power";

#define VALUE_STR_LEN_MAX 32
static const char *VALUE_SWITCH_ON = "on";
static const char *VALUE_SWITCH_OFF = "off";
static char g_switch[VALUE_STR_LEN_MAX] = "off";

extern void set_switch_status(bool status);

bool handle_get_request_on_resource_capability_switch(st_things_get_request_message_s* req_msg, st_things_representation_s* resp_rep)
{
	DBG("Received a GET request on %s\n", req_msg->resource_uri);

	if (req_msg->has_property_key(req_msg, PROP_POWER)) {
		resp_rep->set_str_value(resp_rep, PROP_POWER, g_switch);
	}

	return true;
}

bool handle_set_request_on_resource_capability_switch(st_things_set_request_message_s* req_msg, st_things_representation_s* resp_rep)
{
	DBG("Received a SET request on %s\n", req_msg->resource_uri);

	char *str_value = NULL;
	req_msg->rep->get_str_value(req_msg->rep, PROP_POWER, &str_value);

	/* check validation */
	if ((0 != strncmp(str_value, VALUE_SWITCH_ON, strlen(VALUE_SWITCH_ON)))
		&& (0 != strncmp(str_value, VALUE_SWITCH_OFF, strlen(VALUE_SWITCH_OFF)))) {
		ERR("Not supported value!!");
		free(str_value);
		return false;
	}

	if (0 != strncmp(str_value, g_switch, strlen(g_switch))) {
		strncpy(g_switch, str_value, VALUE_STR_LEN_MAX);
		if (0 == strncmp(g_switch, VALUE_SWITCH_ON, strlen(VALUE_SWITCH_ON))) {
			set_switch_status(true);
		} else {
			set_switch_status(false);
		}
	}
	resp_rep->set_str_value(resp_rep, PROP_POWER, g_switch);

	st_things_notify_observers(req_msg->resource_uri);

	free(str_value);

	return true;
}
