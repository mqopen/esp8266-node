#ifndef __BMP180_H__
#define __BMP180_H__

#include <c_types.h>

#define BMP180_ADDRESS_WRITE    0xee
#define BMP180_ADDRESS_READ     0xef
#define BMP180_CHIP_ID          0x55

#define BMP180_REGISTER_CALIBRATION 0xaa
#define BMP180_REGISTER_CHIP_ID     0xd0

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
} __attribute__((__packed__));

enum bmp180_read_status {
    BMP180_READ_STATUS_OK,
    BMP180_READ_STATUS_WRITE_ADDRESS_ERROR,
    BMP180_READ_STATUS_WRITE_REGISTER_ERROR,
    BMP180_READ_STATUS_READ_ADDRESS_ERROR,
};

extern int16_t bmp180_temperature;

void ICACHE_FLASH_ATTR bmp180_init(void);
uint8_t ICACHE_FLASH_ATTR bmp180_get_chip_id(void);
bool ICACHE_FLASH_ATTR bmp180_test(void);
enum bmp180_read_status bmp180_read_temperature(void);

#endif
