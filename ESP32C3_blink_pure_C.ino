

// xiaolaba, 2023-FEB-05
// ref: https://c2plabs.com/blog/2021/12/31/esp32-led-blink-tutorial-using-esp-idf-freertos-framework/
// modified and tested
// build enviroment/compiler : Arduino/ESP32C3 compiler, not ESP-IDF enviroment.
// hardware : Luatos CORE-ESP32C3, RMB 12.9, USB-UART chip CH343, or,
//            Luatos CORE-ESP32C3-L, RMB9.9, GPIO_18 GPIO_19 USB-JTAG; no CH343.
// ESP32C3 is RISCV cpu core, not like esp32 (xtensa cpu core), compiler and disassembler are different.
// Sketch uses 216396 bytes (10%) of program storage space. Maximum is 2097152 bytes.
// Global variables use 15520 bytes (4%) of dynamic memory, leaving 312160 bytes for local variables. Maximum is 327680 bytes.


#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


/*
// on broad LED is GPIO 12 & 13
// #define LED_PIN_12 12
// #define LED_PIN_13 13
//// compile time error: invalid conversion from 'int' to 'gpio_num_t' [-fpermissive]
//// solution, https://www.esp32.com/viewtopic.php?t=1591
*/
#define LED_PIN_12 GPIO_NUM_12
#define LED_PIN_13 GPIO_NUM_13


void led_blink(void *pvParams) {
  // config LED GPIO PIN
  gpio_pad_select_gpio(LED_PIN_12);
  gpio_pad_select_gpio(LED_PIN_13);

/*
// gpio_set_direction (LED_PIN_12,GPIO_MODE_OUTPUT); 
// gpio_set_direction (LED_PIN_13,GPIO_MODE_OUTPUT); //error: invalid conversion from 'int' to 'gpio_num_t' [-fpermissive]
//// compile time error: invalid conversion from 'int' to 'gpio_num_t' [-fpermissive]  
//// solution, https://www.esp32.com/viewtopic.php?t=1591
*/
  // set GPIO PIN direction to output mode
  gpio_set_direction((gpio_num_t)LED_PIN_12, GPIO_MODE_OUTPUT);
  gpio_set_direction((gpio_num_t)LED_PIN_13, GPIO_MODE_OUTPUT);

    //forever loop
    while (1) {
        gpio_set_level(LED_PIN_12,0); //LED PIN low
        gpio_set_level(LED_PIN_13,1); //LED PIN high
        vTaskDelay(1000/portTICK_RATE_MS);  // delay 1 sec

        // flip LED PINs
        gpio_set_level(LED_PIN_12,1);
        gpio_set_level(LED_PIN_13,0);
        vTaskDelay(1000/portTICK_RATE_MS); // delay 1 sec
    }
}


//app_main is the first function called by Application startup code in ESP-IDF freeRTOS.
void app_main() {
    xTaskCreate(&led_blink,"LED_BLINK",512,NULL,5,NULL);
}


// Adruino C code and the start up function
void setup(){
  app_main(); // call blinking function
}



/*
  use ardiuino IDE esp32 compiler, this is a must dummy function to add.
  otherwise compile time error as following,
c:/users/user0/appdata/local/arduino15/packages/esp32/tools/riscv32-esp-elf-gcc/gcc8_4_0-esp-2021r2-patch5/bin/../lib/gcc/riscv32-esp-elf/8.4.0/../../../../riscv32-esp-elf/bin/ld.exe: C:\Users\user0\AppData\Local\Temp\arduino_cache_605650\core\core_858b30540e9022259665ac38db8257ac.a(main.cpp.o): in function `loopTask(void*)':
C:\Users\user0\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.6\cores\esp32/main.cpp:45: undefined reference to `loop()'
*/
void loop(){
}
