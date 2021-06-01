#ifndef ROTARY_ENCODER_HPP
#define ROTARY_ENCODER_HPP

#include <Arduino.h>

/** @struct RotaryEncoder
 *  @brief Data structure for easy representation of Incremental Rotary 
 *        Encoder electrical component.
*/
typedef struct{

  // untuk menyimpan kondisi pin CLK yang sekarang
  bool currentCLK;

  // untuk menyimpan kondisi pin CLK yang lalu
  bool lastCLK;

  // address dari PORTx register, untuk set pull-up ke OFF
  volatile uint8_t *PORT_REG;

  // address dari DDRx register, untuk set pin ATMega ke input mode
  volatile uint8_t *DDR_REG;

  // address dari PINx register, untuk membaca kondisi pin input
  volatile uint8_t *PIN_REG;

  // pin ATMega yang terhubung ke pin Encoder
  uint8_t DT;

  // pin ATMega yang terhubung ke pin Encoder
  uint8_t CLK;

  // pin ATMega yang terhubung ke pin Encoder
  uint8_t SW;

  // untuk memudahkan membaca akan dibuat bitmasking berdasarkan
  // pin assignment
  uint8_t bit_mask;

}RotaryEncoder;



void initEncoder(RotaryEncoder*, volatile uint8_t *PORT, volatile uint8_t *DDR,
                 volatile uint8_t *PIN, uint8_t CLK = 0, uint8_t DT = 1, 
                 uint8_t SW = 2);
bool readEncoder(RotaryEncoder*, int *counter);

#endif