#ifndef __PARAMS_MANAGE_H__
#define __PARAMS_MANAGE_H__
 
#include <stdint.h>
 
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
 
 
#define FMC_PAGE_SIZE           ((uint16_t)0x400U)
#define FMC_WRITE_START_ADDR    ((uint32_t)0x0800FC00U)//最后1KB保存参数
#define FMC_WRITE_END_ADDR			((uint32_t)0x0800FFFFU)

typedef struct _CONFIG_PARAMS
{
    uint8_t  data_valid_flag;
    uint8_t  test_completed;
    uint32_t uuid[16];
    uint32_t key[32];
}CONFIG_PARAMS;

typedef union _CONFIG_PARAMS_UNION
{
    CONFIG_PARAMS   config_params;
    uint32_t        data[64];
}CONFIG_PARAMS_UNION;

CONFIG_PARAMS *get_config_params();
uint8_t         save_config_params(CONFIG_PARAMS *params);
uint8_t         load_config_params();
uint8_t         is_config_params_avaliable();

//test api
int     add_uuidkey_auth(uint8_t uuid_len,uint8_t key_len);
void    set_test_completed(uint8_t completed);
#ifdef __cplusplus
}
#endif
#endif