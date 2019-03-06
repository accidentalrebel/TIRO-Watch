#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

const byte ledPin = 0; // declare and initialise led on ATTiny digital pin zero
byte saveADCSRA; // variable to save the content of the ADC for later. if needed.

unsigned long wdCounter = 0;
unsigned long motorDuration = 500000;
unsigned long waitDuration = 1500000;
unsigned long wdCounterTarget = 3625;

unsigned long timeDelayStarted = 0;

bool isInDelay = false;
bool isMotorRunning = false;

void setup ()
{
  pinMode ( ledPin, OUTPUT );

	resetWatchDog();
	startMotorSequence();
	resetWatchDog();
}

void loop ()
{
	if ( wdCounter == wdCounterTarget ) {
		wdCounter = 0;
		timeDelayStarted = micros();
		isInDelay = true;
	}

	if ( isInDelay
			 && micros() - timeDelayStarted >= waitDuration) {
		startMotorSequence();
		wdCounter = 0;
		isInDelay = false;
	}

	if ( !isInDelay ) {
		sleepNow();
	}
}

void startMotorSequence()
{
		runMotor();
		_delay_us(motorDuration);
		stopMotor();
}

void runMotor() 
{
  digitalWrite ( ledPin, HIGH );
	isMotorRunning = true;
}

void stopMotor() {
  digitalWrite ( ledPin, LOW );
	isMotorRunning = false;
}

void sleepNow ()
{
  set_sleep_mode ( SLEEP_MODE_PWR_DOWN ); // set sleep mode Power Down
  saveADCSRA = ADCSRA;                    // save the state of the ADC. We can either restore it or leave it turned off.
  ADCSRA = 0;                             // turn off the ADC
  power_all_disable ();                   // turn power off to ADC, TIMER 1 and 2, Serial Interface

  noInterrupts ();                        // turn off interrupts as a precaution
  resetWatchDog ();                       // reset the WatchDog before beddy bies
  sleep_enable ();                        // allows the system to be commanded to sleep
  interrupts ();                          // turn on interrupts
 
  sleep_cpu ();                           // send the system to sleep, night night!

  sleep_disable ();                       // after ISR fires, return to here and disable sleep
  power_all_enable ();                    // turn on power to ADC, TIMER1 and 2, Serial Interface
  // ADCSRA = saveADCSRA;                 // turn on and restore the ADC if needed. Commented out, not needed.

} // end of sleepNow ()

void resetWatchDog ()
{
  MCUSR = 0;
  WDTCR = bit ( WDCE ) | bit ( WDE ) | bit ( WDIF ); // allow changes, disable reset, clear existing interrupt
  // WDTCR = bit ( WDIE ) | bit ( WDP2 )| bit ( WDP1 ); // set WDIE ( Interrupt only, no Reset ) and 1 second TimeOut
  //WDTCR = bit ( WDIE ) | bit ( WDP3 )| bit ( WDP0 ); // 8 second TimeOut
	//WDTCR = bit ( WDIE ) | bit (WDP0); // 32 ms TimeOut
	WDTCR = bit ( WDIE ) | 0 << WDP3 | 0 << WDP2 | 0 << WDP1 | 0 << WDP0; // 16 ms TimeOut
  //WDTCR = bit ( WDIE ) | bit ( WDP3 );     // 4 second time out

  wdt_reset ();                            // reset WDog to parameters

} // end of resetWatchDog ()

ISR ( WDT_vect )
{
  wdt_disable ();                           // until next time....
  wdCounter ++;                             // increase the WDog firing counter. Used in the loop to time the flash
  // interval of the LED. If you only want the WDog to fire within the normal
  // presets, say 2 seconds, then comment out this command and also the associated
  // commands in the if ( wdCounter..... ) loop, except the 2 digitalWrites and the
  // delay () commands.
} // end of ISR
