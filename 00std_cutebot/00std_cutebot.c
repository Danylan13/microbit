#include "nrf.h"

/*
sources:
    https://github.com/elecfreaks/circuitpython_cutebot/blob/main/cutebot.py
    https://github.com/Krakenus/microbit-cutebot-micropython/blob/master/cutebot.py
    https://github.com/bbcmicrobit/micropython/blob/master/source/microbit/microbiti2c.cpp
    https://microbit-micropython.readthedocs.io/en/latest/i2c.html#
    https://makecode.microbit.org/device/pins

I2C:
    config:
        7-bit addressing
    pins:
        freq: 100000
        micro:bit V2: P19=SCL=P0.26, P20=SDA=P1.00
        micro:bit V1: P19=SCL=P0.00, P20=SDA=P0.30
*/

#if defined(NRF51822_XXAA)
#define MICROBIT_I2C_SCL_PIN_CNF_INDEX 0u
#define MICROBIT_I2C_SDA_PIN_CNF_INDEX 30u
#define MICROBIT_I2C_SCL_PSEL          0x00000000u
#define MICROBIT_I2C_SDA_PSEL          0x0000001Eu
#define SIDE_DELAY_CYCLES              1500000u
#define TURN_DELAY_CYCLES               600000u
#else
#define MICROBIT_I2C_SCL_PIN_CNF_INDEX 26u
#define MICROBIT_I2C_SDA_PIN_CNF_INDEX 0u
#define MICROBIT_I2C_SCL_PSEL          0x0000001Au
#define MICROBIT_I2C_SDA_PSEL          0x00000020u
#define SIDE_DELAY_CYCLES              6000000u
#define TURN_DELAY_CYCLES              2400000u
#endif

/*
motor control
[
    motor,     // 0x01: left,    0x02: right
    direction, // 0x02: forward, 0x01: backward
    speed,     // between 0 and 100
    0,
]
*/
#define DRIVE_SPEED          60u  // [0...100]
#define TURN_SPEED           55u  // [0...100]

/*
LED control
[
    led,       // 0x04: left,    0x08: right
    r,         // 0..255?
    g,         // 0..255?
    b,         // 0..255?
]
*/
#define LED_INTENSITY 0xff // [0x00...0xff]
uint8_t I2CBUF_LED_LEFT_GREEN[]   = {0x04,         0x00,LED_INTENSITY,         0x00};
uint8_t I2CBUF_LED_LEFT_OFF[]     = {0x04,         0x00,         0x00,         0x00};
uint8_t I2CBUF_LED_RIGHT_GREEN[]  = {0x08,         0x00,LED_INTENSITY,         0x00};
uint8_t I2CBUF_LED_RIGHT_OFF[]    = {0x08,         0x00,         0x00,         0x00};

void i2c_init(void) {
   //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:   0=Input
    // .... .... .... .... .... .... .... ..B. B: INPUT: 1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:  0=Disabled
    // .... .... .... .... .... .DDD .... .... D: DRIVE: 6=S0D1
    // .... .... .... ..EE .... .... .... .... E: SENSE: 0=Disabled
    // xxxx xxxx xxxx xx00 xxxx x110 xxxx 0010 
    //    0    0    0    0    0    6    0    2 0x00000602
#if defined(NRF51822_XXAA)
    NRF_GPIO->PIN_CNF[MICROBIT_I2C_SCL_PIN_CNF_INDEX] = 0x00000602;
    NRF_GPIO->PIN_CNF[MICROBIT_I2C_SDA_PIN_CNF_INDEX] = 0x00000602;
#else
    NRF_P0->PIN_CNF[MICROBIT_I2C_SCL_PIN_CNF_INDEX] = 0x00000602;
    NRF_P1->PIN_CNF[MICROBIT_I2C_SDA_PIN_CNF_INDEX] = 0x00000602;
#endif

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE              = 0x00000005;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    26 (P0.26)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.26)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx01 1010 
    //    0    0    0    0    0    0    1    a 0x0000001a
    #if defined(NRF51822_XXAA)
    NRF_TWI0->PSELSCL             = MICROBIT_I2C_SCL_PSEL;
    #else
    NRF_TWI0->PSEL.SCL            = MICROBIT_I2C_SCL_PSEL;
    #endif

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    00 (P1.00)
    // .... .... .... .... .... .... ..B. .... B: PORT:    1 (P1.00)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx10 0000 
    //    0    0    0    0    0    0    2    0 0x00000020
    #if defined(NRF51822_XXAA)
    NRF_TWI0->PSELSDA             = MICROBIT_I2C_SDA_PSEL;
    #else
    NRF_TWI0->PSEL.SDA            = MICROBIT_I2C_SDA_PSEL;
    #endif

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // AAAA AAAA AAAA AAAA AAAA AAAA AAAA AAAA A: FREQUENCY: 0x01980000==K100==100 kbps
    NRF_TWI0->FREQUENCY           = 0x01980000;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .AAA AAAA A: ADDRESS: 16
    // xxxx xxxx xxxx xxxx xxxx xxxx x001 0000 
    //    0    0    0    0    0    0    1    0 0x00000010
    NRF_TWI0->ADDRESS             = 0x10;
}

