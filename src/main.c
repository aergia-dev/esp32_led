
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include "servo/servo.h"

#include "nvs_storage.h"
#include "sleep_light.h"
#include "spi_ws2812.h"
#include "ble_server.h"
#include "gpio.h"

#include "esp_log.h"
#include "esp_system.h"

#include "led_event_loop.h"

void app_main() {
	nvs_init();
	gpio_init();
	ble_start();
	light_init();
	initSPIws2812();

	reg_led_event();
}


// extern "C" void app_main(){
// 	servoControl myServo;
// 	myServo.attach(GPIO_NUM_15);
// 	//Defaults: myServo.attach(pin, 400, 2600, LEDC_CHANNEL_0, LEDC_TIMER0);
// 	// to use more servo set a valid ledc channel and timer
	
// 	myServo.write(0);
// 	vTaskDelay(1000 / portTICK_RATE_MS);
	
// 	while (1){
// 		for (int i = 0; i<180; i++){
// 			myServo.write(i);
// 			vTaskDelay(10 / portTICK_RATE_MS);
// 		}
// 		for (int i = 180; i>0; i--){
// 			myServo.write(i);
// 			vTaskDelay(60 / portTICK_RATE_MS);
// 		}
// 	}
// }