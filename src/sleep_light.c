
#include "spi_ws2812.h"
#include "sleep_light.h"
#include "esp_log.h"
#include "nvs_storage.h"
#include "nvs_flash.h"
#include "nvs.h"

#define LIGHT_TAG "Light Log"
#define LED_CNT 24 
#define CONT_STEP 20 

int using_led_cnt = 0;
#define DEFAULT_COLOR PaleGreen
ARGB current_color;
static bool light_state = false;

void light_chage_color(ARGB color, int led_cnt)
{
    ARGB c = {.argb.red=color.argb.blue, .argb.green = color.argb.green, .argb.blue = color.argb.red};   
    change_color(c.code, led_cnt);
    led_strip_update();
}

void limit_val(ARGB* dst, const ARGB_float* src, ARGB limit, bool is_inc)
{
    #define UPPER_LIMIT(x, limit) ((x > limit) ? limit : x ) 
    #define LOWER_LIMIT(x, limit) ((x < limit) ? limit : x ) 

    if(is_inc)
    {
        dst->argb.red = UPPER_LIMIT(src->red, limit.argb.red);
        dst->argb.blue = UPPER_LIMIT(src->blue, limit.argb.blue);
        dst->argb.green =UPPER_LIMIT(src->green, limit.argb.green);
        dst->argb.alpha = UPPER_LIMIT(src->alpha, limit.argb.alpha);
    } else 
    {
        dst->argb.red = LOWER_LIMIT(src->red, limit.argb.red);
        dst->argb.blue = LOWER_LIMIT(src->blue, limit.argb.blue);
        dst->argb.green = LOWER_LIMIT(src->green, limit.argb.green);
        dst->argb.alpha = LOWER_LIMIT(src->alpha, limit.argb.alpha);
    }

}

void light_chage_color_dimming(const int step, const int duration, ARGB from_color, ARGB to_color, bool is_turn_on)
{
    const int DELAY_TIME = duration / step;
    const int MARGIN = step * 20 / 100;
    ARGB_float argb_step = {.red = 0.0, .blue = 0.0, .green = 0, .alpha =0};

    argb_step.red = (to_color.argb.red - from_color.argb.red) / (float)step; 
    argb_step.blue = (to_color.argb.blue - from_color.argb.blue)/ (float)step; 
    argb_step.green = (to_color.argb.green - from_color.argb.green) / (float)step; 
    argb_step.alpha = (to_color.argb.alpha - from_color.argb.alpha)/ (float)step; 

    printf("step - r: %f, g: %f, b: %f\n", argb_step.red, argb_step.green, argb_step.blue);
    ARGB_float argb_accum = {
    .red = (float)from_color.argb.red, 
    .blue = (float)from_color.argb.blue, 
    .green = (float)from_color.argb.green, 
    .alpha = (float)from_color.argb.alpha};

    ARGB cur_color;
    cur_color.code = from_color.code;

    for(int i = 1; i <= step + MARGIN ; i++)
    {
        argb_accum.red += argb_step.red;
        argb_accum.blue += argb_step.blue;
        argb_accum.green += argb_step.green;
        argb_accum.alpha += argb_step.alpha;

        limit_val(&cur_color, &argb_accum, to_color, is_turn_on);

        printf("r: %d, g: %d, b: %d\n", cur_color.argb.red, cur_color.argb.green, cur_color.argb.blue);
        change_color(cur_color.code, LED_CNT);
        led_strip_update();
        esp_rom_delay_us(DELAY_TIME);
    }

    printf("r: %d, g: %d, b: %d\n", to_color.argb.red, to_color.argb.green, to_color.argb.blue);
    change_color(to_color.code, LED_CNT);
    led_strip_update();
}

void light_on()
{
    light_chage_color(read_color_nvs(), LED_CNT);
    light_state = true;
}

void light_off()
{
    ARGB color;
    color.code = Black;

    light_chage_color(color, LED_CNT);
    light_state = false;
}

void light_on_dimming()
{
    ARGB to_color = read_color_nvs();
    ARGB from_color = {.code = 0};

    const int step = 100;
    const int duration_us = 1000000;
    light_chage_color_dimming(step, duration_us, from_color, to_color, true);
    light_state = true;
}

void light_off_dimming()
{
    ARGB from_color = read_color_nvs();
    ARGB to_color = {.code = 0};

    to_color.code = Black;
    const int step = 100;
    const int duration_us = 700000;
    light_chage_color_dimming(step, duration_us, from_color, to_color, false);
    light_state = false;
}

void toggle_light()
{
    if(light_state)
    {
        light_off_dimming();
    }
    else
    {
        light_on_dimming();
    }
}


void darker_light()
{
    int cnt = using_led_cnt - CONT_STEP;

    if(cnt < 0)
        cnt = 0;

    using_led_cnt = cnt;

    ARGB color;
    color.code = White;


    printf("less light : %d\n", using_led_cnt);
    light_chage_color(color, cnt);
}

void brighter_light()
{
    int cnt = using_led_cnt + CONT_STEP;

    if(cnt > LED_CNT - 1 )
        cnt = LED_CNT -1;

    using_led_cnt = cnt;

    printf("more light : %d\n", using_led_cnt);
    ARGB color;
    color.code = White;


    light_chage_color(color, cnt);
}

void light_change_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    light_chage_color(fromRGB(r,g,b), LED_CNT);
}


ARGB read_color_nvs()
{
    ARGB saved_color = {.code =  nvs_read_uint32("saved_color", DEFAULT_COLOR)};
    ESP_LOGI(LIGHT_TAG, "color from nvs %" PRIu32, saved_color.code);   
    return saved_color;
}

void write_color_nvs(uint32_t color)
{
    nvs_write_uint32("saved_color", color);
}

void save_color_nvs(uint8_t r, uint8_t g, uint8_t b)
{
    write_color_nvs((r<<16) | (g<<8) | b);
}

void light_init()
{
    initSPIws2812();
    //todo: for testing
    save_color_nvs(0xD2, 0xFA, 0xFA);
    current_color = read_color_nvs();
    
    light_on_dimming();
    ESP_LOGI(LIGHT_TAG, "change light color %" PRIu32, current_color.code);
}

void get_current_color(uint8_t* color)
{
    color[0] = current_color.argb.alpha;
    color[1] = current_color.argb.red;
    color[2] = current_color.argb.green;
    color[3] = current_color.argb.blue;
}

bool get_light_on_off()
{
    return light_state;
}

ARGB fromRGB(uint8_t r, uint8_t g, uint8_t b)
{
    ARGB color = {.argb.alpha=0, .argb.red=r, .argb.blue=b, .argb.green=g, };

    return color;
}