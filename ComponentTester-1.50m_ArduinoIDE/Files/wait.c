/* ************************************************************************
 *
 *   wait functions
 *
 *   based on code from Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define WAIT_C


/*
 *  includes
 */

/* basic includes */
#include <Arduino.h>

/* local includes */
#include "config.h"           /* global configuration */


void wait5s() {   delay(5000); }
void wait4s() {   delay(4000); }
void wait3s() {   delay(3000); }
void wait2s() {   delay(2000); }
void wait1s() {   delay(1000); }
void wait1000ms() {   delay(1000); }
void wait500ms() {   delay(500); }
void wait400ms() {   delay(400); }
void wait300ms() {   delay(300); }
void wait200ms() {   delay(200); }
void wait100ms() {   delay(100); }
void wait50ms() {   delay(50); }
void wait40ms() {   delay(40); }
void wait30ms() {   delay(30); }
void wait20ms() {   delay(20); }
void wait10ms() {   delay(10); }
void wait5ms() {   delay(5); }
void wait4ms() {   delay(4); }
void wait3ms() {   delay(3); }
void wait2ms() {   delay(2); }
void wait1ms() {   delay(1); }
void wait500us() {   delayMicroseconds(500); }
void wait400us() {   delayMicroseconds(400); }
void wait300us() {   delayMicroseconds(300); }
void wait200us() {   delayMicroseconds(200); }
void wait100us() {   delayMicroseconds(100); }
void wait50us() {   delayMicroseconds(50); }
void wait40us() {   delayMicroseconds(40); }
void wait30us() {   delayMicroseconds(30); }
void wait20us() {   delayMicroseconds(20); }
void wait10us() {   delayMicroseconds(10); }
void wait5us() {   delayMicroseconds(5); }
void wait4us() {   delayMicroseconds(4); }
void wait3us() {   delayMicroseconds(3); }
void wait2us() {   delayMicroseconds(2); }
void wait1us() {   delayMicroseconds(1); }
void wait500ns() {   delayNanoseconds(500); }

/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef WAIT_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
