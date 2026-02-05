#include "led.h"
#include "esp_log.h"

namespace led {

static const char* TAG = "LED";
static gpio_num_t s_led_gpio = GPIO_NUM_NC;
static bool s_led_state = false;

void init(gpio_num_t gpio_num)
{
    s_led_gpio = gpio_num;
    
    gpio_reset_pin(s_led_gpio);
    gpio_set_direction(s_led_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(s_led_gpio, 1);  // Active LOW - LED éteinte au démarrage
    
    s_led_state = false;
    ESP_LOGI(TAG, "LED initialisée sur GPIO%d", s_led_gpio);
}

void on()
{
    if (s_led_gpio == GPIO_NUM_NC) return;
    
    gpio_set_level(s_led_gpio, 0);  // Active LOW
    s_led_state = true;
    ESP_LOGI(TAG, "LED ON");
}

void off()
{
    if (s_led_gpio == GPIO_NUM_NC) return;
    
    gpio_set_level(s_led_gpio, 1);  // Active LOW
    s_led_state = false;
    ESP_LOGI(TAG, "LED OFF");
}

void toggle()
{
    if (s_led_state) {
        off();
    } else {
        on();
    }
}

bool get_state()
{
    return s_led_state;
}

void set_state(bool state)
{
    if (state) {
        on();
    } else {
        off();
    }
}

} // namespace led
