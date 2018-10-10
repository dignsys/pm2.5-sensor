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

#include <service_app.h>
#include <app_common.h>
#include <stdio.h>
#include <Ecore.h>
#include "st_things.h"
#include "log.h"
#include "resource/resource_pms7003_sensor.h"

#define EVENT_INTERVAL_SECOND	(1.0f)	// sensor event timer : 1 second interval
#define JSON_PATH "device_def.json"

static const char *RES_CAPABILITY_SWITCH_MAIN_0 = "/capability/switch/main/0";
static const char *RES_CAPABILITY_FANSPEED_MAIN_0 = "/capability/fanSpeed/main/0";
static const char *RES_CAPABILITY_DUSTSENSOR_MAIN_0 = "/capability/dustSensor/main/0";

Ecore_Timer *sensor_event_timer = NULL;
pthread_mutex_t  mutex_lock = PTHREAD_MUTEX_INITIALIZER;

#define MUTEX_LOCK		pthread_mutex_lock(&mutex_lock)
#define MUTEX_UNLOCK	pthread_mutex_unlock(&mutex_lock)
#define UNUSED(x)		(void)(x)

_concentration_unit_t	standard_particle;	// CF=1，standard particle
_concentration_unit_t	atmospheric_env;	// under atmospheric environment

/* get and set request handlers */
extern bool handle_get_request_on_resource_capability_switch_main_0(st_things_get_request_message_s *req_msg, st_things_representation_s *resp_rep);
extern bool handle_set_request_on_resource_capability_switch_main_0(st_things_set_request_message_s *req_msg, st_things_representation_s *resp_rep);
extern bool handle_get_request_on_resource_capability_fanspeed_main_0(st_things_get_request_message_s *req_msg, st_things_representation_s *resp_rep);
extern bool handle_set_request_on_resource_capability_fanspeed_main_0(st_things_set_request_message_s *req_msg, st_things_representation_s *resp_rep);
extern bool handle_get_request_on_resource_capability_dustsensor_main_0(st_things_get_request_message_s *req_msg, st_things_representation_s *resp_rep);

/* resource pms7003 functions */
extern bool resource_pms7003_init(void);
extern void resource_pms7003_fini(void);
extern bool resource_pms7003_read(void);

static void _init_mutex(void)
{
	pthread_mutex_init(&mutex_lock, NULL);
}

static void _deinit_mutex(void)
{
	INFO("deinit_mutex...");
	pthread_mutex_destroy(&mutex_lock);
}

// get PM10 level
void get_dust_level(uint32_t *dust_level)
{
	MUTEX_LOCK;
	*dust_level = standard_particle.PM10;
	MUTEX_UNLOCK;
}

// get PM2.5 level
void get_fine_dust_level(uint32_t *fine_dust_level)
{
	MUTEX_LOCK;
	*fine_dust_level = standard_particle.PM2_5;
	MUTEX_UNLOCK;
}

static Eina_Bool _sensor_interval_event_cb(void *data)
{
	bool switch_status = false;

	// read sensor data from PMS7003 module
	if (!resource_pms7003_read()) {
		ERR("resource_pms7003_read Failed");

		// cancel periodic event timer operation
		return ECORE_CALLBACK_CANCEL;
	} else {
		#ifndef _DEBUG_PRINT_
		struct timeval __tv;
		gettimeofday(&__tv, NULL);

		// get sensor value from PMS7003 module
		uint32_t dust = 0;
		uint32_t fine = 0;
		get_dust_level(&dust);			// PM10 level
		get_fine_dust_level(&fine);		// PM2.5 level

		INFO("[%d.%06d] dustLevel : %d ug/m3, fineDustLevel : %d ug/m3", __tv.tv_sec, __tv.tv_usec, dust, fine);
		#endif
	}

	// reset next event timer
	return ECORE_CALLBACK_RENEW;
}

static void _clear_timer_resource(void)
{
	INFO("clear_timer_resource...");

	if (sensor_event_timer) {
		ecore_timer_del(sensor_event_timer);
		sensor_event_timer = NULL;
	}
}

