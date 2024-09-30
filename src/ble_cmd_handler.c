#include "protocol.h"
#include "sleep_light.h"
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "ble_cmd_handler.h"
#include <string.h>

#define CMD_HANDLER_TAG "CMD_HANDLER"
#define rsp_buffer_sz 10
uint8_t rsp_buffer[rsp_buffer_sz];
uint8_t rsp_sz = 0;

void cmd_handler(uint16_t main, uint8_t len, uint8_t * sub)
{
   ESP_LOGI(CMD_HANDLER_TAG, "main: %d, len: %d, subs: %d, %d, %d", main, len, sub[0], sub[1], sub[2]);

    switch(main)
    {
        case LED_ON:
            light_on();
            break;
        case LED_OFF:
            light_off();
            break;
        case CHANGE_COLOR:
        //  light_change_rgb(sub[0], sub[1], sub[2]);
         break;
        case SAVE_COLOR:
         save_color_nvs(sub[0], sub[1], sub[2]);
         ESP_LOGI(CMD_HANDLER_TAG, "color saved into flash: %" PRIu32, read_color_nvs().code);

        break;
        case RESET:
          esp_restart();
        break;
        case READ_COLOR:
        {
          memset(rsp_buffer, 0, sizeof(uint8_t) * rsp_buffer_sz);
          get_current_color(rsp_buffer);
          rsp_sz = 4;
          break;
        }
        case READ_LIGHTONOFF:
        {
          memset(rsp_buffer, 0, sizeof(uint8_t) * rsp_buffer_sz);
          rsp_buffer[0] = get_light_on_off();
          rsp_sz = 1;
          break;
        }
        case BRIGHTNESS:
          // control_brightness(sub[0]);
          break;

        default:
            ESP_LOGI(CMD_HANDLER_TAG, "not yet.. main: %d, len: %d, subs: %d, %d, %d", main, len, sub[0], sub[1], sub[2]);

    }

}

uint8_t* get_rsp_buffer()
{
    return rsp_buffer;
}

uint8_t get_rsp_sz()
{
    return rsp_sz;
}