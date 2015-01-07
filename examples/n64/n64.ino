#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 
#define del14 __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t")
#define del15 __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t")
#define del16 __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t")
#define del3s del14; del16; del16
//ICES1 = 1 -> rising edge
//ICES1 = 0 -> falling edge
const int data =  0;
int counter = 0;
uint8_t memory[4];

void setup() {

  cbi(PORTB, data);
  cbi(DDRB, data);
  TCCR1A = 0; //??outmode for pwm stuff? should be irrelevant for me
  TCCR1B = (1 << ICNC1) | (1 <<CS10); //CS is clockmode, ICE is rising/falling, ICNC is noise canceller
  //TIFR1 = _BV(ICF1) | _BV(OCF1A) | _BV(TOV1); // clear all timer flags - clears on 1
  //TIFR1 = _BV(ICF1) | _BV(TOV1); 
  Serial.begin(9600);
}
//register uint8_t ices1_val = _BV(ICES1); 
//ICF1 = input compare flag
//OCF1A = output compare flag
//TOV1 =overflow flag 
//
//let's say we actually want to wait for the 4 bytes to come through, at 16 mhz, that's...
// 
//TIMSK1 = _BV(ICIE1);
//ICIE1 = interrupt  enable bit
//TOIE1 timer overflow interrupt //not needed, we can reset ourselves
//Actually we could use the output compare flag interrupt to stop capturing! -
//just wait a decent amount of time to start processing, as waiting for the
//whole counter is actually too slow
//Note: check the 5th bit in the counter every time, as that is the one
//differing for low / high bits

inline void SENDNULL() {
  sbi(DDRB, data);
  del3s;
  //delayMicroseconds(3);
  cbi(DDRB, data);
  del14;
  //delayMicroseconds(1);
}
inline void SENDONE() {
  sbi(DDRB, data);
  del14;
  //delayMicroseconds(3);
  cbi(DDRB, data);
  del3s;
  //delayMicroseconds(1);
}
inline void SENDSTOP() {
  sbi(DDRB, data);
  del14;
  cbi(DDRB, data);
  //del14;
  //del16;
}

void loop() {
  register uint8_t buffer;
  //what is the idea?
  //1. step: send something to the n64
  //
  //copy paste from n64.ino

  //now: we can wait for input
  //setup should be done already
  //either go into waiting loop or do it via interrupts
  //
  //

  for (int i = 0; i < 7; ++i) {
    SENDNULL();
  }
  SENDONE();
  SENDSTOP();
  //counter = 0;
  //while ((PINB & 1)  && counter < 2000) {
  //  ++counter;
 // }
  //Serial.println(counter);

  TCNT1 = 0;
  TIFR1 = _BV(ICF1) | _BV(TOV1); 
  TCCR1B &= ~(1 << ICES1);

  while(!(TIFR1 & (_BV(ICF1)| _BV(TOV1)))) {
  }
  TCNT1 = 0;
  TIFR1 |= _BV(ICF1);
  TCCR1B |= (1 << ICES1);

  for (int i = 0; i < 4; ++i) {
    buffer = 0;
    for (int j = 0; j < 8; ++j) {
      while(!(TIFR1 & (_BV(ICF1) | _BV(TOV1)))) {
      }
      buffer |= (((ICR1>>5)& 1))<< j;
      TIFR1 |= _BV(ICF1);
      //if (! (TIFR1 & _BV(TOV1))) { ++counter;}
    }
    memory[i] = buffer;
  }
  if (TIFR1 & _BV(TOV1)) {
    //overflow happened!
    Serial.print("Useless values");
  }
  Serial.println(memory[0], BIN);
  Serial.println(memory[1], BIN);
  Serial.println(memory[2], BIN);
  Serial.println(memory[3], BIN);
  Serial.println("");
  //Serial.println("counter");
  //Serial.println(counter, DEC);
  delay(500);

}
