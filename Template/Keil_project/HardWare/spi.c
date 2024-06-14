#include "spi0.h"
#include "gd32l23x_rcu.h"

#define SET_SPI0_NSS_HIGH          gpio_bit_set(GPIOA,GPIO_PIN_4);
#define SET_SPI0_NSS_LOW           gpio_bit_reset(GPIOA,GPIO_PIN_4);

void spi0_rcu_config(void){
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
}

void spi0_gpio_config(void){
    /* configure SPI0 GPIO : SCK/PA5, MISO/PA6, MOSI/PA7 */
    gpio_af_set(GPIOA, GPIO_AF_5,  GPIO_PIN_5);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_af_set(GPIOA, GPIO_AF_5,  GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7);
    /* set GPIO PA4 as SPI0 NSS output */
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
}

void spi0_config(void){
    spi_parameter_struct spi_init_struct;
    /* deinitialize SPI and the parameters */
    spi_i2s_deinit(SPI0);

    spi_struct_para_init(&spi_init_struct);

    /* configure SPI0 parameter */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
    spi_init_struct.prescale             = SPI_PSC_8;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init(SPI0, &spi_init_struct);
}

void spi0_init(){
    spi0_rcu_config();
    spi0_gpio_config();
    SET_SPI0_NSS_HIGH
    spi0_config();
    spi_enable(SPI0);
    SET_SPI0_NSS_LOW
}