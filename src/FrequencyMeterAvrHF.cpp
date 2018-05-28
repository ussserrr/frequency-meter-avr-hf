#include <Arduino.h>


#include <LiquidCrystal.h>
LiquidCrystal lcd(0, 1, 2, 3, 4, 6, 7);  // RS, RW (optional), E, D4-7
#define BUFFER_SIZE 25
char buffer[BUFFER_SIZE];  // buffer for raw of LCD


/*
 *  Accumulate counted pulses of input signal for averaging
 */
uint32_t ticks_accumulator = 0;
/*
 *  When counting Timer/Counter1 is overflowed (i.e. frequency is high enough to
 *  contain 2^16 ticks before LCD update), we increment corresponding variable
 *  to use it later
 */
uint8_t timer1_overflows_counter = 0;

double frequency;
double previous_frequency = 0.0;

/*
 *  We use additional "prescaler" (integer variable) to get whether 1-second or
 *  1/3-second updating period. Bigger period is used when input measured
 *  frequency is stable while lesser one is for tuning mode (when input
 *  frequency changes rapidly)
 */
#define TIMER2_ADDITIONAL_PRESCALER_NORMAL_MODE 76
#define TIMER2_ADDITIONAL_PRESCALER_TUNING_MODE 25
uint8_t timer2_additional_prescaler = TIMER2_ADDITIONAL_PRESCALER_TUNING_MODE;
uint8_t timer2_additional_prescaler_cnt = 0;
/*
 *  Changing in 0.25% of current frequency mean that user is tuning right now
 *  so we need to increase refresh rate
 */
#define FREQUENCY_OFFSET 0.0025

/*
 *  256 ticks of Timer2 (w/ 1024 prescaler and 16 MHz clock) are equal to
 *  1/76.293945313 Hz so we need to multiply frequency by 76.293945313
 *
 *    16 MHz - 61.03515625
 *    20 MHz - 76.293945313
 *    25 MHz - 95.3674316406 (overclocked MCU, you may need to increase Vcc
 *                            voltage, but it will make possible to measure
 *                            higher frequencies)
 */
#define TIMER2_ADJUSTMENT 61.03515625
/*
 *  Constant adjustments because of errors
 */
#define ERROR_ADJUSTMENT 1.0  // 1.000136814232641724



int main() {

    lcd.begin(16, 2);
	lcd.clear();
	lcd.print("starting...");

    /*
     *  Timer1' clock source is the measured signal. We simply count its ticks.
     *  When TCNT1 overflows we increment special variable in ISR
     */
    TIMSK1 |= (1<<TOIE1);
    TCCR1B |= (1<<CS12)|(1<<CS11);

    /*
     *  Timer2 simply counts ~1 s intervals for frequency output.
     *  But because of timer' 8-bit width ISR fires 60 times so we
     *  accumulate values and then average them
     */
    TIMSK2 |= (1<<TOIE2);
    TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);

    sei();

    while (1) {}
}



/*
 *  Timer for LCD. Inside it we use additional "prescaler" (integer variable) to
 *  get whether 1-second or 1/3-second updating period. Bigger period is used
 *  when input measured frequency is stable while lesser one is for tuning mode
 *  (when input frequency changes rapidly)
 */
ISR (TIMER2_OVF_vect, ISR_NAKED) {

    // stop both timers/counters
    TCCR1B = 0;
    TCCR2B = 0;

    // Accumulate counted pulses including previous overflows and current value
    // of register
    ticks_accumulator += timer1_overflows_counter*65535 + TCNT1;
    timer1_overflows_counter = 0;

    // Additional prescaler so we display the measured frequency much less often
    // (also averaging)
    if (++timer2_additional_prescaler_cnt == timer2_additional_prescaler) {
        timer2_additional_prescaler_cnt = 0;

        frequency = ((double)ticks_accumulator/timer2_additional_prescaler) * TIMER2_ADJUSTMENT * ERROR_ADJUSTMENT;
        ticks_accumulator = 0;

        // If frequency is changed too often we assume that we are in the Tuning
        // mode and switch to more frequent rate of LCD updates
        if ( fabs(frequency-previous_frequency) > (previous_frequency*FREQUENCY_OFFSET) )
            timer2_additional_prescaler = TIMER2_ADDITIONAL_PRESCALER_TUNING_MODE;
        else
            timer2_additional_prescaler = TIMER2_ADDITIONAL_PRESCALER_NORMAL_MODE;
        previous_frequency = frequency;

        lcd.clear();
        sprintf(buffer, "%g kHz", frequency/1000);
        lcd.print(buffer);
    }

    // reset counting registers of both timers/counters
    TCNT1 = 0;
    TCNT2 = 0;

    // set prescalers/sources and start both timers/counters
    TCCR1B |= (1<<CS12)|(1<<CS11);
    TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);

    reti();
}



/*
 *  When counting Timer/Counter1 is overflowed (i.e. frequency is high enough to
 *  contain 2^16 ticks before LCD update), we increment corresponding variable
 *  to use it later
 */
ISR (TIMER1_OVF_vect, ISR_NAKED) {

    timer1_overflows_counter++;

    reti();
}
