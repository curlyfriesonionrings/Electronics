/*

Clapper Controller for Arduino Nano

2016/04/07

Controller program for a clapper circuit. The paired circuit should have an electret microphone
connected to one of the analog pins on the Nano. The Nano samples the mic output to detect
changes to determine if a clap (or loud noise) has been processed. The Nano then drives one of
the output pins, based on the logic for detemining the switch trigger.

In this version, when a loud clap is detected, the Nano goes into a state where it listens for
a second clap within a short time frame. This state is managed by a simple boolean.

*/

/*
TODO: Take care with normalizing values when the MCU sleeps. It's possible to record one mic
value and sleep, waking up to a drastically different reading. The sleep duration is small
enough that this shouldn't be that big of an issue, but worth mentioning
*/

/*
 * For the Arduino Nano, it looks like the output pin numbers are equal to
 * the target digital pin. IE:
 *  1 = D1
 *  2 = D2
 *  3 = D3
 *  ...
 *  12 = D12
 */

#include <avr/sleep.h>
#include <avr/power.h>

// Using a sleep of 0.004 s, we can count loops for a timeout
// For 0.4 s timeout, this will be 100 loops
const unsigned int DOUBLE_CLAP_TIMEOUT = 100;

// Arbitrary sensitivity (obtained through testing) for detecting a clap
const int DETECT_THRESHOLD = 5;

const int micIn = A0;     // Microphone input

const int output = 2;     // D2, output for driving load

volatile int micVal;
volatile int micValLast;
volatile int micDiff;

// Timeout counter
unsigned int resetCount;

volatile bool listenState = false;
volatile bool turnOn = true;

volatile int f_timer = 0;

/***************************************************
 *  Name:        ISR(TIMER1_OVF_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Timer1 Overflow interrupt.
 *
 ***************************************************/
ISR(TIMER1_OVF_vect)
{
  /* set the flag. */
   if (f_timer == 0)
   {
     f_timer = 1;
   }
}


/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  sleep_enable();


  /* Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   */
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

void setup()
{
  // Outputs
  pinMode(output, OUTPUT);

  // Inputs
  pinMode(micIn, INPUT);
  
  // Initialization
  micVal = analogRead(micIn);
  micValLast = micVal;
  digitalWrite(output, LOW);
  
  Serial.begin(9600);

  /* Normal timer operation.*/
  TCCR1A = 0x00; 

  /* Timeout period formula:
   *  Timeout = [(1/16) * prescaler * 2^(# bit timer avail)] / 10^6, in s
   *  
   * In this case, using Counter1, we have 16 bits available. Prescaler value
   * is the xx value in 1:xx
   */
  
  /* Clear the timer counter register.
   * You can pre-load this register with a value in order to 
   * reduce the timeout period
   */
  TCNT1 = 0x0000; 
  
  /* Configure the prescaler for 1:1, giving us a 
   * timeout of ~0.004 seconds
   * 
   * Prescaler possible values:  1:1, 1:8, 1:64, 1:256, 1:1024
   */
  TCCR1B = 0x01;
  
  /* Enable the timer overlow interrupt. */
  TIMSK1 = 0x01;
}

/***
 * Poll mic for value and compare to last cycle value. A double-clap should be
 * required to turn the switch on. Uses a flag boolean to determine if a
 * double-clap has been detected.
 */
void loop()
{
  if (f_timer == 1)
  {
    f_timer = 0;
    // Read ADC
    micVal = analogRead(micIn);
    // Only count rising edges
    micDiff = micVal - micValLast;
  
  //  Serial.println("Mic val: " + String(micVal));
//    Serial.println(String(micDiff));
  
    // Clap detected
    if (micDiff >= DETECT_THRESHOLD)
    {
      Serial.println("Diff: " + String(micDiff));
      Serial.println("CLAP DETECTED");
  
      // This is first clap, listen for the second clap
      if (!listenState)
      {
        listenState = true;
        resetCount = 0;
        Serial.println("Listening for second clap");
      }
      else
      {
        Serial.println("Second clap!");
        // Turn on if we were NOT on before, turn off if we were ON
        if (turnOn)
        {
          digitalWrite(output, HIGH);
          turnOn = false;
        }
        else
        {
          digitalWrite(output, LOW);
          turnOn = true;
        }
        listenState = false;
      }
    }
  
    // Increment reset timer
    if (listenState)
    {
      if (++resetCount == DOUBLE_CLAP_TIMEOUT)
      {
        Serial.println("RESET THRESHOLD");
        listenState = false;
        resetCount = 0;
      }
    }
  
    // No clap detected
    micValLast = micVal;
    /* Enter sleep to conserve power */
    enterSleep();
  }
}
