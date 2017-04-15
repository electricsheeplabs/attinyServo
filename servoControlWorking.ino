#include <avr/io.h>

volatile uint8_t tot_overflow;
volatile int flag = 1;
int i = 125;
 
void timer0_init()
{
    // set up timer with prescaler = 1024
    TCCR0B |= (1 << CS01)|(1 << CS00);
    //TCCR0B |= (1 << CS00);
 
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

void setup(){}
int main()
{
    // connect led to pin PC0
    DDRB |= (1 << 0);
 
    // initialize timer
    timer0_init();
 
    // loop forever
    while(1)
    {
      if (tot_overflow >= 10)  // NOTE: '>=' is used
        {
          if(flag){
            PORTB ^= (1 << 0);    // toggles the led
            flag = 0;
          }
          // check if the timer count reaches 124
          if (TCNT0 >= i)
          {
              PORTB ^= (1 << 0);    // toggles the led
              TCNT0 = 0;            // reset counter
              tot_overflow = 0; 
              flag = 1;
              if(i>=254) i=125;
              i++;
              
          }
        }
       
    }
}
