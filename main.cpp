/*
 * main.cpp
 *
 *  Created on: 25.10.2019
 *      Author: Arnali
 */
 #include "stdbool.h"
 #include "stdint.h"
 #include "inc/hw_ints.h"
 #include "inc/hw_gpio.h"
 #include "inc/hw_types.h"
 #include "inc/hw_memmap.h"
 #include "inc/hw_adc.h"
 #include "inc/hw_timer.h"
 #include "driverlib/gpio.h"
 #include "driverlib/interrupt.h"
 #include "driverlib/pin_map.h"
 #include "driverlib/sysctl.h"
 #include "driverlib/rom_map.h"
 #include "driverlib/adc.h"
 #include "driverlib/timer.h"
 #include "driverlib/eeprom.h"
 #include "driverlib/debug.h"
 #include "Tools/MAFilter.hpp"
 #include "Tools/Timer"
 #include "Tools/LED.hpp"
 #include "can/CanDriverBIB.hpp"
 #include "hvmpb.hpp"
 #include "driverlib/sysctl.h"
 #include "ADCDriver.hpp"

#ifdef DEBUG
#define PRINT_DEBUG
#endif

#define ADC_AVERAGE_MEASUREMENTS 100


Timer ledBlinkTimer1(89);   // Blink LED to show that BIB is alive
Timer adcSampleTimer(10);   // Read voltages every 10ms
Timer canSendTimer(500);   // Send data twice per second
Timer saveEEPROMTimer(1000, Timer::ONCE);   // Save new value of powerUpCounter to EEPROM

MAFilter cellVoltage1(ADC_AVERAGE_MEASUREMENTS);
MAFilter cellVoltage2(ADC_AVERAGE_MEASUREMENTS);
MAFilter cellVoltage3(ADC_AVERAGE_MEASUREMENTS);
MAFilter cellVoltage4(ADC_AVERAGE_MEASUREMENTS);
MAFilter batteryVoltage(ADC_AVERAGE_MEASUREMENTS);
MAFilter BIBVoltage(ADC_AVERAGE_MEASUREMENTS);
MAFilter boardTemperature(ADC_AVERAGE_MEASUREMENTS);

LED statusLed(LED::PORTPF2);  //BIB Led Status

CANDriverBIB can;

void setupADC(void);
void readADC(void);
void setupEEPROM();

uint32_t adc0Buffer[8];
uint32_t adc1Buffer[8];
uint32_t powerUpCounter = 0;






extern "C" void Timer0BISR(void);

volatile bool  timer1msflag = false;


void timer1msInit(void){
	// Initializes a timer with 1ms load
	//Timer0B periodic interrupt every millisecond
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_B, SysCtlClockGet() / 1000);
	// Configure the Timer0B interrupt for timer timeout.
	TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
	IntEnable(INT_TIMER0B);
	TimerEnable(TIMER0_BASE, TIMER_B);
}

int main(){
	SysCtlClockSet(SYSCTL_SYSDIV_8 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); //25Mhz
	timer1msInit();
	HVMPB hvmpb;
	IntMasterEnable();
	hvmpb.init();
	while (true){
		if (timer1msflag){
			//timer1msflag = false;
			hvmpb.run();
			timer1msflag = false;
		}
	}
	return 0;
}


// Interrupts
void Timer0BISR(void){
	 TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
	 timer1msflag = true;

}
