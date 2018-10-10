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

static const char *PROP_FANSPEED = "fanSpeed";

static uint32_t fan_speed;

extern void set_fan_speed(uint32_t fan_speed);
extern void get_fan_speed(uint32_t *fan_speed);

bool handle_get_request_on_resource_capability_fanspeed(st_things_get_request_message_s* req_msg, st_things_representation_s* resp_rep)
{
	//DBG("Received a GET request on %s\n", req_msg->resource_uri);

	if (req_msg->has_property_key(req_msg, PROP_FANSPEED)) {
		get_fan_speed(&fan_speed);
		INFO("Received a GET fan_speed [%d]", fan_speed);
		resp_rep->set_int_value(resp_rep, PROP_FANSPEED, fan_speed);
	}
	return true;
}

bool handle_set_request_on_resource_capability_fanspeed(st_things_set_request_message_s* req_msg, st_things_representation_s* resp_rep)
{
	//DBG("Received a SET request on %s\n", req_msg->resource_uri);
	int64_t speed;

	req_msg->rep->get_int_value(req_msg->rep, PROP_FANSPEED, &speed);

	INFO("Received a SET fan speed [%d]", speed);
	resp_rep->set_int_value(resp_rep, PROP_FANSPEED, speed);
	set_fan_speed(speed);

    return true;
}
