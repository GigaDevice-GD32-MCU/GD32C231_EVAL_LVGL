#include "lv_GPIO_running_LED.h"


void lv_GPIO_running_LED_init(void)
{
    /* enable the LEDs GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOB);

    /* configure LEDs GPIO pin */
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL_1, GPIO_PIN_15);

    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL_1, GPIO_PIN_1);
    
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL_1, GPIO_PIN_3);
    
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL_1, GPIO_PIN_4);

    /* reset LEDs GPIO pin */
    gpio_bit_reset(GPIOA, GPIO_PIN_15);
    gpio_bit_reset(GPIOD, GPIO_PIN_1);
    gpio_bit_reset(GPIOD, GPIO_PIN_3);
    gpio_bit_reset(GPIOB, GPIO_PIN_4);
}


void lv_GPIO_running_LED_on(void)
{
    gpio_bit_set(GPIOA, GPIO_PIN_15);
        gpio_bit_reset(GPIOD, GPIO_PIN_1);
        gpio_bit_reset(GPIOD, GPIO_PIN_3);
        gpio_bit_reset(GPIOB, GPIO_PIN_4);
        delay_1ms(500);

        /* turn on LED2, turn off other LED  */
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        gpio_bit_set(GPIOD, GPIO_PIN_1);
        gpio_bit_reset(GPIOD, GPIO_PIN_3);
        gpio_bit_reset(GPIOB, GPIO_PIN_4);
        delay_1ms(500);
        
        /* turn on LED3, turn off other LED  */
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        gpio_bit_reset(GPIOD, GPIO_PIN_1);
        gpio_bit_set(GPIOD, GPIO_PIN_3);
        gpio_bit_reset(GPIOB, GPIO_PIN_4);
        delay_1ms(500);
        
        /* turn on LED4, turn off other LED  */
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        gpio_bit_reset(GPIOD, GPIO_PIN_1);
        gpio_bit_reset(GPIOD, GPIO_PIN_3);
        gpio_bit_set(GPIOB, GPIO_PIN_4);
        delay_1ms(500);
}
