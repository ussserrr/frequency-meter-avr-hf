#include <Arduino.h>
#include <LiquidCrystal.h>


LiquidCrystal lcd(0, 1, 2, 3, 4, 6, 7);  // RS, RW (optional), E, D4-7
#define BUFFER_SIZE 25
char bufferA[BUFFER_SIZE];  // 1st raw of LCD
// char bufferB[BUFFER_SIZE];  // 2nd raw of LCD

uint32_t accumulator = 0;
uint8_t timer1_overflows_counter = 0;
double frequency;
double previous_frequency = 0.0;
uint8_t timer2_additional_prescaler = 25;
// #define TIMER2_ADDITIONAL_PRESCALER 60
uint8_t timer2_additional_prescaler_cnt = 0;
/*
 *  256 ticks of Timer2 (w/ 1024 prescaler) are equal to 1/76.293945313 Hz
 *  so we need to multiply frequency by 76.293945313
 */
 // 16 MHz - 61.03515625
 // 20 MHz - 76.293945313
 // 25 MHz - 95.3674316406
#define TIMER2_ADJUSTMENT 61.03515625
/*
 *  Constant adjustments because of errors
 */
// #define ERROR_ADJUSTMENT 1.000136814232641724
#define ERROR_ADJUSTMENT 1
/*
 *  Changing in 0.25% of current frequency mean that user is tuning right now
 *  so we need to increase refresh rate
 */
#define FREQUENCY_OFFSET 0.0025


int main() {
    lcd.begin(16, 2);
	lcd.clear();
	lcd.print("starting...");

    // PCICR |= (1<<PCIE0);
    // PCMSK0 |= (1<<PCINT0);

    /*
     *  Timer1' clock source is measured signal. We simply count ticks.
     *  When TCNT1 overflows we increment special variable in ISR
     */
    TIMSK1 |= (1<<TOIE1);
    TCCR1B |= (1<<CS12) | (1<<CS11);

    /*
     *  Timer2 simply counts ~1 s intervals for frequency output.
     *  But because of timer' 8-bit width ISR fires 60 times so we
     *  accumulate values and then average them
     */
    TIMSK2 |= (1<<TOIE2);
    TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);

    sei();

    while (1) {}
}


ISR (TIMER2_OVF_vect, ISR_NAKED) {
    TCCR1B = 0;
    TCCR2B = 0;

    accumulator += timer1_overflows_counter*65535 + TCNT1;
    timer1_overflows_counter = 0;

    if (++timer2_additional_prescaler_cnt == timer2_additional_prescaler) {
        timer2_additional_prescaler_cnt = 0;

        frequency = ((double)accumulator/timer2_additional_prescaler) * TIMER2_ADJUSTMENT * ERROR_ADJUSTMENT;
        accumulator = 0;

        if ( fabs(frequency-previous_frequency) > (previous_frequency*FREQUENCY_OFFSET) ) {
            timer2_additional_prescaler = 25;
        }
        else {
            timer2_additional_prescaler = 76;
        }
        previous_frequency = frequency;

        lcd.clear();
        sprintf(bufferA, "%g kHz", frequency/1000);
        lcd.print(bufferA);
    }

    TCNT1 = 0;
    TCNT2 = 0;

    TCCR1B |= (1<<CS12) | (1<<CS11);
    TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);

    reti();
}


ISR (TIMER1_OVF_vect, ISR_NAKED) {
    timer1_overflows_counter++;

    reti();
}
