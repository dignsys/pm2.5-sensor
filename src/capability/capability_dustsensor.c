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

static const char *PROP_DUSTLEVEL = "dustLevel";
static const char *PROP_FINEDUSTLEVEL = "fineDustLevel";

extern void get_dust_level(uint32_t *dust_level);
extern void get_fine_dust_level(uint32_t *fine_dust_level);

/*
 * Dust Sensor capability attributes:
 *   fineDustLevel: PM 2.5
 *   dustLevel: PM 10
 */

bool handle_get_request_on_resource_capability_dustsensor(st_things_get_request_message_s* req_msg, st_things_representation_s* resp_rep)
{
	// A value representation of PM 10, micrograms per cubic meter
	uint32_t dust_level;

	// A value representation of PM 2.5, micrograms per cubic meter
	uint32_t fine_dust_level;

	if (req_msg->has_property_key(req_msg, PROP_DUSTLEVEL)) {
		get_dust_level(&dust_level);
		resp_rep->set_int_value(resp_rep, PROP_DUSTLEVEL, dust_level);
	}
	if (req_msg->has_property_key(req_msg, PROP_FINEDUSTLEVEL)) {
		get_fine_dust_level(&fine_dust_level);
		resp_rep->set_int_value(resp_rep, PROP_FINEDUSTLEVEL, fine_dust_level);
	}
    return true;
}
