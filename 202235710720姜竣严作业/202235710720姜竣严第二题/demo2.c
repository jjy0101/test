#include <stdio.h>

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#include "hi_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "hi_errno.h"
#include "hi_pwm.h"
#include <unistd.h>
#include <hi_types_base.h>


#define APP_DEMO_ADC

#include <hi_adc.h>
#include <hi_stdlib.h>
#include <hi_early_debug.h>
#include <iot_errno.h>

#define RED_LED_PIN_NAME        10
#define GREEN_LED_PIN_NAME      11
#define BLUE_LED_PIN_NAME       12
#define HUMAN_SENSOR_CHAN_NAME  3
#define LIGHT_SENSOR_CHAN_NAME  4

#define LED_PWM_FUNCTION        5
#define IOT_PWM_PORT_PWM1       1
#define IOT_PWM_PORT_PWM2       2
#define IOT_PWM_PORT_PWM3       3
#define IOT_PWM_PORT_PWM_MAX    4
#define IOT_PWM_DUTY_MAX        100

#define LED_BRIGHT              IOT_GPIO_VALUE1
#define LED_DARK                IOT_GPIO_VALUE0

#define NUM_BLINKS              2
#define NUM_SENSORS             2
#define NUM_2                   2
#define NUM_3                   3

#define ADC_RESOLUTION          4096
#define PWM_FREQ_DIVITION       64000
#define CLK_160M                160000000

#define STACK_SIZE             (4096)
#define DELAY_300MS            (300 * 1000)
#define DELAY_10MS             (10 * 1000)

int flag = 0;


static void ADCTask(void)
{
    static const unsigned int pins[] = {RED_LED_PIN_NAME, GREEN_LED_PIN_NAME, BLUE_LED_PIN_NAME};

    // set Red/Green/Blue LED pin to pwm function
    hi_io_set_func(RED_LED_PIN_NAME, LED_PWM_FUNCTION);
    hi_io_set_func(GREEN_LED_PIN_NAME, LED_PWM_FUNCTION);
    hi_io_set_func(BLUE_LED_PIN_NAME, LED_PWM_FUNCTION);

    IoTPwmInit(IOT_PWM_PORT_PWM1); // R
    IoTPwmInit(IOT_PWM_PORT_PWM2); // G
    IoTPwmInit(IOT_PWM_PORT_PWM3); // B

    for (int i = 1; i < IOT_PWM_PORT_PWM_MAX; i++) {
        // use PWM control BLUE LED brightness
        for (int j = 1; j <= 60; j *= NUM_2) {
            IoTPwmStart(i, j, CLK_160M / PWM_FREQ_DIVITION);
            usleep(DELAY_300MS);
            IoTPwmStop(i);
        }
    }
           

    while (1) {
        unsigned short duty[NUM_SENSORS] = {0, 0};
        unsigned short data[NUM_SENSORS] = {0, 0};
        static const int chan[] = {HUMAN_SENSOR_CHAN_NAME, LIGHT_SENSOR_CHAN_NAME};
        static const int port[] = {IOT_PWM_PORT_PWM1, IOT_PWM_PORT_PWM2};
        if (hi_adc_read(HUMAN_SENSOR_CHAN_NAME, &data[0], HI_ADC_EQU_MODEL_8, HI_ADC_CUR_BAIS_DEFAULT, 0)
            == IOT_SUCCESS) {
            unsigned int value = (unsigned int)data[0];
            printf("duty[0] = %d\n",value);
            if(value >= 200) flag = 1;
            else flag = 0;
        }
        if(flag)
        {
            if (hi_adc_read(LIGHT_SENSOR_CHAN_NAME, &data[1], HI_ADC_EQU_MODEL_8, HI_ADC_CUR_BAIS_DEFAULT, 0)
            == IOT_SUCCESS) {
                unsigned int value = (unsigned int)data[1]/10%100;
                printf("value = %d\n",value);
                IoTPwmStart(3, value, PWM_FREQ_DIVITION);
                usleep(100);
            }
        }
        else  IoTPwmStop(3);
        usleep(100000);
    }
}



void Adc_demo(void)
{
    osThreadAttr_t attr;

    hi_gpio_init();
    hi_io_set_func(HI_GPIO_IDX_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_IN);

    hi_io_set_func(HI_GPIO_IDX_7, HI_IO_FUNC_GPIO_7_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_7, HI_GPIO_DIR_IN);

    IoTGpioInit(RED_LED_PIN_NAME);
    IoTGpioInit(GREEN_LED_PIN_NAME);
    IoTGpioInit(BLUE_LED_PIN_NAME);

    // set Red/Green/Blue LED pin as output
    IoTGpioSetDir(RED_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(GREEN_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(BLUE_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    
    attr.name = "ADCTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = 26;

    if (osThreadNew(ADCTask, NULL, &attr) == NULL) {
        printf("[ADCTask] Falied to create KeyTask!\n");
    }
    
}


SYS_RUN(Adc_demo);

