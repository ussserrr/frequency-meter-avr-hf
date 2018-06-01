## Overview
Frequency meter firmware for ATmega328P MCU. It uses 16-bit Timer/Counter1 (TC1) as a main measuring part of the MCU. Can be used as a standalone solution (no need in PC - HD44780 compatible LCD is used for indication). See [frequency-meter-avr-lf](https://github.com/ussserrr/frequency-meter-avr-lf) for measuring an LF band.

For HF bands the right way is to use TC1 as a counter of an external signal' pulses. Measure out a 1-second interval (or any other known) by another available timer/counter (TC0 or TC2) and you can determine the frequency as simply as
```
             N_pulses
frequency = ----------
             Interval
```
In this case, the accuracy is proportional to
```
              1
accuracy ~ --------
            period
```
value so decreasing the period (i.e. increasing an input frequency) we get better results. We also use an averaging to get more accurate results.

The algorithm also detects when input frequency is rapidly changes (e.g. you tune it) and increases refresh rate of LCD indications.


## Pinout
Pin | Function
--- | --------
PD5 (Arduino's pin 5) | Input signal
Arduino's 0, 1, 2, 3, 4, 6, 7 pins (respective PDx pins) | RS, RW, E, D4-7 (HD44780 LCD, LiquidCrystal library)


## Build and run
Build and run via [PlatformIO](https://platformio.org/). In `platformio.ini` specify parameters (e.g. F_CPU or programmer). Then run:
```bash
$ pio run  # build

$ pio run -t program  # flash using external USBASP programmer
or
$ pio run -t upload  # flash using on-board programmer
```


## Limits and accuracy
Check this [spreadsheet](https://docs.google.com/spreadsheets/d/1x5buIiSePPuyIJX-X4MWf-NA7JHJYz23IA_RD0JLFfs/edit?usp=sharing) to see some test measurements (first page). Generally, the working interval for default 16 MHz crystal is [1 kHz - 8 MHz], but wires and connections introduce a great impact on stability and accuracy, especially at high frequencies. For example, measurements mentioned above are made with a pretty long coaxial cable and maximal frequency is about 5.6 MHz. But with some much lesser wire, I was able to measure an 8 MHz signal.

Also, you can replace crystal oscillator with another one (and even overclock a little bit) to increase maximal measurable frequency (remember to adjust `TIMER2_ADJUSTMENT` macro for correct calculations and `board_build.f_cpu` in `platformio.ini` file). For testing purposes, I also applied external clock to MCU from generator and was able to achieve 32 MHz frequency. With this, I have measured a 14.3 MHz input signal and possibly may increase it up to 16 MHz (based on previous measurements results).

While TC1 simply counts input ticks (triggering by voltage level), MCU is able to measure not only strict square signals - sin wave, saw, triangle forms are also can be correctly recognized within certain limits of distortions.
