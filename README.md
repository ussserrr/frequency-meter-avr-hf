## Overview
Frequency meter firmware for ATmega328P MCU. It uses 16-bit Timer/Counter1 (TC1) as a main measuring part of the MCU.

Configured in "Input capture" mode, TC1 stores a number of internally generated pulses (with known timings) that had been counted during the one period of input signal, so we can find out its frequency as . This method is especially good for LF band as the accuracy there is a parameter of `N_pulses*period` value, so to improve it we should whether increase the internal frequency of TC1 (which we can't do in general) or increase the period of the measured signal (i.e. measure LF bands).

For HF bands the right way is to use TC1 as a counter of an external signal' pulses. Measure out a 1-second interval (or any other known) by another available timer/counter (TC0 and TC2) and you can determine the frequency as simply as . In this case, the accuracy is proportional to `1/period` value so decreasing the period (i.e. increasing an input frequency) we get better results.

Both methods are also use an averaging as an approach to improve the quality of the measuring.


## Build and run
PlatformIO
