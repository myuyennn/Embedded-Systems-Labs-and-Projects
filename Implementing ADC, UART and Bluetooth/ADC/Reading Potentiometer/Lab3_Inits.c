/**
 * EE/CSE 474: Lab3 drivers starter code Lab 3 task 1a
 */

#include "PLL_Header.h"
#include "Lab3_Inits.h"

// STEP 0a: Include your header file here
#include <stdint.h> 
#include "lab3.h"

int PLL_Init(enum frequency freq) {
    // Do NOT modify this function.
    MOSCCTL &= ~(0x4);                      // Power up MOSC
    MOSCCTL &= ~(0x8);                      // Enable MOSC
    while ((RIS & 0x100) == 0) {};          // Wait for MOSC to be ready
    RSCLKCFG |= (0x3 << 20);                // Select MOSC as system clock source
    RSCLKCFG |= (0x3 << 24);                // Select MOSC as PLL clock source

    PLLFREQ0 |= 0x60;                       // Set MINT field
    PLLFREQ1 |= 0x4;                        // Set N field

    MEMTIM0 &= ~((0xF << 22) | (0xF << 6));     // Reset FBCHT and EBCHT
    MEMTIM0 &= ~((0xF << 16) | (0xF << 0));     // Reset EWS and FWS
    MEMTIM0 &= ~((0x1 << 21) | (0x1 << 5));     // Reset FBCE and EBCE

    RSCLKCFG &= ~(0x1 << 28);                   // Temporarilly bypass PLL

    switch (freq) {
        case 120:
            MEMTIM0 |= (0x6 << 22) | (0x6 << 6);    // Set FBCHT and EBCHT
            MEMTIM0 |= (0x5 << 16) | (0x5 << 0);    // Set EWS and FWS
            RSCLKCFG |= 0x3;                        // Set PSYSDIV to use 120 MHZ clock
            RSCLKCFG &= ~0x3FC;                     // Update PSYSDIV field
            break;
        case 60:
            MEMTIM0 |= (0x3 << 22) | (0x3 << 6);    // Set FBCHT and EBCHT
            MEMTIM0 |= (0x2 << 16) | (0x2 << 0);    // Set EWS and FWS
            RSCLKCFG |= 0x7;                        // Set PSYSDIV to use 60 MHZ clock
            RSCLKCFG &= ~0x3F8;                     // Update PSYSDIV field
            break;
        case 12:
            MEMTIM0 |= (0x1 << 21) | (0x1 << 5);    // Set FBCE and EBCE
            RSCLKCFG |= 0x27;                       // Set PSYSDIV to use 12 MHZ clock
            RSCLKCFG &= ~0x3D8;                     // Update PSYSDIV field
            break;
        default:
            return -1;
    }

    RSCLKCFG |= (0x1 << 30);                // Enable new PLL settings
    PLLFREQ0 |= (0x1 << 23);                // Power up PLL
    while ((PLLSTAT & 0x1) == 0) {};        // Wait for PLL to lock and stabilize

    RSCLKCFG |= (0x1u << 31) | (0x1 << 28);  // Use PLL and update Memory Timing Register
    return 1;
}

void LED_Init(void) {
  // STEP 1: Initialize the 4 on board LEDs by initializing the corresponding
  // GPIO pins.
  volatile unsigned short delay = 0;
  RCGCGPIO |= 0x1020; // Enable PortF and PortN GPIO
  delay++; // Delay 2 more cycles before access Timer registers
  delay++; // Refer to Page. 756 of Datasheet for info 
 
  GPIODIR_F = 0x11; // Set PF0 and PF4 to output
  GPIODEN_F = 0x11; // Set PF0 and PF4 to digital port
  GPIODATA_F = 0x0; // Set PF0 and PF4 to 0
 
  GPIODIR_N = 0x3; // Set PN0 and PN1 to output
  GPIODEN_N = 0x3; // Set PN0 and PN1 to digital port
  GPIODATA_N = 0x0; // Set PN0 and PN1 to 0
}

// Using PD7
void ADCReadPot_Init(void) {
  // STEP 2: Initialize ADC0 SS3.
  // 2.1: Enable the ADC0 clock
  volatile unsigned short delay = 0;
  RCGCADC |= 0x1;
  // 2.2: Delay for RCGCADC (Refer to page 1073)
  delay++;
  delay++;
  // 2.3: Power up the PLL (if not already)
  PLLFREQ0 |= 0x00800000; // we did this for you
  // 2.4: Wait for the PLL to lock
  while (PLLSTAT != 0x1); // we did this for you
  // 2.5: Configure ADCCC to use the clock source defined by ALTCLKCFG
  ADCCC_0 |= 0x1; // only & when inverting
  // 2.6: Enable clock to the appropriate GPIO Modules (Hint: Table 15-1)
  RCGCGPIO |= 0x8; // 1000 – Change to Port D ONLY
  // 2.7: Delay for RCGCGPIO
  delay++;
  delay++;
  // 2.8: Set the GPIOAFSEL bits for the ADC input pins
  GPIOAFSEL_D |= 0x80;  // 1000 0000 -- PD7
  // 2.9: Clear the GPIODEN bits for the ADC input pins
  GPIODIR_D &= ~0x80; 
  GPIODEN_D &= ~0x80;
  // 2.10: Disable the analog isolation circuit for ADC input pins (GPIOAMSEL)
  GPIOAMSEL_D |= 0x80;
  // 2.11: Disable sample sequencer 3 (SS3)
  ADCACTSS_0 &= ~0x8;
  // 2.12: Select timer as the trigger for SS3
  ADCEMUX_0 = 0x5000; // 0101 0000 0000 0000 0000
  // 2.13: Select the analog input channel for SS3 (Hint: Table 15-1)
  ADCSSEMUX3_0 |= 0x0;
  ADCSSMUX3_0 |= 4;
  // 2.14: Configure ADCSSCTL3 register
  ADCSSCTL3_0 |= 0x6; 
  // 2.15: Set the SS3 interrupt mask
  ADCIM_0 |= 0x8;
  // 2.16: Set the corresponding bit for ADC0 SS3 in NVIC
  EN0 = 1 << 17; // 0010 0000 0000 0000 0000
  // 2.17: Enable ADC0 SS3
  ADCACTSS_0 |= 0x8;
}

void TimerADCTrigger_Init(void) {
  // STEP 3: Initialize Timer0A to trigger ADC0 at 1 HZ.
  // Hint: Refer to section 13.3.7 of the datasheet
  volatile unsigned short delay = 0;
  RCGCTIMER_0 |= 0x1;
  delay++;
  delay++;
  
  GPTMCTL_0 |= 0x0;
  GPTMCTL_0 |= 0x20; // 0010 0001
  GPTMCFG_0 = 0x00000000;
  GPTMTAMR_0 = 0x2;
  GPTMIMR_0 |= 0x1;
  GPTMTAILR_0 = 16000000;
  GPTMADCEV |= 0x1;
  GPTMCTL_0 |= 0x1;
}

// NEXT STEP: Go to Lab3_Task1a.c and finish implementing ADC0SS3_Handler
