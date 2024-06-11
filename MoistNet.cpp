#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "tusb.h"

#include <map>
#include <string>

#include "pico_http_client.h"

const uint MOIST_AOUT_PIN = 0;
const uint MOIST_POWER_PIN = 1;

static const float conversion_factor = 3.3f / (1 << 12);

void send_http_request(const char *url, const std::map <std::string, std::string> headers) {
    http_client_t *client = new_http_client(url);
    for (auto [key, value]: headers) {
        add_header(client, key.c_str(), value.c_str());
    }
    printf("Sending HTTP Request\n");
    http_response_t response = http_request(HTTPMethod::GET, client);
    printf("response code: %d\n", response.code);
    printf("response body: %s\n", response.body);
    free_http_client(client);
    free(response.body);
}

int main()
{
    stdio_init_all();

   while (!tud_cdc_connected()) {
      printf(".");
      sleep_ms(500);
    }
    printf("usb host detected!\n");

    // Configure ADC for Moisture Probe
    adc_init();
    adc_select_input(MOIST_AOUT_PIN);

    // Configure Power Control for Moisture Probe
    gpio_init(MOIST_POWER_PIN);
    gpio_set_dir(MOIST_POWER_PIN, GPIO_OUT);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    const char *ssid = "";
    const char *password = "";

    cyw43_arch_enable_sta_mode();
    printf("Station Mode enabled\n");

    while (cyw43_arch_wifi_connect_blocking(ssid, password, CYW43_AUTH_WPA2_MIXED_PSK)) {
        printf("Wi-Fi connect failed\n");
        sleep_ms(1000);
    }

    std::map <std::string, std::string> headers{
            {"Content-Type", "application/json"},
    };

    send_http_request("http://worldtimeapi.org/api/timezone/Europe/Paris", headers);

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        gpio_put(MOIST_POWER_PIN, 1);
        printf("MOIST_POWER_PIN ON\n");
        sleep_ms(250);

        for(int elapsed=0; elapsed<2; ++elapsed)
        {
            uint32_t result = adc_read();
            printf("0x%03x -> %f V\n", result, result * conversion_factor);
            sleep_ms(250);
        }

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        gpio_put(MOIST_POWER_PIN, 0);
        printf("MOIST_POWER_PIN OFF\n");
        sleep_ms(250);
        for(int elapsed=0; elapsed<10; ++elapsed)
        {
            uint32_t result = adc_read();
            printf("0x%03x -> %f V\n", result, result * conversion_factor);
            sleep_ms(250);
        }
    }

}