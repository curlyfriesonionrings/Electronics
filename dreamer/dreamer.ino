/*

Lucid Dreamer for Arduino Nano

2015/07/06

This program contains the code for a lucid dream inducer when the correct
equipment is connected to the Arduino Nano (or possibly other Arduino boards).

This program reads the input of phototransitor and detects any significant
change. Such changes (edges) are classified as a rapid eye movement, which
this program will track. If the number of movements reachers a certain threshold,
it is determined that the user is in REM sleep, so two LEDs (one per eye) will
be flashed at repeated intervals for a number of times.

The Arduino contains a reset button to restart the program. A pushbutton is
connected to test the eye LED functionality, as I found occasionally that the
LEDs had issues lighting up.

Originally, a test mode was implemented so that the user can verify eye movement
detections, but such a mode is deprecated with the Arduino. An output pin can be
used for test regardless of whether an LED is connected or not. "Enabling" test
mode then simply becomes connecting an LED to the designed test mode pin.

*/

/*
To save power, a timer is implemented in this solution. The timer is set up for 0.26 s.
I found that keeping everything on overnight at the Arduino clock frequency of 16 Hz
drained two 3.3V batteries way too quickly.

According to:
http://www.brown.edu/Departments/Engineering/Courses/122JDD/Lcturs/sacc05.html
Saccades happen at about 3 Hz, so arranging the timer to make ~4 Hz measurements seems
like a good idea.

For more detail on the timer, see:
http://donalmorrissey.blogspot.hk/2011/11/sleeping-arduino-part-4-wake-up-via.html
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

// Minutes for the reset threshold timer
const unsigned long RESET_THRESH_MIN = 10;
const unsigned long SEC_PER_MIN = 60;

// # eye movement counts before initiating blink sequence
const unsigned int EDGE_THRESHOLD = 20;
// 1 s sleep, so we can count the number of sleeps without a detection
const unsigned long RESET_THRESHOLD = RESET_THRESH_MIN * SEC_PER_MIN;
// Arbitrary sensitivity, obtained through trial and error
const unsigned int DETECT_THRESHOLD = 7;
// For testing
//const unsigned int DETECT_THRESHOLD = 1500;
// # of blink cycles to go through
const unsigned int BLINK_CYCLES = 100;

// ~0.1 s and ~0.2 s delays
const unsigned int SHORT_DELAY = 1000;
const unsigned int LONG_DELAY = 2000;

const int pTIn = A0;      // Phototransitor input pin
const int pbIn = 13;      // D13, input for pushbutton reset

const int ledOut1 = 2;    // D2, output for one eye LED
const int ledOut2 = 3;    // D3, output for other eye LED
const int ledTest = 12;   // D12, output for one eye LED

const int iRPWR = 6;      // D6, output power for IR LED
const int pTPWR = 7;      // D7, output power for PT

unsigned int edges = 0;   // # eye movement counts
// Counter used to reset edges if it hits the threshold without any edge detections
// Since there is a sleep of 1 s every loop, we can use this easily to get the minute
// value for the threshold
volatile unsigned long resetCount = 0;

int ptVal;
int ptValLast;
int ptDiff;
int pbState;
int pbStateLast;

volatile int f_timer=0;

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
  pinMode(ledOut1, OUTPUT);
  pinMode(ledOut2, OUTPUT);
  pinMode(ledTest, OUTPUT);

  pinMode(iRPWR, OUTPUT);
  pinMode(pTPWR, OUTPUT);

  // Inputs
  pinMode(pbIn, INPUT);
  
  // Initialization
  ptVal = analogRead(pTIn);
  ptValLast = ptVal;
  pbState = digitalRead(pbIn);
  pbStateLast = pbState;

  // Turn on IR and PT
  // For some reason, LOW turns power on instead of HIGH
  digitalWrite(iRPWR, LOW);
  digitalWrite(pTPWR, LOW);

  Serial.begin(9600);

  /* Normal timer operation.*/
  TCCR1A = 0x00; 

  /* Timeout period formula:
   *  Timeout = [(1/16) * prescaler * 2^(# bit timer avail)] / 10^6, in s
   *  
   * In this case, using Counter1, we have 16 bits available
   */
  
  /* Clear the timer counter register.
   * You can pre-load this register with a value in order to 
   * reduce the timeout period
   */
  TCNT1 = 0x0000; 
  
  /* Configure the prescaler for 1:64, giving us a 
   * timeout of ~0.26 seconds
   * 
   * Prescaler is the second value in the pair
   * 
   * Prescaler possible values:  1:1, 1:8, 1:64, 1:256, 1:1024
   */
  TCCR1B = 0x03;
  
  /* Enable the timer overlow interrupt. */
  TIMSK1 = 0x01;
}

/***
 * Use of delay in this function is intentional. During a blink sequence, no
 * reading of the phototransitor should be done.
 */
void loop()
{
  if (f_timer == 1)
  {
    f_timer = 0;
    // Read ADC
    ptVal = analogRead(pTIn);
    ptDiff = abs(ptVal - ptValLast);
  
    pbStateLast = pbState;
  //  Serial.println("Phototrans val: " + String(ptVal));
    Serial.println(String(ptDiff));
  
    // Edge detected
    if (ptDiff >= DETECT_THRESHOLD && (pbState == LOW && pbStateLast == LOW))
    {
      Serial.println("EDGE DETECTED");
      digitalWrite(ledTest, HIGH);
      delay(SHORT_DELAY);
      digitalWrite(ledTest, LOW);
      edges++;
      Serial.println("Edges: " + String(edges) + "/" + String(EDGE_THRESHOLD));
      
      if (edges == EDGE_THRESHOLD)
      {
  //      Serial.println("EDGE THRESHOLD REACHED");
        // Turn off power for IR and PT to save energy
        // Again, HIGH means turn power off for some bizarre reason
        digitalWrite(iRPWR, HIGH);
        digitalWrite(pTPWR, HIGH);
    
        // Enter blink sequence
        for (int i = 0; i < BLINK_CYCLES; ++i)
        {
          digitalWrite(ledOut1, HIGH);
          digitalWrite(ledOut2, HIGH);
          delay(LONG_DELAY);
          digitalWrite(ledOut1, LOW);
          digitalWrite(ledOut2, LOW);
          delay(LONG_DELAY);
        }
        // Reset variables
        edges = 0;
        digitalWrite(iRPWR, LOW);
        digitalWrite(pTPWR, LOW);
      }
      resetCount = 0;
    }
    
    // No edge detected
    ptValLast = ptVal;
    if (++resetCount == RESET_THRESHOLD)
    {
//      Serial.println("RESET THRESHOLD");
      edges = 0;
      resetCount = 0;
    }
    
    // Check pushbutton for eye LED test
    pbState = digitalRead(pbIn);
    if (pbState == HIGH)
    {
  //    Serial.println("PUSHBUTTON DOWN");
      digitalWrite(ledOut1, HIGH);
      digitalWrite(ledOut2, HIGH);
      resetCount = 0;
    }
    else
    {
      digitalWrite(ledOut1, LOW);
      digitalWrite(ledOut2, LOW);
    }

//    Serial.println("Entering sleep");
    /* Enter sleep to conserve power */
    enterSleep();
  }
}
