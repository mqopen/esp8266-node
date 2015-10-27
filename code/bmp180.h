#ifndef __BMP180_H__
#define __BMP180_H__

#include <c_types.h>

#define BMP180_ADDRESS_WRITE    0xee
#define BMP180_ADDRESS_READ     0xef
#define BMP180_CHIP_ID          0x55

#define BMP180_REGISTER_CALIBRATION 0xaa
#define BMP180_REGISTER_CHIP_ID     0xd0

#define BMP180_CALIBRATION_AC1_MSB  0
#define BMP180_CALIBRATION_AC1_LSB  1
#define BMP180_CALIBRATION_AC2_MSB  2
#define BMP180_CALIBRATION_AC2_LSB  3
#define BMP180_CALIBRATION_AC3_MSB  4
#define BMP180_CALIBRATION_AC3_LSB  5
#define BMP180_CALIBRATION_AC4_MSB  6
#define BMP180_CALIBRATION_AC4_LSB  7
#define BMP180_CALIBRATION_AC5_MSB  8
#define BMP180_CALIBRATION_AC5_LSB  9
#define BMP180_CALIBRATION_AC6_MSB  10
#define BMP180_CALIBRATION_AC6_LSB  11
#define BMP180_CALIBRATION_B1_MSB   12
#define BMP180_CALIBRATION_B1_LSB   13
#define BMP180_CALIBRATION_B2_MSB   14
#define BMP180_CALIBRATION_B2_LSB   15
#define BMP180_CALIBRATION_MB_MSB   16
#define BMP180_CALIBRATION_MB_LSB   17
#define BMP180_CALIBRATION_MC_MSB   18
#define BMP180_CALIBRATION_MC_LSB   19
#define BMP180_CALIBRATION_MD_MSB   20
#define BMP180_CALIBRATION_MD_LSB   21

#define BMP180_OUT_MSB              0xf6
#define BMP180_OUT_LSB              0xf7
#define BMP180_OUT_XLSB             0xf8

struct bmp180_calibration_data {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
};

enum bmp180_read_status {
    BMP180_READ_STATUS_OK,
    BMP180_READ_STATUS_WRITE_ADDRESS_ERROR,
    BMP180_READ_STATUS_WRITE_REGISTER_ERROR,
    BMP180_READ_STATUS_READ_ADDRESS_ERROR,
};

/**
 * Sensor measurement.
 */
struct bmp180_data {
    int32_t temperature;            /**< Temperature in m degrees C. */
    uint32_t pressure;              /**< Pressure in Pa. */
};

extern struct bmp180_data bmp180_data;

void ICACHE_FLASH_ATTR bmp180_init(void);
uint8_t ICACHE_FLASH_ATTR bmp180_get_chip_id(void);
bool ICACHE_FLASH_ATTR bmp180_test(void);
enum bmp180_read_status bmp180_read(void);

#endif
