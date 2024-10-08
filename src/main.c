#include "nvs_storage.h"
#include "sleep_light.h"
#include "spi_ws2812.h"
#include "ble_server.h"
#include "gpio.h"

#include "esp_log.h"
#include "esp_system.h"

#include "led_event_loop.h"

void app_main() {
    printf("## nvs init ##\n");
	nvs_init();

    printf("## gpio init ##\n");
	gpio_init();

    printf("## ble init ##\n");
	ble_start();

    printf("## light init ##\n");
	light_init();

    printf("## reg_led_event init ##\n");
	reg_led_event();
}
