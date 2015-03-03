/* gas-detector-bricklet
 * Copyright (C) 2015 Olaf Lüke <olaf@tinkerforge.com>
 *
 * gas_detector.h: Implementation of Gas Detector Bricklet messages
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GAS_DETECTOR_H
#define GAS_DETECTOR_H

#include <stdint.h>
#include "bricklib/com/com_common.h"

#define DETECTOR_TYPE_EEPROM_POSITION (BRICKLET_PLUGIN_MAX_SIZE + 96)

#define FID_GET_VALUE 1
#define FID_SET_VALUE_CALLBACK_PERIOD 2
#define FID_GET_VALUE_CALLBACK_PERIOD 3
#define FID_SET_VALUE_CALLBACK_THRESHOLD 4
#define FID_GET_VALUE_CALLBACK_THRESHOLD 5
#define FID_SET_DEBOUNCE_PERIOD 6
#define FID_GET_DEBOUNCE_PERIOD 7
#define FID_SET_MOVING_AVERAGE 8
#define FID_GET_MOVING_AVERAGE 9
#define FID_SET_DETECTOR_TYPE 10
#define FID_GET_DETECTOR_TYPE 11
#define FID_HEATER_ON 12
#define FID_HEATER_OFF 13
#define FID_IS_HEATER_ON 14
#define FID_VALUE 15
#define FID_VALUE_REACHED 16

typedef struct {
	MessageHeader header;
} __attribute__((__packed__)) StandardMessage;

typedef struct {
	MessageHeader header;
	uint8_t length;
} __attribute__((__packed__)) SetMovingAverage;

typedef struct {
	MessageHeader header;
} __attribute__((__packed__)) GetMovingAverage;

typedef struct {
	MessageHeader header;
	uint8_t length;
} __attribute__((__packed__)) GetMovingAverageReturn;

typedef struct {
	MessageHeader header;
	uint8_t detector_type;
} __attribute__((__packed__)) SetDetectorType;

typedef struct {
	MessageHeader header;
} __attribute__((__packed__)) GetDetectorType;

typedef struct {
	MessageHeader header;
	uint8_t detector_type;
} __attribute__((__packed__)) GetDetectorTypeReturn;

typedef struct {
	MessageHeader header;
} __attribute__((__packed__)) HeaterOn;

typedef struct {
	MessageHeader header;
} __attribute__((__packed__)) HeaterOff;

typedef struct {
	MessageHeader header;
} __attribute__((__packed__)) IsHeaterOn;

typedef struct {
	MessageHeader header;
	bool heater;
} __attribute__((__packed__)) IsHeaterOnReturn;

void set_moving_average(const ComType com, const SetMovingAverage *data);
void get_moving_average(const ComType com, const GetMovingAverage *data);
void set_detector_type(const ComType com, const SetDetectorType *data);
void get_detector_type(const ComType com, const GetDetectorType *data);
void heater_on(const ComType com, const HeaterOn *data);
void heater_off(const ComType com, const HeaterOff *data);
void is_heater_on(const ComType com, const IsHeaterOn *data);

void write_detector_type_to_eeprom(void);
void read_detector_type_from_eeprom(void);
void update_detector_type(void);
void reinitialize_moving_average(void);
int32_t from_analog_value(const int32_t value);

void invocation(const ComType com, const uint8_t *data);
void constructor(void);
void destructor(void);
void tick(const uint8_t tick_type);

#endif
