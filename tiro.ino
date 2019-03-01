#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

const byte ledPin = 0; // declare and initialise led on ATTiny digital pin zero
byte saveADCSRA; // variable to save the content of the ADC for later. if needed.
volatile byte counterWD = 0; // Count how many times WDog has fired. Used in the timing of the
// loop to increase the delay before the LED is illuminated. For example,
// if WDog is set to 1 second TimeOut, and the counterWD loop to 10, the delay
// between LED illuminations is increased to 1 x 10 = 10 seconds

void setup ()
{
  resetWatchDog ();                     // do this first in case WDog fires
  pinMode ( ledPin, OUTPUT );           // I could put to INPUT between sleep_enable() and interrupts()
  // to save more power, then to OUTPUT in the ISR after wdt_disable()

  runMotor();
} // end of setup

void loop ()
{
  if ( counterWD == 73 ) {                // Check if has run for a total of 5 minutes ((4s * 73 counts + 3s delay = 295) )
    delay(3000);                          // Delay for 3 seconds
    
    runMotor();
    counterWD = 0;                     
  } 

  sleepNow ();                          

} // end of loop ()

void runMotor() 
{
  digitalWrite ( ledPin, HIGH );     
  delay ( 500 );      
  digitalWrite ( ledPin, LOW ); 
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
  // WDTCR = bit ( WDIE ) | bit ( WDP3 )| bit ( WDP0 ); // 8 second TimeOut	
  WDTCR = bit ( WDIE ) | bit ( WDP3 );     // 4 second time out


  wdt_reset ();                            // reset WDog to parameters

} // end of resetWatchDog ()

ISR ( WDT_vect )
{
  wdt_disable ();                           // until next time....
  counterWD ++;                             // increase the WDog firing counter. Used in the loop to time the flash
  // interval of the LED. If you only want the WDog to fire within the normal
  // presets, say 2 seconds, then comment out this command and also the associated
  // commands in the if ( counterWD..... ) loop, except the 2 digitalWrites and the
  // delay () commands.
} // end of ISR
