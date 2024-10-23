#include <asf.h>
#include <board.h>
#include <gpio.h>
#include <sysclk.h>
#include "busy_delay.h"

#define CONFIG_USART_IF (AVR32_USART2)

// defines for BRTT interface
#define TEST_A      AVR32_PIN_PA31
#define RESPONSE_A  AVR32_PIN_PA30
#define TEST_B      AVR32_PIN_PA29
#define RESPONSE_B  AVR32_PIN_PA28
#define TEST_C      AVR32_PIN_PA27
#define RESPONSE_C  AVR32_PIN_PB00


// for devbugging
bool debug = true;

void print_debug(const char* message, bool debug)
{
    if (debug)
    {
        printf(message);
    }
}

__attribute__((__interrupt__)) static void interrupt_J3(void) {
    // check wich pin triggered the interrupt and respond accordingly
    if (gpio_get_pin_interrupt_flag(TEST_A))
    {
        gpio_clear_pin_interrupt_flag(TEST_A);
        gpio_set_pin_low(RESPONSE_A);
        print_debug("RESPONSE_A set to low\n", debug);
        gpio_set_pin_high(RESPONSE_A);
        print_debug("RESPONSE_A set to high\n", debug);
    }

    if (gpio_get_pin_interrupt_flag(TEST_B))
    {
        gpio_clear_pin_interrupt_flag(TEST_B);
        gpio_set_pin_low(RESPONSE_B);
        print_debug("RESPONSE_B set to low\n", debug);
        gpio_set_pin_high(RESPONSE_B);
        print_debug("RESPONSE_B set to high\n", debug);
    }

    if (gpio_get_pin_interrupt_flag(TEST_C))
    {
        gpio_clear_pin_interrupt_flag(TEST_C);
        gpio_set_pin_low(RESPONSE_C);
        print_debug("RESPONSE_C set to low\n", debug);
        gpio_set_pin_high(RESPONSE_C);
        print_debug("RESPONSE_C set to high\n", debug);
}


void init(){
    sysclk_init();
    board_init();
    busy_delay_init(BOARD_OSC0_HZ);
    
    cpu_irq_disable();
    INTC_init_interrupts();
    INTC_register_interrupt(&interrupt_J3, AVR32_GPIO_IRQ_3, AVR32_INTC_INT1);

    // enable interrupts for the TEST pins
    gpio_enable_pin_interrupt(TEST_A);
    gpio_enable_pin_interrupt(TEST_B);
    gpio_enable_pin_interrupt(TEST_C);
    
    cpu_irq_enable();
    
    stdio_usb_init(&CONFIG_USART_IF);

    #if defined(__GNUC__) && defined(__AVR32__)
        setbuf(stdout, NULL);
        setbuf(stdin,  NULL);
    #endif

    // init GPIO pins
    gpio_configure_pin(TEST_A, GPIO_DIR_INPUT);
    gpio_configure_pin(RESPONSE_A, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
    gpio_configure_pin(TEST_B, GPIO_DIR_INPUT);
    gpio_configure_pin(RESPONSE_B, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
    gpio_configure_pin(TEST_C, GPIO_DIR_INPUT);
    gpio_configure_pin(RESPONSE_C, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
}



int main (void){
    init();

    while (1){
        gpio_toggle_pin(LED0_GPIO);
        print_debug("tick\n", debug);
        busy_delay_us(500);
    }

}
