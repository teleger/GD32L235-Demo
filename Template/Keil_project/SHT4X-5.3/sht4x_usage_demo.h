#ifndef SHT4X_USAGE_DEMO_H
#define SHT4X_USAGE_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "sht4x.h"

#if 0
int     mainSHT_53(void);
#endif
int     SHT4x_init();
void    Init_SHT4X(void);
int8_t sht4x_measure_detail(int32_t *temperature, int32_t *humidity);
#ifdef __cplusplus
}
#endif

#endif 