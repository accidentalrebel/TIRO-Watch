#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

const byte motorPin = 0;

volatile unsigned long wdCounter = 0; // Volatile tells compiler to reload each time instead of optimizing

unsigned long motorDuration = 500000;
unsigned long waitDuration = 0;
unsigned long wdCounterTarget = 75;
unsigned long timeDelayStarted = 0;

bool isInDelay = false;

void setup () {
	pinMode ( motorPin, OUTPUT );

	ADCSRA = 0; // Turn off the ADC

	startMotorSequence(); // Run the motor once on startup
	sleepNow();
}

void loop () {
	if ( wdCounter == wdCounterTarget ) { // Check if counter has reached target.
		wdCounter = 0;
		timeDelayStarted = micros();
		isInDelay = true; // Now in delay mode
	}

	// Continue to loop until enough time has passed to reach waitDuration
	if ( isInDelay
			 && micros() - timeDelayStarted >= waitDuration) {
		startMotorSequence(); // Run the motor sequence
		wdCounter = 0;
		isInDelay = false; // End delay mode
	}

	if ( !isInDelay ) {
		sleepNow();
	}
}

void startMotorSequence() {
	digitalWrite( motorPin, HIGH );
	_delay_us(motorDuration); // Blocking function delay
	digitalWrite ( motorPin, LOW );
}

void sleepNow () {
	set_sleep_mode ( SLEEP_MODE_PWR_DOWN ); // set sleep mode to Power Down. The most energy efficient setting.

	power_all_disable (); // Turn power off to TIMER 1, TIMER 2, and Serial Interface

	noInterrupts (); // Turn off interrupts as a precaution. Timed sequence follows
	resetWatchDog (); // Reset watchdog, making sure every flag is properly set
	sleep_enable (); // Allows the system to be commanded to sleep

	interrupts (); // Turn on interrupts. Guarantees next instruction is executed
	sleep_cpu (); // Goodnight, ATTiny85

	sleep_disable (); // Returns here after Watchdog ISR fires.
	power_all_enable (); // Turn on power to TIMER1, TIMER 2, and Serial Interface
}

void resetWatchDog () {
	MCUSR = 0; // Clear various "reset" flags
	WDTCR = bit ( WDCE ) | bit ( WDE ) | bit ( WDIF ); // Allow changes, disable reset, clear existing interrupt
	WDTCR = bit ( WDIE ) | 1 << WDP3 | 0 << WDP2 | 0 << WDP1 | 0 << WDP0; // 4s timeout

	wdt_reset (); // Reset the watchdog using the parameters
}

ISR ( WDT_vect ) {
	wdt_disable(); // Disable the watchdog timer
	wdCounter++; // Increase the watchdog firing counter.
}
