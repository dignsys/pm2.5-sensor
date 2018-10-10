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
#include <peripheral_io.h>
#include "resource/resource_pms7003_sensor.h"
#include "log.h"

/*
 * PMS7003 Frame Packet Information
 * UART setting : 9600bps, None parity, 1 stop bit
 * frame length is 32 Bytes, 1 second period data transmition
 *
 * Frame Format : 32 Bytes = START_CHAR1[1] + START_CHAR2[1] + FRAME_LENGTH[2] + DATA[2 x 13] + CHECKSUM[2]
 * Start Byte : start of frame [ 0x42, 0x4D ]
 * Length : number of bytes except Start Byte [Frame length = 2 x 13 + 2(data + check bytes)]
 * Data : 2 x 13 data : standard particle [data1, data2, data3], under atmospheric environment [data4, data5, data6], other [data7 ~ 13]
 * Checksum : Check code = START_CHAR1 + START_CHAR2 + data1 + …….. + data13
 */

/*
 * There are two options for digital output: passive and active.
 * Default mode is active after power up.
 * In this mode sensor would send serial data to the host automatically
 * The active mode is divided into two sub-modes: stable mode and fast mode.
 * If the concentration change is small the sensor would run at stable mode with the real interval of 2.3s.
 * And if the change is big the sensor would be changed to fast mode automatically with the interval of 200~800ms,
 * the higher of the concentration, the shorter of the interval.
 */

//#define DEBUG

#define MAX_TRY_COUNT	10
#define UART_PORT		4	// ARTIK 530 : UART0
#define MAX_FRAME_LEN	32

int incoming_byte = 0;          // for incoming serial data
char frame_buf[MAX_FRAME_LEN];  // for save pms7003 protocol data
int byte_position = 0;          // next byte position in frame_buf
int frame_len = MAX_FRAME_LEN;  // length of frame
bool in_frame = false;          // to check start character
unsigned int calc_checksum = 0; // to save calculated checksum value

// DATA STRUCTURE FOR PMS7003 PROTOCOL
static _pms7003_protocol_t pms7003_protocol;

static bool initialized = false;
static peripheral_uart_h g_uart_h;

//extern void set_sensor_value(_pms7003_protocol_t pms7003_protocol);
void set_sensor_value(_pms7003_protocol_t pms7003_protocol)
{

}

/*
 * open UART port and set UART handle resource
 * set BAUD rate, byte size, parity bit, stop bit, flow control
 * Appendix I：PMS7003 transport protocol-Active Mode
 * Default baud rate：9600bps Check bit：None Stop bit：1 bit
 */
bool resource_pms7003_init(void)
{
	if (initialized) return true;

	INFO("----- resource_pms7003_init -----");
	peripheral_error_e ret = PERIPHERAL_ERROR_NONE;

	// Opens the UART slave device
	ret = peripheral_uart_open(UART_PORT, &g_uart_h);
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("UART port [%d] open Failed, ret [%d]", UART_PORT, ret);
		return false;
	}
	// Sets baud rate of the UART slave device.
	ret = peripheral_uart_set_baud_rate(g_uart_h, PERIPHERAL_UART_BAUD_RATE_9600);	// The number of signal in one second is 9600
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("uart_set_baud_rate set Failed, ret [%d]", ret);
		return false;
	}
	// Sets byte size of the UART slave device.
	ret = peripheral_uart_set_byte_size(g_uart_h, PERIPHERAL_UART_BYTE_SIZE_8BIT);	// 8 data bits
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("byte_size set Failed, ret [%d]", ret);
		return false;
	}
	// Sets parity bit of the UART slave device.
	ret = peripheral_uart_set_parity(g_uart_h, PERIPHERAL_UART_PARITY_NONE);	// No parity is used
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("parity set Failed, ret [%d]", ret);
		return false;
	}
	// Sets stop bits of the UART slave device
	ret = peripheral_uart_set_stop_bits (g_uart_h, PERIPHERAL_UART_STOP_BITS_1BIT);	// One stop bit
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("stop_bits set Failed, ret [%d]", ret);
		return false;
	}
	// Sets flow control of the UART slave device.
	// No software flow control & No hardware flow control
	ret = peripheral_uart_set_flow_control (g_uart_h, PERIPHERAL_UART_SOFTWARE_FLOW_CONTROL_NONE, PERIPHERAL_UART_HARDWARE_FLOW_CONTROL_NONE);
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("flow control set Failed, ret [%d]", ret);
		return false;
	}

	initialized = true;
	return true;
}

/*
 * To write data to a slave device
 */
bool resource_write_data(uint8_t *data, uint32_t length)
{
	peripheral_error_e ret = PERIPHERAL_ERROR_NONE;

	// write length byte data to UART
	ret = peripheral_uart_write(g_uart_h, data, length);
	if (ret != PERIPHERAL_ERROR_NONE) {
		ERR("UART write failed, ret [%d]", ret);
		return false;
	}

	return true;
}

/*
 * To read data from a slave device
 */
