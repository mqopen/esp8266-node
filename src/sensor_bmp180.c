#include <c_types.h>
#include <osapi.h>
#include "sensor.h"
#include "dht22.h"
#include "sensor_bmp180.h"

#if ENABLE_SENSOR_BMP180_TEMPERATURE
static char _sensor_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

#if CONFIG_SENSOR_BMP180_PRESSURE
static char _sensor_data_pressure_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

const uint8_t sensor_topics_count = 2;

enum sensor_io_result sensor_read(void) {
    return SENSOR_IO_OK;
}

char *sensor_get_topic(uint8_t index, uint8_t *buf_len) {
    return 0;
}

char *sensor_get_value(uint8_t index, uint8_t *buf_len) {
    return 0;
}
