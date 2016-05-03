#include <osapi.h>
#include "sensor.h"
#include "dht22.h"
#include "sensor_dht22.h"

/* Check that at least one sensor reading is enabled. */
#if ! ENABLE_SENSOR_DHT22_TEMPERATURE && ! ENABLE_SENSOR_DHT22_HUMIDITY
#error No sensor reading is enabled.
#endif

/**
 * Temperature data.
 */
#if ENABLE_SENSOR_DHT22_TEMPERATURE
static char _sensor_data_temperature_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * humidity data.
 */
#if ENABLE_SENSOR_DHT22_HUMIDITY
static char _sensor_data_humidity_str[SENSOR_VALUE_BUFFER_SIZE];
#endif

/**
 * Topics.
 */
static struct sensor_str sensor_topics[] = {
#if ENABLE_SENSOR_DHT22_TEMPERATURE
    {
        .data = CONFIG_SENSOR_DHT22_TEMPERATURE_TOPIC,
        .len = sizeof(CONFIG_SENSOR_DHT22_TEMPERATURE_TOPIC),
    },
#endif
#if ENABLE_SENSOR_DHT22_HUMIDITY
    {
        .data = CONFIG_SENSOR_DHT22_HUMIDITY_TOPIC,
        .len = sizeof(CONFIG_SENSOR_DHT22_HUMIDITY_TOPIC),
    },
#endif
};

/**
 * Values.
 */
static struct sensor_str sensor_data[] = {
#if ENABLE_SENSOR_DHT22_TEMPERATURE
    {
        .data = _sensor_data_temperature_str,
        .len = 0,
    },
#endif
#if ENABLE_SENSOR_DHT22_HUMIDITY
    {
        .data = _sensor_data_humidity_str,
        .len = 0,
    },
#endif
};

const uint8_t sensor_topics_count = sizeof(sensor_topics) / sizeof(sensor_topics[0]);

enum sensor_io_result sensor_read(void) {
    uint8_t i = 0;
    uint8_t _len = 0;
    char buf[SENSOR_VALUE_BUFFER_SIZE];
    enum dht22_io_result _io_result = dht22_read();
    switch (_io_result) {
        case DHT22_IO_OK:
#if ENABLE_SENSOR_DHT22_TEMPERATURE
            _len = os_sprintf(
                sensor_data[i].data,
                "%d.%d",
                dht22_data.temperature / 1000, dht22_data.temperature % 1000);
            sensor_data[i++].len = _len;
#endif
#if ENABLE_SENSOR_DHT22_HUMIDITY
            _len = os_sprintf(
                sensor_data[i].data,
                "%d.%d",
                dht22_data.humidity / 1000, dht22_data.humidity % 1000);
            sensor_data[i++].len = _len;
#endif
            return SENSOR_IO_OK;
        case DHT22_IO_WRITE_ADDRESS_ERROR:
            _len = os_sprintf(sensor_data[i].data, "E_WRITE_ADDRESS");
            break;
        case DHT22_IO_WRITE_REGISTER_ERROR:
            _len = os_sprintf(sensor_data[i].data, "E_WRITE_REGISTER");
            break;
        case DHT22_IO_WRITE_VALUE_ERROR:
            _len = os_sprintf(sensor_data[i].data, "E_WRITE_VALUE");
            break;
        case DHT22_IO_READ_ADDRESS_ERROR:
            _len = os_sprintf(sensor_data[i].data, "E_READ_ADDRESS");
            break;
        case DHT22_IO_INVALID_DATA:
            _len = os_sprintf(sensor_data[i].data, "E_INVALID_DATA");
            break;
    }
    for (i = 0; i < sensor_topics_count; i++) {
        os_memcpy(sensor_data[i].data, buf, _len);
        sensor_data[i].len = _len;
    }

    return SENSOR_IO_ERROR;
}

/*
static void ICACHE_FLASH_ATTR _mqttclient_publish(void) {
    if (!_publish_sending) {
        _publish_sending = true;
        // TODO: hardcoded constant
        char buf[20];
        uint16_t _len = 0;
        enum bmp180_io_result _io_result = bmp180_read(BMP180_OSS_SINGLE);
        switch (_io_result) {
            case BMP180_IO_OK:
                _len = os_sprintf(buf, "%d.%d", bmp180_data.temperature / 1000, bmp180_data.temperature % 1000);
                umqtt_publish(&_mqtt, CONFIG_MQTT_TOPIC_TEMPERATURE, (uint8_t *) buf, _len, 0);
                _len = os_sprintf(buf, "%d", bmp180_data.pressure);
                umqtt_publish(&_mqtt, CONFIG_MQTT_TOPIC_PRESSURE, (uint8_t *) buf, _len, 0);
                break;
            case BMP180_IO_WRITE_ADDRESS_ERROR:
                _len = os_sprintf(buf, "E_WRITE_ADDRESS");
                break;
            case BMP180_IO_WRITE_REGISTER_ERROR:
                _len = os_sprintf(buf, "E_WRITE_REGISTER");
                break;
            case BMP180_IO_WRITE_VALUE_ERROR:
                _len = os_sprintf(buf, "E_WRITE_VALUE");
                break;
            case BMP180_IO_READ_ADDRESS_ERROR:
                _len = os_sprintf(buf, "E_READ_ADDRESS");
                break;
            case BMP180_IO_INVALID_DATA:
                _len = os_sprintf(buf, "E_INVALID_DATA");
                break;
        }

        if (_io_result != BMP180_IO_OK) {
            umqtt_publish(&_mqtt, CONFIG_MQTT_TOPIC_TEMPERATURE, (uint8_t *) buf, _len, 0);
            umqtt_publish(&_mqtt, CONFIG_MQTT_TOPIC_PRESSURE, (uint8_t *) buf, _len, 0);
        }
        _mqttclient_data_send();
    }
}
 */

char *sensor_get_topic(uint8_t index, uint8_t *buf_len) {
    *buf_len = sensor_topics[index].len;
    return sensor_topics[index].data;
}

char *sensor_get_value(uint8_t index, uint8_t *buf_len) {
    *buf_len = sensor_data[index].len;
    return sensor_data[index].data;
}
