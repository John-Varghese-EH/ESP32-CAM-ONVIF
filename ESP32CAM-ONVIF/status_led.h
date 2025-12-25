// Status LED Control
void status_led_init();
void status_led_loop();
void status_led_wifi_connecting();
void status_led_connected();
void status_led_error();
void status_led_off();
void status_led_flash(int count, int delayMs = 200); // New helper

void status_led_init();
void status_led_loop();

// Patterns
void status_led_wifi_connecting();
void status_led_connected();
void status_led_error();
void status_led_off();
