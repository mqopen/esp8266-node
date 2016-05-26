#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include <c_types.h>

/* Check if pins aren't same. */
#if CONFIG_BUS_I2C_GPIO_SDA == CONFIG_BUS_I2C_GPIO_SCL
  #error I2C SDA and SCL are defined at same GPIO pin!
#endif

/**
 * SDA pin.
 */
#define I2C_MASTER_SDA_GPIO CONFIG_BUS_I2C_GPIO_SDA
#if CONFIG_BUS_I2C_GPIO_SDA == 0
  #define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO0_U
  #define I2C_MASTER_SDA_FUNC FUNC_GPIO0
#elif CONFIG_BUS_I2C_GPIO_SDA == 2
  #define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_GPIO2_U
  #define I2C_MASTER_SDA_FUNC FUNC_GPIO2
#elif CONFIG_BUS_I2C_GPIO_SDA == 13
  #define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_MTCK_U
  #define I2C_MASTER_SDA_FUNC FUNC_GPIO13
#elif CONFIG_BUS_I2C_GPIO_SDA == 14
  #define I2C_MASTER_SDA_MUX PERIPHS_IO_MUX_MTMS_U
  #define I2C_MASTER_SDA_FUNC FUNC_GPIO14
#else
  #error Unsupported I2C SDA pin number!
#endif

/**
 * SCL pin.
 */
#define I2C_MASTER_SCL_GPIO CONFIG_BUS_I2C_GPIO_SCL
#if CONFIG_BUS_I2C_GPIO_SCL == 0
  #define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO0_U
  #define I2C_MASTER_SCL_FUNC FUNC_GPIO0
#elif CONFIG_BUS_I2C_GPIO_SCL == 2
  #define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_GPIO2_U
  #define I2C_MASTER_SCL_FUNC FUNC_GPIO2
#elif CONFIG_BUS_I2C_GPIO_SCL == 13
  #define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTCK_U
  #define I2C_MASTER_SCL_FUNC FUNC_GPIO13
#elif CONFIG_BUS_I2C_GPIO_SCL == 14
  #define I2C_MASTER_SCL_MUX PERIPHS_IO_MUX_MTMS_U
  #define I2C_MASTER_SCL_FUNC FUNC_GPIO14
#else
  #error Unsupported I2C SCL pin number!
#endif

#if 0
#define I2C_MASTER_GPIO_SET(pin)  \
    gpio_output_set(1<<pin,0,1<<pin,0)

#define I2C_MASTER_GPIO_CLR(pin) \
    gpio_output_set(0,1<<pin,1<<pin,0)

#define I2C_MASTER_GPIO_OUT(pin,val) \
    if(val) I2C_MASTER_GPIO_SET(pin);\
    else I2C_MASTER_GPIO_CLR(pin)
#endif

#define I2C_MASTER_SDA_HIGH_SCL_HIGH()  \
    gpio_output_set(1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_HIGH_SCL_LOW()  \
    gpio_output_set(1<<I2C_MASTER_SDA_GPIO, 1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_LOW_SCL_HIGH()  \
    gpio_output_set(1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_LOW_SCL_LOW()  \
    gpio_output_set(0, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

void i2c_master_gpio_init(void);
void i2c_master_init(void);

#define i2c_master_wait    os_delay_us
void i2c_master_stop(void);
void i2c_master_start(void);
void i2c_master_setAck(uint8 level);
uint8 i2c_master_getAck(void);
uint8 i2c_master_readByte(void);
void i2c_master_writeByte(uint8 wrdata);

bool i2c_master_checkAck(void);
void i2c_master_send_ack(void);
void i2c_master_send_nack(void);

#endif
