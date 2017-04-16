/*Programmed by Nick Crescimanno - Electric Sheep Labs - 4/15/17
 *many thanks to MaxEmbedded for help understanding timers and registers (much of the code below is his), http://maxembedded.com/
 *
 *This code is uploaded to an AVR attiny85 mcu (84 works too) to generate servo control (PB0) signals based on an analog input (PB4). 
 *
 *Revisions:
 *first draft - Nick Crescimanno - 4/15/17
 *
 *To do:
 *make total period fixed for any signal period 
 *make it more easy to read and configureable....naaah
 *
 *
 * How to use:
 * Wire an ESC or servo's control line into PB0 (pin 5 when counting ccw from top left) and wire in 0-5V control signal to PB4 (pin 3 ccw from top left).
 * Upload this program to an attiny mcu by means of an ISP, or use an arduino as an ISP (plenty of help online). This code was made for attiny running at 8mhz, 
 * but simple modifications could be made for 16mhz. Configure the updateSP method for desired analog in level adustment and min_ and max_speed for your device.
 * Make sure the servo/ESC has power (5V usually) and is common grounded. Should be ready to rock and embed!
 * 
 */


#include <avr/io.h>

#define max_speed 237
#define min_speed 130
#define enable_speed 125
#define cycles_before_update 12
#define enable_cycles 200
#define timer_overflows 10 //this gives desired servo control signal period of about 20 ms
#define smooth_speed 2

volatile uint8_t tot_overflow;
volatile int flag = 1, flag2 = 1;
int i = 11;
int analogIn = 0;
int speedOut = min_speed;
int speedSP = min_speed;
 
void timer0_init()
{
    // set up timer with prescaler = 64 (for attiny85)
    TCCR0B |= (1 << CS01)|(1 << CS00);
 
    // initialize counter
    TCNT0 = 0;

    // enable overflow interrupt
    TIMSK |= (1 << TOIE0);
 
    // enable global interrupts
    sei();
 
    // initialize overflow counter variable
    tot_overflow = 0;
}

ISR(TIMER0_OVF_vect)
{
    // keep a track of number of overflows
    tot_overflow++;
}

void adc_init()
{
    // ADC Enable and prescaler of 128
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);
 
  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));
 
  return (ADC);
}

void updateSP(){
  //get sensor imput, convert to delay...this should be configured for min and max period desired
  analogIn = adc_read(2);
  speedSP = (int)(((float)(analogIn-500))/5.0+134.0); //again, this looks weird because of the setup im using
  if(speedSP > max_speed) speedSP = max_speed;
  else if(speedSP < min_speed) speedSP = min_speed;
}

void enableMotor(){
  unsigned int p = 0;
    while(p<enable_cycles){
     
          if (tot_overflow >= timer_overflows)  // NOTE: '>=' is used (skipping may occur)
            {
              if(flag){
                PORTB ^= (1 << 0);    // toggles the pin (on)
                flag = 0;
              }
              // check if the timer count reaches 124
              if (TCNT0 >= enable_speed)
              {
                  PORTB ^= (1 << 0);    // toggles the pin (off)
                  TCNT0 = 0;            // reset counter
                  tot_overflow = 0; 
                  flag = 1;
                  p++;
              }
            }    
    }
}

void setupStuff(){
  // connect led to pin PB0
    DDRB |= (1 << 0);
    //analog read init on PB2
    DDRB |= (0 << 4);
    // initialize timer
    timer0_init();
    //adc setup
    adc_init();
  }
  
int main(void)
{
    setupStuff();
    enableMotor();
    // loop forever
    while(1)
    { 
      if (tot_overflow >= timer_overflows)  // NOTE: '>=' is used
        {
          if(flag){
            PORTB ^= (1 << 0);    // toggles the pin (on)
            flag = 0;
          }
          // check if the timer count reaches desired speed
          if (TCNT0 >= speedOut)
          {
              PORTB ^= (1 << 0);    // toggles the pin (off)
              TCNT0 = 0;            // reset counter
              tot_overflow = 0; 
              flag = 1;

              //this is for smoothing as the SP changes... smoothing can be removed if u really want...
              if(abs(speedOut-speedSP)<=smooth_speed) ; //have reached value (close enough anyway
              else if(speedOut > speedSP) speedOut-=smooth_speed;
              else speedOut+=smooth_speed;
              i++; 

              //update once every so often
              if(i >= cycles_before_update){
              i=0;
              updateSP();
              }    
          }
        } 
    }
}
