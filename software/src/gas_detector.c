/* gas-detector-bricklet
 * Copyright (C) 2015 Olaf Lüke <olaf@tinkerforge.com>
 *
 * gas_detector.c: Implementation of Gas Detector Bricklet messages
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

#include "gas_detector.h"

#include "bricklib/bricklet/bricklet_communication.h"
#include "bricklib/drivers/adc/adc.h"
#include "bricklib/utility/util_definitions.h"
#include "brickletlib/bricklet_entry.h"
#include "brickletlib/bricklet_simple.h"
#include "config.h"

#define SIMPLE_UNIT_VALUE 0

const SimpleMessageProperty smp[] = {
	{SIMPLE_UNIT_VALUE, SIMPLE_TRANSFER_VALUE, SIMPLE_DIRECTION_GET}, // TYPE_GET_VALUE
	{SIMPLE_UNIT_VALUE, SIMPLE_TRANSFER_PERIOD, SIMPLE_DIRECTION_SET}, // TYPE_SET_VALUE_CALLBACK_PERIOD
	{SIMPLE_UNIT_VALUE, SIMPLE_TRANSFER_PERIOD, SIMPLE_DIRECTION_GET}, // TYPE_GET_VALUE_CALLBACK_PERIOD
	{SIMPLE_UNIT_VALUE, SIMPLE_TRANSFER_THRESHOLD, SIMPLE_DIRECTION_SET}, // TYPE_SET_VALUE_CALLBACK_THRESHOLD
	{SIMPLE_UNIT_VALUE, SIMPLE_TRANSFER_THRESHOLD, SIMPLE_DIRECTION_GET}, // TYPE_GET_VALUE_CALLBACK_THRESHOLD
	{0, SIMPLE_TRANSFER_DEBOUNCE, SIMPLE_DIRECTION_SET}, // TYPE_SET_DEBOUNCE_PERIOD
	{0, SIMPLE_TRANSFER_DEBOUNCE, SIMPLE_DIRECTION_GET}, // TYPE_GET_DEBOUNCE_PERIOD
};

const SimpleUnitProperty sup[] = {
	{from_analog_value, SIMPLE_SIGNEDNESS_INT, FID_VALUE, FID_VALUE_REACHED, SIMPLE_UNIT_VALUE}, // gas value
};

const uint8_t smp_length = sizeof(smp);


void invocation(const ComType com, const uint8_t *data) {
	switch(((MessageHeader*)data)->fid) {
		case FID_GET_VALUE:
		case FID_SET_VALUE_CALLBACK_PERIOD:
		case FID_GET_VALUE_CALLBACK_PERIOD:
		case FID_SET_VALUE_CALLBACK_THRESHOLD:
		case FID_GET_VALUE_CALLBACK_THRESHOLD:
		case FID_SET_DEBOUNCE_PERIOD:
		case FID_GET_DEBOUNCE_PERIOD: {
			simple_invocation(com, data);
			break;
		}

		case FID_SET_MOVING_AVERAGE: {
			set_moving_average(com, (SetMovingAverage*)data);
			break;
		}

		case FID_GET_MOVING_AVERAGE: {
			get_moving_average(com, (GetMovingAverage*)data);
			break;
		}

		case FID_SET_DETECTOR_TYPE: {
			set_detector_type(com, (SetDetectorType*)data);
			break;
		}

		case FID_GET_DETECTOR_TYPE: {
			get_detector_type(com, (GetDetectorType*)data);
			break;
		}

		case FID_HEATER_ON: {
			heater_on(com, (HeaterOn*)data);
			break;
		}

		case FID_HEATER_OFF: {
			heater_off(com, (HeaterOff*)data);
			break;
		}

		case FID_IS_HEATER_ON: {
			is_heater_on(com, (IsHeaterOn*)data);
			break;
		}

		default: {
			BA->com_return_error(data, sizeof(MessageHeader), MESSAGE_ERROR_CODE_NOT_SUPPORTED, com);
			break;
		}
	}
}

void constructor(void) {
	_Static_assert(sizeof(BrickContext) <= BRICKLET_CONTEXT_MAX_SIZE, "BrickContext too big");

	PIN_AD.type = PIO_INPUT;
	PIN_AD.attribute = PIO_DEFAULT;
    BA->PIO_Configure(&PIN_AD, 1);

    PIN_200K.type = PIO_OUTPUT_0;
    PIN_200K.attribute = PIO_DEFAULT;
    BA->PIO_Configure(&PIN_200K, 1);

    PIN_20K.type = PIO_OUTPUT_1;
    PIN_20K.attribute = PIO_DEFAULT;
    BA->PIO_Configure(&PIN_20K, 1);

    PIN_HEATER.type = PIO_OUTPUT_1;
    PIN_HEATER.attribute = PIO_DEFAULT;
    BA->PIO_Configure(&PIN_HEATER, 1);

	adc_channel_enable(BS->adc_channel);
	// TOOD: Read detector type from EEPROM

	SLEEP_MS(2);

	BC->moving_average_upto = MAX_MOVING_AVERAGE;
	reinitialize_moving_average();

	simple_constructor();
}

void destructor(void) {
	simple_destructor();

	PIN_AD.type = PIO_INPUT;
	PIN_AD.attribute = PIO_PULLUP;
    BA->PIO_Configure(&PIN_AD, 1);

	adc_channel_disable(BS->adc_channel);
}

void reinitialize_moving_average(void) {
	int32_t initial_value = BA->adc_channel_get_data(BS->adc_channel);
	for(uint8_t i = 0; i < BC->moving_average_upto; i++) {
		BC->moving_average[i] = initial_value;
	}
	BC->moving_average_tick = 0;
	BC->moving_average_sum = initial_value*BC->moving_average_upto;
}

int32_t from_analog_value(const int32_t value) {
	uint16_t analog_data = BA->adc_channel_get_data(BS->adc_channel);
	BC->moving_average_sum = BC->moving_average_sum -
	                         BC->moving_average[BC->moving_average_tick] +
	                         analog_data;

	BC->moving_average[BC->moving_average_tick] = analog_data;
	BC->moving_average_tick = (BC->moving_average_tick + 1) % BC->moving_average_upto;

	return (BC->moving_average_sum + BC->moving_average_upto/2)/BC->moving_average_upto;
}

void set_moving_average(const ComType com, const SetMovingAverage *data) {
	if(BC->moving_average_upto != data->length) {
		if(data->length < 1) {
			BC->moving_average_upto = 1;
		} else if(data->length > MAX_MOVING_AVERAGE) {
			BC->moving_average_upto = MAX_MOVING_AVERAGE;
		} else {
			BC->moving_average_upto = data->length;
		}

		reinitialize_moving_average();
	}

	BA->com_return_setter(com, data);
}

void get_moving_average(const ComType com, const GetMovingAverage *data) {
	GetMovingAverageReturn gmar;
	gmar.header        = data->header;
	gmar.header.length = sizeof(GetMovingAverageReturn);
	gmar.length        = BC->moving_average_upto;

	BA->send_blocking_with_timeout(&gmar, sizeof(GetMovingAverageReturn), com);
}

void set_detector_type(const ComType com, const SetDetectorType *data) {
	// TODO: Change detector type
	// TOOD: Write detector type to EEPROM
	BC->detector_type = data->detector_type;
	BA->com_return_setter(com, data);
}

void get_detector_type(const ComType com, const GetDetectorType *data) {
	GetDetectorTypeReturn gdtr;
	gdtr.header        = data->header;
	gdtr.header.length = sizeof(GetDetectorTypeReturn);
	gdtr.detector_type = BC->detector_type;

	BA->send_blocking_with_timeout(&gdtr, sizeof(GetDetectorTypeReturn), com);
}

void heater_on(const ComType com, const HeaterOn *data) {
	// TODO: Turn heater on
	BC->heater = true;
	BA->com_return_setter(com, data);
}

void heater_off(const ComType com, const HeaterOff *data) {
	// TODO: Turn heater off
	BC->heater = false;
	BA->com_return_setter(com, data);
}

void is_heater_on(const ComType com, const IsHeaterOn *data) {
	IsHeaterOnReturn ihor;
	ihor.header        = data->header;
	ihor.header.length = sizeof(IsHeaterOnReturn);
	ihor.heater        = BC->heater;

	BA->send_blocking_with_timeout(&ihor, sizeof(IsHeaterOnReturn), com);
}

void tick(const uint8_t tick_type) {
	simple_tick(tick_type);
}
