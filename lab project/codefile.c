#include <LPC17xx.h>
#include <stdio.h>
#include <string.h>

#define RS_CTRL (1 << 8)
#define EN_CTRL (1 << 9)
#define DT_CTRL (0xF << 4)
#define BUZZER_PIN (1 << 17)

#define refVtg 5.0
#define digitalMax 4095.0
#define AQI_THRESHOLD 100

// LCD Messages
char msg[]  = "CO PPM: ";
char msg2[] = "AQI: ";
char msg3[] = " ";
char msg4[10];

// LCD Initialization Command Sequence
unsigned long int init_command[] = {0x30, 0x30, 0x30, 0x20, 0x28, 0x0C, 0x06, 0x01, 0x80};

// Global variables
unsigned int temp1, temp2, i, flag1, flag2;

// Function Prototypes
void lcd_init(void);
void lcd_write(void);
void port_write(void);
void delay(unsigned int);
void lcd_print_msg(void);
void lcd_print_msg2(void);
void lcd_print_msg4(void);
void buzzer_dash(void);
int map_adc_to_ppm(int adc_val);

// Main Function
int main(void) {
    unsigned int mqReading;
    float analogVtg;
    int ppm, aqi;
    char ppmStr[16], aqiStr[16];

    SystemInit();
    SystemCoreClockUpdate();

    // ADC Initialization
    LPC_PINCON->PINSEL1 &= ~(3 << 14);  // P0.23 -> AD0.0
    LPC_PINCON->PINSEL1 |= (1 << 14);
    LPC_SC->PCONP |= (1 << 12);         // Power up ADC
    LPC_ADC->ADCR = (1 << 0) | (4 << 8) | (1 << 21); // Select AD0.0, CLKDIV=4, ADC enable

    // GPIO Initialization for LCD and Buzzer
    LPC_GPIO0->FIODIR |= RS_CTRL | EN_CTRL | DT_CTRL | BUZZER_PIN;

    lcd_init();
    lcd_print_msg();
    lcd_print_msg2();

    while (1) {
        // Start ADC conversion
        LPC_ADC->ADCR |= (1 << 24);
        while (!(LPC_ADC->ADGDR & (1 << 31)));  // Wait for conversion to complete

        mqReading = (LPC_ADC->ADGDR >> 4) & 0xFFF;
        analogVtg = ((float)mqReading * refVtg) / digitalMax;

        ppm = map_adc_to_ppm(mqReading);
        aqi = ppm / 2; // Simplified logic for AQI calculation

        // Convert integer values to strings
        sprintf(ppmStr, "%dPPM ", ppm);
        sprintf(aqiStr, "%d", aqi);

        // Display PPM value at position 0x87
        temp1 = 0x87; flag1 = 0; lcd_write();
        delay(500); flag1 = 1;
        i = 0;
        while (ppmStr[i] != '\0') {
            temp1 = ppmStr[i++];
            lcd_write();
        }

        // Display AQI at position 0xC5
        temp1 = 0xC5; flag1 = 0; lcd_write();
        delay(500); flag1 = 1;
        i = 0;
        while (aqiStr[i] != '\0') {
            temp1 = aqiStr[i++];
            lcd_write();
        }

        // AQI Condition Messages and Buzzer Control
        if (aqi >= AQI_THRESHOLD) {
            LPC_GPIO0->FIOSET = BUZZER_PIN;
            strcpy(msg4, "DANGER");
            lcd_print_msg4();
            buzzer_dash();
        } else if (aqi >= 50) {
            strcpy(msg4, "BAD");
            lcd_print_msg4();
            LPC_GPIO0->FIOCLR = BUZZER_PIN;
        } else {
            strcpy(msg4, "GOOD");
            lcd_print_msg4();
            LPC_GPIO0->FIOCLR = BUZZER_PIN;
        }

        delay(1000000);
    }
}

// Map ADC value to approximate PPM value
int map_adc_to_ppm(int adc_val) {
    return (adc_val * 1000) / 4095; // Scale to 0–1000 PPM
}

// Initialize LCD
void lcd_init(void) {
    flag1 = 0;
    for (i = 0; i < 9; i++) {
        temp1 = init_command[i];
        lcd_write();
        delay(30000);
    }
    flag1 = 1;
}

// Write to LCD (4-bit mode)
void lcd_write(void) {
    flag2 = (flag1 == 1) ? 0 : (((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0);
    temp2 = temp1 >> 4;
    port_write();
    if (!flag2) {
        temp2 = temp1 & 0x0F;
        port_write();
    }
}

// Send nibble to LCD
void port_write(void) {
    LPC_GPIO0->FIOCLR = DT_CTRL;
    LPC_GPIO0->FIOSET = (temp2 << 4) & DT_CTRL;

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL; // Command
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // Data

    LPC_GPIO0->FIOSET = EN_CTRL;
    delay(50);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay(1000);
}

// Display "CO PPM:" label
void lcd_print_msg(void) {
    flag1 = 1;
    for (i = 0; msg[i] != '\0'; i++) {
        temp1 = msg[i];
        lcd_write();
    }
}

// Display "AQI:" label
void lcd_print_msg2(void) {
    temp1 = 0xC0; // Move cursor to second line
    flag1 = 0; lcd_write(); delay(800);
    flag1 = 1;
    for (i = 0; msg2[i] != '\0'; i++) {
        temp1 = msg2[i];
        lcd_write();
    }
}

// Display Air Quality Status message
void lcd_print_msg4(void) {
    temp1 = 0xCA; // Move cursor to position for status
    flag1 = 0; lcd_write(); delay(800);
    flag1 = 1;
    for (i = 0; msg4[i] != '\0'; i++) {
        temp1 = msg4[i];
        lcd_write();
    }
}

// Simple delay function
void delay(unsigned int r1) {
    volatile unsigned int r;
    for (r = 0; r < r1; r++);
}

// Buzzer alert pulse
void buzzer_dash(void) {
    LPC_GPIO0->FIOSET = BUZZER_PIN;
    delay(500000);
    LPC_GPIO0->FIOCLR = BUZZER_PIN;
    delay(500000);
}
