#ifndef WIRINGPI_H
#define WIRINGPI_H

#ifdef __cplusplus
extern "C" {
#endif

#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

int wiringPiSetup(void);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);

#ifdef __cplusplus
}
#endif

#endif
