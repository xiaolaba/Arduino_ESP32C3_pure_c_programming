// original code, https://developer.espressif.com/blog/2025/10/cores_with_fpu/


#include <stdio.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <esp_cpu.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_random.h>
#include <esp_log.h>

#define TAG         "test"
#define ITERATIONS  10000

static void float_test(const char* label, float seed, float (*function) (float))
{
    uint32_t start_cycles = esp_cpu_get_cycle_count();
    for (uint32_t i = 0; i < ITERATIONS; i++)
    {
        seed = function(seed);
    }
    uint32_t end_cycles = esp_cpu_get_cycle_count();

    printf(TAG ": %s %-10.10s average %"PRIu32 " cycles\n",label,  "float:", (end_cycles - start_cycles) / ITERATIONS);
}

static void double_test(const char* label, double seed, double (*function) (double))
{
    uint32_t start_cycles = esp_cpu_get_cycle_count();
    for (uint32_t i = 0; i < ITERATIONS; i++)
    {
        seed = function(seed);
    }
    uint32_t end_cycles = esp_cpu_get_cycle_count();

    printf(TAG ": %s %-10.10s average %"PRIu32 " cycles\n", label, "double:", (end_cycles - start_cycles) / ITERATIONS);
}

typedef struct {
    float (*float_function) (float);
    double (*double_function) (double);
} test_t;

static void test(const char* label, test_t* test) {
    printf(TAG ": %s\n", label);

    double seed = 123456.789;

    if (test->float_function != NULL) {
        float_test(label, (float) seed, test->float_function);
    }

    if (test->double_function != NULL) {
        double_test(label, (double) seed, test->double_function);
    }

    printf("\n");
}

/*** trivial functions ***/
static float float_sum10(float value) {
    return (value + 10);
}

static double double_sum10(double value) {
    return (value + 10);
}

static float float_div10(float value) {
    return (value / 10);
}

static double double_div10(double value) {
    return (value / 10);
}

/*
 * mixed calculations
 */
static float float_mixed(float value) {
    return cosf(sqrtf(value / 2.3f * 0.5f / value));
}

static double double_mixed(double value) {
    return cos(sqrt(value / 2.3 * 0.5 / value));
}

void app_main(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    const char *model;
    switch (chip_info.model) {
        case CHIP_ESP32:
            model = "ESP32";
            break;
        case CHIP_ESP32S2:
            model = "ESP32-S2";
            break;
        case CHIP_ESP32S3:
            model = "ESP32-S3";
            break;
        case CHIP_ESP32C3:
            model = "ESP32-C3";
            break;
        case CHIP_ESP32H2:
            model = "ESP32-H2";
            break;
        case CHIP_ESP32C2:
            model = "ESP32-C2";
            break;
        case CHIP_ESP32C6:
            model = "ESP32-C6";
            break;
        default:
            model = "<UNKNOWN>";
            break;
    }
    printf("\n");
    printf(TAG ": CHIP: %s\n", model);
    printf("\n");

    test_t sum10 = {
        .float_function =   float_sum10,
        .double_function =  double_sum10,
    };
    test("SUM", &sum10);

    test_t div10 = {
        .float_function =   float_div10,
        .double_function =  double_div10,
    };
    test("DIV", &div10);

    test_t cosine = {
        .float_function =   cosf,
        .double_function =  cos,
    };
    test("COS", &cosine);

    test_t mixed = {
        .float_function =   float_mixed,
        .double_function =  double_mixed,
    };
    test("MIX", &mixed);
}