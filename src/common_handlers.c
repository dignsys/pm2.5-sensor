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

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "st_things.h"
#include "log.h"

/* callback functions */
bool handle_reset_request(void)
{
    DBG("Received a request for RESET.");

    // TODO: Write your implementation in this section.
    return false;  // FIXME: Modify this line with the appropriate return value.
}

void handle_reset_result(bool result)
{
    DBG("Reset %s.\n", result ? "succeeded" : "failed");

    // TODO: Write your implementation in this section.
}

bool handle_ownership_transfer_request(void)
{
    DBG("Received a request for Ownership-transfer.");

    // TODO: Write your implementation in this section.
    return true;  // FIXME: Modify this line with the appropriate return value.
}

void handle_things_status_change(st_things_status_e things_status)
{
    DBG("Things status is changed: %d\n", things_status);

    // TODO: Write your implementation in this section.
}

bool init_user() {
	FN_CALL;

	bool ret = false;

	// TODO: Write your implementation in this section.
	return ret;  // FIXME: Modify this line with the appropriate return value.
}
