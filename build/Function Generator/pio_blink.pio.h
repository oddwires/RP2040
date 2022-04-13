// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// --------- //
// pio_blink //
// --------- //

#define pio_blink_wrap_target 2
#define pio_blink_wrap 7

static const uint16_t pio_blink_program_instructions[] = {
    0x80a0, //  0: pull   block                      
    0x6040, //  1: out    y, 32                      
            //     .wrap_target
    0xa022, //  2: mov    x, y                       
    0xe001, //  3: set    pins, 1                    
    0x0044, //  4: jmp    x--, 4                     
    0xa022, //  5: mov    x, y                       
    0xe000, //  6: set    pins, 0                    
    0x0047, //  7: jmp    x--, 7                     
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program pio_blink_program = {
    .instructions = pio_blink_program_instructions,
    .length = 8,
    .origin = -1,
};

static inline pio_sm_config pio_blink_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + pio_blink_wrap_target, offset + pio_blink_wrap);
    return c;
}

// this is a raw helper function for use by the user which sets up the GPIO output, and configures the SM to output on a particular pin
void blink_program_init(PIO pio, uint sm, uint offset, uint pin) {
   pio_gpio_init(pio, pin);
   pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
   pio_sm_config c = pio_blink_program_get_default_config(offset);
   sm_config_set_set_pins(&c, pin, 1);
   pio_sm_init(pio, sm, offset, &c);
}

#endif