/* handle : for getting request on resources */
bool handle_get_request(st_things_get_request_message_s *req_msg, st_things_representation_s *resp_rep)
{
	DBG("resource_uri [%s]", req_msg->resource_uri);

	if (0 == strcmp(req_msg->resource_uri, RES_CAPABILITY_SWITCH_MAIN_0)) {
		return handle_get_request_on_resource_capability_switch_main_0(req_msg, resp_rep);
	}
	if (0 == strcmp(req_msg->resource_uri, RES_CAPABILITY_FANSPEED_MAIN_0)) {
		return handle_get_request_on_resource_capability_fanspeed_main_0(req_msg, resp_rep);
	}
	if (0 == strcmp(req_msg->resource_uri, RES_CAPABILITY_DUSTSENSOR_MAIN_0)) {
		return handle_get_request_on_resource_capability_dustsensor_main_0(req_msg, resp_rep);
	}

	ERR("not supported uri");
	return false;
}

/* handle : for setting request on resources */
bool handle_set_request(st_things_set_request_message_s *req_msg, st_things_representation_s *resp_rep)
{
	DBG("resource_uri [%s]", req_msg->resource_uri);

	if (0 == strcmp(req_msg->resource_uri, RES_CAPABILITY_SWITCH_MAIN_0)) {
		return handle_set_request_on_resource_capability_switch_main_0(req_msg, resp_rep);
	}
	if (0 == strcmp(req_msg->resource_uri, RES_CAPABILITY_FANSPEED_MAIN_0)) {
		return handle_set_request_on_resource_capability_fanspeed_main_0(req_msg, resp_rep);
	}

	ERR("not supported uri");
	return false;
}

/* callback functions */
bool handle_reset_request(void)
{
	DBG("Received a request for RESET.");

	return false;
}

void handle_reset_result(bool result)
{
	DBG("Reset %s.\n", result ? "succeeded" : "failed");
}

bool handle_ownership_transfer_request(void)
{
	DBG("Received a request for Ownership-transfer.");

	return true;
}

void handle_things_status_change(st_things_status_e things_status)
{
	DBG("Things status is changed: %d\n", things_status);
}

bool init_user()
{
	FN_CALL;

	bool ret = true;
	_init_mutex();

	ret = resource_pms7003_init();
	if (ret == false) {
		ERR("Failed to resource_pms7003_init");
		ret = false;
	}

	sensor_event_timer = ecore_timer_add(EVENT_INTERVAL_SECOND, _sensor_interval_event_cb, NULL);
	if (!sensor_event_timer) {
		ERR("Failed to add sensor_event_timer");
		ret = false;
	}

	return ret;
}

/* initialize */
void init_thing()
{
	FN_CALL;
	static bool binitialized = false;
	if (binitialized) {
		DBG("Already initialized!!");
		return;
	}

	bool easysetup_complete = false;

	char app_json_path[128] = {0,};
	char *app_res_path = NULL;
	char *app_data_path = NULL;

	app_res_path = app_get_resource_path();
	if (!app_res_path) {
		ERR("app_res_path is NULL!!");
		return;
	}

	app_data_path = app_get_data_path();
	if (!app_data_path) {
		ERR("app_data_path is NULL!!");
		free(app_res_path);
		return;
	}

	snprintf(app_json_path, sizeof(app_json_path), "%s/%s", app_res_path, JSON_PATH);

	if (0 != st_things_set_configuration_prefix_path((const char *)app_res_path, (const char *)app_data_path)) {
		ERR("st_things_set_configuration_prefix_path() failed!!");
		free(app_res_path);
		free(app_data_path);
		return;
	}

	free(app_res_path);
	free(app_data_path);

	if (0 != st_things_initialize(app_json_path, &easysetup_complete)) {
		ERR("st_things_initialize() failed!!");
		return;
	}

	binitialized = true;
	init_user();

	DBG("easysetup_complete:[%d] ", easysetup_complete);

	st_things_register_request_cb(handle_get_request, handle_set_request);
	st_things_register_reset_cb(handle_reset_request, handle_reset_result);
	st_things_register_user_confirm_cb(handle_ownership_transfer_request);
	st_things_register_things_status_change_cb(handle_things_status_change);

	st_things_start();

	FN_END;
}

static bool service_app_create(void *user_data)
{
	UNUSED(user_data);

	return true;
}

static void service_app_terminate(void *user_data)
{
	UNUSED(user_data);

	_clear_timer_resource();

	resource_pms7003_fini();

	_deinit_mutex();
}

static void service_app_control(app_control_h app_control, void *user_data)
{
	UNUSED(user_data);

	if (app_control == NULL) {
		ERR("app_control is NULL");
		return;
	}

	init_thing();
}

int main(int argc, char *argv[])
{
	FN_CALL;

	service_app_lifecycle_callback_s event_callback;

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	return service_app_main(argc, argv, &event_callback, NULL);
}