void i2c_send(uint8_t* buf, uint8_t buflen) {
    uint8_t i;

    NRF_TWI0->EVENTS_STOPPED = 0;
    i=0;
    NRF_TWI0->TXD                 = buf[i];
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    NRF_TWI0->TASKS_STARTTX       = 1;
    i++;
    while(i<buflen) {
        while(NRF_TWI0->EVENTS_TXDSENT==0);
        NRF_TWI0->EVENTS_TXDSENT  = 0;
        NRF_TWI0->TXD             = buf[i];
        i++;
    }
    while(NRF_TWI0->EVENTS_TXDSENT==0);
    NRF_TWI0->TASKS_STOP          = 1;
    while(NRF_TWI0->EVENTS_STOPPED==0);
    NRF_TWI0->EVENTS_STOPPED      = 0;
}

static void delay_cycles(volatile uint32_t cycles) {
    while(cycles--) {
        __asm volatile ("nop");
    }
}

static void set_motor(uint8_t motor, uint8_t forward, uint8_t speed) {
    uint8_t buf[4];

    buf[0] = motor;
    buf[1] = forward ? 0x02 : 0x01;
    buf[2] = speed;
    buf[3] = 0x00;

    i2c_send(buf, sizeof(buf));
}

static void set_motors_forward(uint8_t speed) {
    set_motor(0x01, 1, speed);
    set_motor(0x02, 1, speed);
}

static void set_motors_turn_right(uint8_t speed) {
    set_motor(0x01, 1, speed);
    set_motor(0x02, 0, speed);
}

static void stop_motors(void) {
    set_motor(0x01, 1, 0);
    set_motor(0x02, 1, 0);
}

static void set_headlights_on(void) {
    i2c_send(I2CBUF_LED_LEFT_GREEN, sizeof(I2CBUF_LED_LEFT_GREEN));
    i2c_send(I2CBUF_LED_RIGHT_GREEN, sizeof(I2CBUF_LED_RIGHT_GREEN));
}

static void set_headlights_off(void) {
    i2c_send(I2CBUF_LED_LEFT_OFF, sizeof(I2CBUF_LED_LEFT_OFF));
    i2c_send(I2CBUF_LED_RIGHT_OFF, sizeof(I2CBUF_LED_RIGHT_OFF));
}

static void drive_square(void) {
    uint8_t side;

    for(side = 0; side < 4; side++) {
        set_motors_forward(DRIVE_SPEED);
        delay_cycles(SIDE_DELAY_CYCLES);

        set_motors_turn_right(TURN_SPEED);
        delay_cycles(TURN_DELAY_CYCLES);
    }

    stop_motors();
}

int main(void) {
    
    i2c_init();

    set_headlights_on();
    delay_cycles(1200000u);

    drive_square();

    set_headlights_off();

    while(1) {
    }
}