bool resource_read_data(uint8_t *data, uint32_t length, bool blocking_mode)
{
	int try_again = 0;
	peripheral_error_e ret = PERIPHERAL_ERROR_NONE;

	if (g_uart_h == NULL)
		return false;

	while (1) {
		// read length byte from UART
		ret = peripheral_uart_read(g_uart_h, data, length);
		if (ret == PERIPHERAL_ERROR_NONE)
			return true;

		// if data is not ready, try again
		if (ret == PERIPHERAL_ERROR_TRY_AGAIN) {
			// if blocking mode, read again
			if (blocking_mode == true) {
				usleep(100 * 1000);
				INFO(".");
				continue;
			} else {
				// if non-blocking mode, retry MAX_TRY_COUNT
				if (try_again >= MAX_TRY_COUNT) {
					ERR("No data to receive");
					return false;
				} else {
					try_again++;
				}
			}
		} else {
			// if return value is not (PERIPHERAL_ERROR_NONE or PERIPHERAL_ERROR_TRY_AGAIN)
			// return with false
			ERR("UART read failed, , ret [%d]", ret);
			return false;
		}
	}

	return true;
}

/*
 * close UART handle and clear UART resources
 */
void resource_pms7003_fini(void)
{
	INFO("----- resource_pms7003_fini -----");
	if(initialized) {
		// Closes the UART slave device
		peripheral_uart_close(g_uart_h);
		initialized = false;
		g_uart_h = NULL;
	}
}

/*
 * read sensor data from PMS7003 and format
 */
bool resource_pms7003_read(void)
{
	uint8_t data;
	bool packet_received = false;
	calc_checksum = 0;

	// clear frame buffer
	memset(frame_buf, 0, MAX_FRAME_LEN);

	// clear pms7003_protocol structure
	memset(&pms7003_protocol, 0, sizeof(_pms7003_protocol_t));

	// set Frame length = 2x13 + 2 (data+check bytes)
	pms7003_protocol.frame_len = MAX_FRAME_LEN - sizeof(pms7003_protocol.checksum);

	if (!initialized) {
		// open UART port and set UART handle resource
		// set BAUD rate, byte size, parity bit, stop bit, flow control
		if (!resource_pms7003_init()) {
			ERR("resource initialization failed");
			return false;
		}
	}

	while (!packet_received) {
		 // read data from a slave device, block until data is received
		if (resource_read_data(&data, 1, true) == false) {
			ERR("resource_read_data failed");
			return false;
		}

		#ifdef DEBUG
		//INFO("READ: [0x%02X]", incoming_byte);
		#endif

		if (!in_frame) {
			if (data == 0x42 && byte_position == 0) {
				#ifdef DEBUG
				INFO("READ: [0x%02X] ST1", data);
				#endif
				frame_buf[byte_position] = data;            // add start character 1 into buffer
				pms7003_protocol.frame_header[0] = data;
				calc_checksum = data;                       // add data to Checksum
				byte_position++;                            // set to next position
			}
			else if (data == 0x4D && byte_position == 1) {
				#ifdef DEBUG
				INFO("READ: [0x%02X] ST2", data);
				#endif
				frame_buf[byte_position] = data;            // add start character 2 into buffer
				pms7003_protocol.frame_header[1] = data;
				calc_checksum += data;                      // add data to Checksum

				// we have valid frame header
				in_frame = true;                                     // received start char 1[0x42] and char 2[0x4d]
				byte_position++;                                     // set to next position
			}
			else {
				// data is not in synced, ignore data
				#ifdef DEBUG
				INFO("Frame syncing... [0x%02X]", data);
				#endif
			}
		}
		else {
			#ifdef DEBUG
			INFO("READ: [0x%02X]", data);
			#endif
			// save data into frame buffer
			frame_buf[byte_position] = data;
			calc_checksum += data;
			byte_position++;

			unsigned int val = (frame_buf[byte_position - 1] & 0xff) + (frame_buf[byte_position - 2] << 8);
			switch (byte_position) {
			case 4:
				pms7003_protocol.frame_len = val;
				frame_len = val + byte_position;
				INFO("frame_len: [%d]", frame_len);
				break;
			case 6:
				pms7003_protocol.standard_particle.PM1_0 = val;
				break;
			case 8:
				pms7003_protocol.standard_particle.PM2_5 = val;
				break;
			case 10:
				pms7003_protocol.standard_particle.PM10 = val;
				break;
			case 12:
				pms7003_protocol.atmospheric_env.PM1_0 = val;
				break;
			case 14:
				pms7003_protocol.atmospheric_env.PM2_5 = val;
				break;
			case 16:
				pms7003_protocol.atmospheric_env.PM10 = val;
				break;
			case 32:
				pms7003_protocol.checksum = val;
				calc_checksum -= ((val>>8) + (val&0xFF));
				break;
			default:
				break;
			}

			// check if all data is received
			if (byte_position >= frame_len) {
				#ifdef DEBUG
				INFO("READ: byte_position[%d] : frame_len[%d]", byte_position, frame_len);
				#endif
				packet_received = true;
				byte_position = 0;
				in_frame = false;
			}
		}
	}

	// check received checksum and calculated checksum is same or not
	// Checksum : Check code = START_CHAR1 + START_CHAR2 + data1 + …….. + data13
	if (calc_checksum == pms7003_protocol.checksum) {
		// save sensor data and return true
		set_sensor_value(pms7003_protocol);
		return true;
	} else {
		// return false
		ERR("Checksum error, calc_checksum[0x%X] != [0x%X] pms7003_protocol.checksum", calc_checksum, pms7003_protocol.checksum);
		return false;
	}
}

