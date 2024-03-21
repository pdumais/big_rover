#include "lcd.h"
#include <stdarg.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#define LCD_RS GPIO_NUM_13
#define LCD_E GPIO_NUM_12
#define LCD_D4 GPIO_NUM_22
#define LCD_D5 GPIO_NUM_32
#define LCD_D6 GPIO_NUM_33
#define LCD_D7 GPIO_NUM_25

static const char *TAG = "lcd_c";

static void lcd_pulse()
{
    gpio_set_level(LCD_E, 1);
    ets_delay_us(50);
    gpio_set_level(LCD_E, 0);
    ets_delay_us(50);
}

static void lcd_send_command(uint8_t cmd)
{
    gpio_set_level(LCD_RS, 0);

    gpio_set_level(LCD_D4, (cmd >> 4) & 1);
    gpio_set_level(LCD_D5, (cmd >> 5) & 1);
    gpio_set_level(LCD_D6, (cmd >> 6) & 1);
    gpio_set_level(LCD_D7, (cmd >> 7) & 1);
    lcd_pulse();

    gpio_set_level(LCD_D4, (cmd >> 0) & 1);
    gpio_set_level(LCD_D5, (cmd >> 1) & 1);
    gpio_set_level(LCD_D6, (cmd >> 2) & 1);
    gpio_set_level(LCD_D7, (cmd >> 3) & 1);
    lcd_pulse();
}
void lcd_write_string(int line, const char *format, ...)
{
    char buffer[16];
    va_list al;
    va_start(al, format);
    uint16_t len = vsnprintf(buffer, 16, format, al);
    va_end(al);

    lcd_send_command(0x80 | (line * 16));
    ets_delay_us(60);

    gpio_set_level(LCD_RS, 1);
    for (int i = 0; i < len; i++)
    {
        unsigned char c = buffer[i];
        if (c == 0)
        {
            break;
        }
        gpio_set_level(LCD_D4, (c >> 4) & 1);
        gpio_set_level(LCD_D5, (c >> 5) & 1);
        gpio_set_level(LCD_D6, (c >> 6) & 1);
        gpio_set_level(LCD_D7, (c >> 7) & 1);
        lcd_pulse();

        gpio_set_level(LCD_D4, (c >> 0) & 1);
        gpio_set_level(LCD_D5, (c >> 1) & 1);
        gpio_set_level(LCD_D6, (c >> 2) & 1);
        gpio_set_level(LCD_D7, (c >> 3) & 1);
        lcd_pulse();
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    for (int i = len; i < 16; i++)
    {
        gpio_set_level(LCD_D4, 0);
        gpio_set_level(LCD_D5, 1);
        gpio_set_level(LCD_D6, 0);
        gpio_set_level(LCD_D7, 0);
        lcd_pulse();

        gpio_set_level(LCD_D4, 0);
        gpio_set_level(LCD_D5, 0);
        gpio_set_level(LCD_D6, 0);
        gpio_set_level(LCD_D7, 0);
        lcd_pulse();
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

static void set_pin_out(uint8_t pin)
{
    ESP_LOGI(TAG, "Set pin out %x", pin);
    gpio_config_t o_conf = {};
    o_conf.pin_bit_mask = (1ULL << pin);
    o_conf.intr_type = GPIO_INTR_DISABLE;
    o_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&o_conf);
}

void lcd_init()
{
    set_pin_out(LCD_RS);
    set_pin_out(LCD_E);
    set_pin_out(LCD_D4);
    set_pin_out(LCD_D5);
    set_pin_out(LCD_D6);
    set_pin_out(LCD_D7);

    gpio_set_level(LCD_E, 0);
    gpio_set_level(LCD_RS, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait 1s for LCD to init properly

    for (int i = 0; i < 3; i++)
    {
        gpio_set_level(LCD_D4, 1);
        gpio_set_level(LCD_D5, 1);
        gpio_set_level(LCD_D6, 0);
        gpio_set_level(LCD_D7, 0);
        lcd_pulse();
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    gpio_set_level(LCD_D4, 0);
    gpio_set_level(LCD_D5, 1);
    gpio_set_level(LCD_D6, 0);
    gpio_set_level(LCD_D7, 0);
    lcd_pulse();
    vTaskDelay(5 / portTICK_PERIOD_MS);

    lcd_send_command(0x2C);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    lcd_send_command(0x0F);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    lcd_send_command(0x01);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    lcd_send_command(0x06);
    vTaskDelay(5 / portTICK_PERIOD_MS);
}
