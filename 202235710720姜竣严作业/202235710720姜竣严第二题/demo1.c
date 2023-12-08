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

#define     STACK_SIZE             (1024)
#define     PWM_DUTY_50            (50)
#define     PWM_FREQ_4K            (4000)
#define KEY_EVENT_NONE      0
#define KEY_EVENT_S1      1
#define KEY_EVENT_S2      2
#define KEY_EVENT_S3      3
#define KEY_EVENT_S4      4
#define IOT_GPIO_IDX_9    9
#define     IOT_GPIO_PWM_FUNCTION   5
#define LED_INTERVAL_TIME_US 300000
#define LED_TASK_STACK_SIZE 512
#define LED_TASK_PRIO 25
#define     IOT_PWM_PORT_PWM0       0

#define LED_TEST_GPIO 10 // for hispark_pegasus
#define LED_TEST1_GPIO 12 // for hispark_pegasus
#define LED_TEST2_GPIO 11 // for hispark_pegasus
#define ADC_TEST_LENGTH  64
#define VLT_MIN 100
static int g_beepState = 0;

hi_u16 g_adc_buf[ADC_TEST_LENGTH] = { 0 };



int key_status = KEY_EVENT_NONE;
char key_flg = 0;


int get_key_event(void)
{
    int tmp = key_status;
    key_status = KEY_EVENT_NONE;
    return tmp;
}

/* asic adc test  */
hi_void convert_to_voltage(hi_u32 data_len)
{
    hi_u32 i;
    float vlt_max = 0;
    float vlt_min = VLT_MIN;

    float vlt_val = 0;

    hi_u16 vlt;
    for (i = 0; i < data_len; i++) {
        vlt = g_adc_buf[i];
        float voltage = (float)vlt * 1.8 * 4 / 4096.0;  /* vlt * 1.8 * 4 / 4096.0: Convert code into voltage */
        vlt_max = (voltage > vlt_max) ? voltage : vlt_max;
        vlt_min = (voltage < vlt_min) ? voltage : vlt_min;
    }
    printf("vlt_min:%.3f, vlt_max:%.3f \n", vlt_min, vlt_max);

    vlt_val = (vlt_min + vlt_max)/2.0;

    if((vlt_val > 0.4) && (vlt_val < 0.6))
    {
        if(key_flg == 0)
        {
            key_flg = 1;
            key_status = KEY_EVENT_S1;
        }
    }
    if((vlt_val > 0.8) && (vlt_val < 1.1))
    {
        if(key_flg == 0)
        {
            key_flg = 1;
            key_status = KEY_EVENT_S2;
        }
    }

    if((vlt_val > 0.01) && (vlt_val < 0.3))
    {
        if(key_flg == 0)
        {
            key_flg = 1;
            key_status = KEY_EVENT_S3;
        }
    }

    if(vlt_val > 3.0)
    {
        key_flg = 0;
        key_status = KEY_EVENT_NONE;
    }
}




void app_demo_adc_test(void)
{
    hi_u32 ret, i;
    hi_u16 data;  /* 10 */

    memset_s(g_adc_buf, sizeof(g_adc_buf), 0x0, sizeof(g_adc_buf));
 
    for (i = 0; i < ADC_TEST_LENGTH; i++) {
        ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_2, &data, HI_ADC_EQU_MODEL_1, HI_ADC_CUR_BAIS_DEFAULT, 0);
        if (ret != HI_ERR_SUCCESS) {
            printf("ADC Read Fail\n");
            return;
        }
        g_adc_buf[i] = data;
    }
    convert_to_voltage(ADC_TEST_LENGTH);

}

/* ���� �����ж���Ӧ */
void my_gpio_isr_demo(void *arg)
{
    arg = arg;
    hi_u32 ret;

    printf("----- gpio isr demo -----\r\n");

    (hi_void)hi_gpio_init();
    
    hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO); /* uart1 rx */

    ret = hi_gpio_set_dir(HI_GPIO_IDX_5, HI_GPIO_DIR_IN);
    if (ret != HI_ERR_SUCCESS) {
        printf("===== ERROR ======gpio -> hi_gpio_set_dir1 ret:%d\r\n", ret);
        return;
    }

    int count = 1000;
    while(count--)
    {
        //��ȡADCֵ
        app_demo_adc_test();

        switch(get_key_event())
        {
            case KEY_EVENT_NONE:
            {
                IoTGpioSetOutputVal(LED_TEST_GPIO, 0);
                IoTGpioSetOutputVal(LED_TEST1_GPIO, 0);
                IoTGpioSetOutputVal(LED_TEST2_GPIO, 0);
                IoTPwmStop(IOT_PWM_PORT_PWM0);
                usleep(LED_INTERVAL_TIME_US);
            }
            break;

            case KEY_EVENT_S1:
            {
                IoTGpioSetOutputVal(LED_TEST_GPIO, 0);
                IoTGpioSetOutputVal(LED_TEST1_GPIO, 1);
                IoTGpioSetOutputVal(LED_TEST2_GPIO, 0);
                IoTPwmStart(IOT_PWM_PORT_PWM0, PWM_DUTY_50, PWM_FREQ_4K);
                usleep(LED_INTERVAL_TIME_US);
                printf("KEY_EVENT_S1 \r\n");
            }
            break;

            case KEY_EVENT_S2:
            {
                IoTGpioSetOutputVal(LED_TEST_GPIO, 0);
                IoTGpioSetOutputVal(LED_TEST1_GPIO, 0);
                IoTGpioSetOutputVal(LED_TEST2_GPIO, 1);
                IoTPwmStart(IOT_PWM_PORT_PWM0, PWM_DUTY_50, PWM_FREQ_4K);
                usleep(LED_INTERVAL_TIME_US);
                printf("KEY_EVENT_S2 \r\n");
            }
            break;

            case KEY_EVENT_S3:
            {
                IoTGpioSetOutputVal(LED_TEST_GPIO, 1);
                IoTGpioSetOutputVal(LED_TEST1_GPIO, 0);
                IoTGpioSetOutputVal(LED_TEST2_GPIO, 0);
                IoTPwmStart(IOT_PWM_PORT_PWM0, PWM_DUTY_50, PWM_FREQ_4K);
                usleep(LED_INTERVAL_TIME_US);
                printf("KEY_EVENT_S3 \r\n");
            }
            break;

        }
    }
    

}



void key_demo(void)
{
    osThreadAttr_t attr;

    IoTGpioInit(LED_TEST_GPIO);
    IoTGpioInit(LED_TEST1_GPIO);
    IoTGpioInit(LED_TEST2_GPIO);
    
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(LED_TEST1_GPIO, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(LED_TEST2_GPIO, IOT_GPIO_DIR_OUT);

    IoTGpioInit(IOT_GPIO_IDX_9);
    hi_io_set_func(IOT_GPIO_IDX_9, IOT_GPIO_PWM_FUNCTION);
    IoTGpioSetDir(IOT_GPIO_IDX_9, IOT_GPIO_DIR_OUT);
    IoTPwmInit(IOT_PWM_PORT_PWM0);
    
    attr.name = "KeyTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = 26;

    if (osThreadNew((osThreadFunc_t)my_gpio_isr_demo, NULL, &attr) == NULL) {
        printf("[key_demo] Falied to create KeyTask!\n");
    }
    
}


SYS_RUN(key_demo);

