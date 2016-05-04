#include <c_types.h>
#include <osapi.h>
#include "sensor.h"
#include "bh1750fvi.h"
#include "sensor_bh1750fvi.h"

static char _sensor_bh1750fvi_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];

static struct sensor_str _sensor_bh1750fvi_topics = {
    .data = CONFIG_SENSOR_BH1750FVI_AMBIENTLIGHT_TOPIC,
    .len = sizeof(CONFIG_SENSOR_BH1750FVI_AMBIENTLIGHT_TOPIC),
};

static struct sensor_str _sensor_bh1750fvi_data = {
    .data = _sensor_bh1750fvi_data_temperature_str,
    .len = 0,
};

const uint8_t sensor_topics_count = 1;

enum sensor_io_result sensor_read(void) {
    uint8_t _len = 0;
    char _buf[SENSOR_VALUE_BUFFER_SIZE];
    enum bh1750fvi_io_result _io_result = bh1750fvi_read();
    switch (_io_result) {
        case BH1750FVI_IO_OK:
            _len = os_sprintf(
                _sensor_bh1750fvi_data.data,
                "%d",
                bh1750fvi_data);
            _sensor_bh1750fvi_data.len = _len;
            return SENSOR_IO_OK;
        case BH1750FVI_IO_WRITE_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_ADDRESS");
            break;
        case BH1750FVI_IO_WRITE_REGISTER_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_REGISTER");
            break;
        case BH1750FVI_IO_WRITE_VALUE_ERROR:
            _len = os_sprintf(_buf, "E_WRITE_VALUE");
            break;
        case BH1750FVI_IO_READ_ADDRESS_ERROR:
            _len = os_sprintf(_buf, "E_READ_ADDRESS");
            break;
        case BH1750FVI_IO_INVALID_DATA:
            _len = os_sprintf(_buf, "E_INVALID_DATA");
            break;
    }

    os_memcpy(_sensor_bh1750fvi_data.data, _buf, _len);
    _sensor_bh1750fvi_data.len = _len;
    return SENSOR_IO_ERROR;
}

char *sensor_get_topic(uint8_t index, uint8_t *buf_len) {
    *buf_len = _sensor_bh1750fvi_topics.len;
    return _sensor_bh1750fvi_topics.data;
}

char *sensor_get_value(uint8_t index, uint8_t *buf_len) {
    *buf_len = _sensor_bh1750fvi_data.len;
    return _sensor_bh1750fvi_data.data;
}
