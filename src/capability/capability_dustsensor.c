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

#include <stdio.h>
#include <stdbool.h>
#include "st_things.h"
#include "log.h"


static const char* PROP_DUSTLEVEL = "dustLevel";
static const char* PROP_FINEDUSTLEVEL = "fineDustLevel";

bool handle_get_request_on_resource_capability_dustsensor_main_0(st_things_get_request_message_s* req_msg, st_things_representation_s* resp_rep)
{
    DBG("Received a GET request on %s\n", req_msg->resource_uri);

    if (req_msg->has_property_key(req_msg, PROP_DUSTLEVEL)) {
        // TODO: Write your implementation in this section.
    }
    if (req_msg->has_property_key(req_msg, PROP_FINEDUSTLEVEL)) {
        // TODO: Write your implementation in this section.
    }
    return false;  // FIXME: Modify this line with the appropriate return value.
}
