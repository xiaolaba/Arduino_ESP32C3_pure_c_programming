// xiao_laba_cn@yahoo.com, 2026-SEP-06
// orignal code snippet, https://github.com/citycafe578/FixedPID/blob/main/src/FixedPID.cpp
// int64_t FixedPID::filterDerivative(int64_t rawD){}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_cpu.h"
#include "esp_log.h"

//#include <unistd.h> // Included for sleep() to slow down execution

static const char *TAG = "PID_BENCHMARK";

//ESP32 C3, RISC-V and FPU available
//#define PID_SCALE 0x3FF // 0~1023
#define PID_SHIFT 10



// 全局變數（加上 volatile 避免編譯器將重複運算直接優化優掉）
volatile int64_t alpha = 1024;
volatile int64_t rawD_test = 1024;
volatile int64_t prevD_1 = 500;
volatile int64_t prevD_2 = 500;


volatile uint16_t PID_SCALE = 0;


// read about, https://developer.espressif.com/blog/2025/10/cores_with_fpu/
// 1. 原始版本（64位元除法 + 2次乘法, FPU used ?? ESP32-C3 no FPU
__attribute__((noinline)) int64_t filterDerivative_Original(int64_t rawD) {
    if (alpha == PID_SCALE) {
        return rawD;
    }
    int64_t filteredD =
        ((int64_t)alpha * rawD +
         (PID_SCALE - (int64_t)alpha) * prevD_1) / PID_SCALE;

    prevD_1 = filteredD;
    return filteredD;
}

// 2. 優化版本（64位元位移 + 1次乘法）
__attribute__((noinline)) int64_t filterDerivative_Optimized(int64_t rawD) {
    if (alpha == PID_SCALE) {
        return rawD;
    }
    int64_t filteredD = prevD_2 + (((int64_t)alpha * (rawD - prevD_2)) >> PID_SHIFT);

    prevD_2 = filteredD;
    return filteredD;
}


/*
ESP32-C3 CPU speed 160MHZ

esp_cpu_get_cycle_count()：ESP-IDF 提供讀取 CPU CCOUNT 暫存器的 API，
精度達到單一 Clock Cycle（在 240MHz ??? 時鐘下，1 cycle = 4.16 ns）。

portDISABLE_INTERRUPTS()：在測試區塊前後關閉與開啟中斷，確保測量期間不會
因為 FreeRTOS 的 Tick 中斷或任務調度而產生計數偏差。

__attribute__((noinline)) 與 volatile：防止編譯器將函式直接 Inline 展開，或將迴圈內的重複
計算在編譯期進行常量折疊（Constant Folding）優化，從而確保測量到的是真實的指令執行週期。

*/

void app_main(void) {
  
    // Built-in USB CDC Terminal (Native USB vs UART)
    // Arduino IDE,
    // Go to Tools -> Core Debug Level -> Info or Verbose
    // Go to Tools -> USB CDC On Boot -> Enabled.
    
    
    // Force the log level for this tag to INFO (or VERBOSE)
    esp_log_level_set(TAG, ESP_LOG_INFO);
  
  while(1){
      switch (PID_SCALE) {
              case 0:
                  // Handles case when PID_SCALE is initially 0
                  PID_SCALE = 1000;
                  break;     
              case 1000:
                  PID_SCALE = 0x3FF; // 1023 in decimal
                  break;
              case 1023:
                  PID_SCALE = 1024; // 1023 in decimal
                  break;
              case 1024:
                  PID_SCALE = 0;
                  break;                      
              default:
                  PID_SCALE = 0;
                  break;
          }
    
    //vTaskDelay(pdMS_TO_TICKS(1000)); // 等待系統日誌穩定

    vTaskDelay(pdMS_TO_TICKS(10000)); // and delay to easy naked eyes to visual terminal log

    const uint16_t iterations = 1000;
    uint32_t start_cycles, end_cycles;
    uint32_t cycles_orig = 0, cycles_opt = 0;

    // Standard C library print
    printf("Hello from ESP32-C3-super-mini. Built-in USB CDC Terminal via printf()!\n\n");

    // ESP-IDF Logging System (Recommended)
    ESP_LOGI(TAG, "System started successfully, by ESP_LOGI(TAG)");
    
    ESP_LOGI(TAG, "開始執行 PID 濾波器基準測試 (%d 次迭代)..., ESP_LOGI(TAG)", iterations);

    // --- 1. 測試原始版本 ---
    portDISABLE_INTERRUPTS(); // 關閉中斷以防 context switch 影響計數
    start_cycles = esp_cpu_get_cycle_count();
    for (int i = 0; i < iterations; i++) {
        filterDerivative_Original(rawD_test + i);
    }
    end_cycles = esp_cpu_get_cycle_count();
    portENABLE_INTERRUPTS();
    cycles_orig = end_cycles - start_cycles;

    // --- 2. 測試優化版本 ---
    portDISABLE_INTERRUPTS();
    start_cycles = esp_cpu_get_cycle_count();
    for (int i = 0; i < iterations; i++) {
        filterDerivative_Optimized(rawD_test + i);
    }
    end_cycles = esp_cpu_get_cycle_count();
    portENABLE_INTERRUPTS();
    cycles_opt = end_cycles - start_cycles;

    // --- 計算結果 ---
    float avg_orig = (float)cycles_orig / iterations;
    float avg_opt = (float)cycles_opt / iterations;
    float speedup = ((avg_orig - avg_opt) / avg_orig) * 100.0f;
    float ratio = avg_orig / avg_opt;

    printf("\n============ 基準測試結果 (Benchmark Result) ============\n");
    printf("PID_SCALE = %d\n", PID_SCALE);
    printf("原始版本 (除法)   - 總週期: %" PRIu32 " Cycles | 單次平均: %.2f Cycles\n", cycles_orig, avg_orig);
    printf("優化版本 (位移)   - 總週期: %" PRIu32 " Cycles | 單次平均: %.2f Cycles\n", cycles_opt, avg_opt);
    printf("-------------------------------------------------------\n");
    printf("速度提升比例: %.2f 倍 (約相當於節省了 %.2f%% 的 CPU 週期)\n", ratio, speedup);
    printf("=======================================================\n\n");

  }

}









/*
  use ardiuino IDE esp32 compiler, this is a must of two dummy functions to be included.
  otherwise compile time error as following,
C:/Users/user0/AppData/Local/Arduino15/packages/esp32/tools/esp-rv32/2507/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld.exe: C:\Users\user0\AppData\Local\Temp\arduino_build_280741\core\core.a(main.cpp.o): in function `loopTask(void*)':
C:\Users\user0\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.2\cores\esp32/main.cpp:58:(.text._Z8loopTaskPv+0x26): undefined reference to `setup()'
C:/Users/user0/AppData/Local/Arduino15/packages/esp32/tools/esp-rv32/2507/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld.exe: C:\Users\user0\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.2\cores\esp32/main.cpp:71:(.text._Z8loopTaskPv+0x4a): undefined reference to `loop()'
collect2.exe: error: ld returned 1 exit status
exit status 1
Error compiling for board ESP32C3 Dev Module.

*/

// Arduino C code and the start up function
void setup(){
  app_main(); // call blinking function
}
void loop(){
}
