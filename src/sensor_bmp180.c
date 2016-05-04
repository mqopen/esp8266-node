#include <c_types.h>
#include <osapi.h>
#include "sensor.h"
#include "bmp180.h"
#include "sensor_bmp180.h"

/* Check that at least one sensor reading is enabled. */
#if ! ENABLE_SENSOR_BMP180_TEMPERATURE && ! CONFIG_SENSOR_BMP180_PRESSURE
#error No sensor reading is enabled.
#endif

#if ENABLE_SENSOR_BMP180_TEMPERATURE
static char _sensor_bmp180_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

#if CONFIG_SENSOR_BMP180_PRESSURE
static char _sensor_bmp180_data_pressure_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * Topics.
 */
static struct sensor_str _sensor_bmp180_topics[] = {
#if ENABLE_SENSOR_BMP180_TEMPERATURE
    {
        .data = CONFIG_SENSOR_BMP180_TEMPERATURE_TOPIC,
        .len = sizeof(CONFIG_SENSOR_BMP180_TEMPERATURE_TOPIC),
    },
#endif
#if CONFIG_SENSOR_BMP180_PRESSURE
    {
        .data = CONFIG_SENSOR_BMP180_PRESSURE_TOPIC,
        .len = sizeof(CONFIG_SENSOR_BMP180_PRESSURE_TOPIC),
    },
#endif
};

/**
 * Values.
 */
static struct sensor_str _sensor_bmp180_data[] = {
#if ENABLE_SENSOR_BMP180_TEMPERATURE
    {
        .data = _sensor_bmp180_data_temperature_str,
        .len = 0,
    },
#endif
#if CONFIG_SENSOR_BMP180_PRESSURE
    {
        .data = _sensor_bmp180_data_pressure_str,
        .len = 0,
    },
#endif
};


const uint8_t sensor_topics_count = sizeof(_sensor_bmp180_topics) / sizeof(_sensor_bmp180_topics[0]);;

enum sensor_io_result sensor_read(void) {
    uint8_t i = 0;
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    enum bmp180_io_result _io_result = bmp180_read(BMP180_OSS_8);
    switch (_io_result) {
        case BMP180_IO_OK:
#if ENABLE_SENSOR_BMP180_TEMPERATURE
            _len = os_sprintf(
                _sensor_bmp180_data[i].data,
                "%d.%d",
                bmp180_data.temperature / 1000, bmp180_data.temperature % 1000);
            _sensor_bmp180_data[i++].len = _len;
#endif
#if CONFIG_SENSOR_BMP180_PRESSURE
            _len = os_sprintf(
                _sensor_bmp180_data[i].data,
                "%d",
                bmp180_data.pressure);
            _sensor_bmp180_data[i++].len = _len;
#endif
            return SENSOR_IO_OK;
        case BMP180_IO_WRITE_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_ADDRESS");
            break;
        case BMP180_IO_WRITE_REGISTER_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_REGISTER");
            break;
        case BMP180_IO_WRITE_VALUE_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_VALUE");
            break;
        case BMP180_IO_READ_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_READ_ADDRESS");
            break;
        case BMP180_IO_INVALID_DATA:
            _len = os_sprintf(_buf, "E_INVALID_DATA");
            break;
    }

    for (i = 0; i < sensor_topics_count; i++) {
        os_memcpy(_sensor_bmp180_data[i].data, _buf, _len);
        _sensor_bmp180_data[i].len = _len;
    }
    return SENSOR_IO_ERROR;
}

char *sensor_get_topic(uint8_t index, uint8_t *buf_len) {
    *buf_len = _sensor_bmp180_topics[index].len;
    return _sensor_bmp180_topics[index].data;
}

char *sensor_get_value(uint8_t index, uint8_t *buf_len) {
    *buf_len = _sensor_bmp180_data[index].len;
    return _sensor_bmp180_data[index].data;
}
