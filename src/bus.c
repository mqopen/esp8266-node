#include "bus.h"

#if ENABLE_BUS_I2C
  #include "i2c_master.h"
#endif

void bus_init(void) {
#if ENABLE_BUS_I2C
    i2c_master_gpio_init();
    i2c_master_init();
#endif
}
