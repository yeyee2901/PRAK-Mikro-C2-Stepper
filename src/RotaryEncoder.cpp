#include <RotaryEncoder.hpp>

// ENCODER FUNCTIONS ---------------------------------------------------------

/**
 * @brief Initialize encoder. Encoders are passed by reference.
 * 
 * @param *Encoder  : Encoder struct address
 * @param *PORT_REG : PORTx register address (ex: &PORTA), for disabling pull-ups
 * @param *DDR_REG  : DDRx register address, for setting as input mode
 * @param *PIN_REG  : PINx register address, for reading input pins
 * @param CLK       : CLK pin num , default = 0
 * @param DT        : DT pin num  , default = 1
 * @param SW        : SW pin num  , default = 2
 * 
 * @return None
*/
void initEncoder( RotaryEncoder *encoder, 
                  volatile uint8_t *PORT_REG,
                  volatile uint8_t *DDR_REG, 
                  volatile uint8_t *PIN_REG, 
                  uint8_t CLK, uint8_t DT, uint8_t SW )
{
  // assign port ke encoder
  encoder->PORT_REG = PORT_REG;
  encoder->DDR_REG  = DDR_REG;
  encoder->PIN_REG  = PIN_REG;

  encoder->CLK = CLK;
  encoder->DT  = DT;
  encoder->SW  = SW;

  // create bitmask
  encoder->bit_mask = (1 << encoder->CLK) | 
                      (1 << encoder->DT)  | 
                      (1 << encoder->SW);
  // bitmask default = 0x07 = 0000 0111
  // namun bisa diubah pada saat memanggil inisialisasi encoder 
  // dengan memasukkan nomor pin CLK, DT, SW yang baru

  // set PORT encoder jadi input
  // pull up OFF
  *(encoder->DDR_REG)  &= ~(encoder->bit_mask); 
  *(encoder->PORT_REG) &= ~(encoder->bit_mask); 

  // coba read 1x untuk set state awal
  // *proses read dibantu dengan bitmask
  uint8_t temp = *(encoder->PIN_REG) & encoder->bit_mask;
  encoder->currentCLK = (temp & (1 << encoder->CLK)) >> encoder->CLK;
  
  // simpan sebagai last CLK state
  encoder->lastCLK = encoder->currentCLK;
}


/**
 * @brief Read using encoder passed in as reference. Pass the variables
 * you want to modify it's value using rotary encoder as a reference.
 * @param *Encoder struct address / reference
 * @param *counter address / reference of variable that you want to modify
 *                 using the encoder
 * @return TRUE if encoder is pressed, FALSE otherwise
 * 
*/
bool readEncoder(RotaryEncoder *encoder, int *counter)
{
  bool EncoderDitekan = false;

  // read encoder pins
  uint8_t temp = *(encoder->PIN_REG) & encoder->bit_mask;

  // simpan CLK state & SW state
  // Operasi bit masking, dan shift ke paling kanan agar yang
  // terbaca hanya 1 bit saja
  encoder->currentCLK = (temp & (1 << encoder->CLK)) >> encoder->CLK;
  uint8_t SW_state    = (temp & (1 << encoder->SW)) >> encoder->SW;

  delay(1);

  // jika ada pergantian state pada CLK, dan state tersebut adalah HIGH,
  if(encoder->currentCLK != encoder->lastCLK &&
     encoder->currentCLK == HIGH)
  {
    // baca encoder lagi untuk cek nilai pin DT
    temp = *(encoder->PIN_REG) & encoder->bit_mask;
    uint8_t DT_state = (temp & (1 << encoder->DT)) >> encoder->DT; // masking & shift

    // jika pin DT state != currentCLK, maka diputar ke kanan
    // jika pin DT state == currentCLK, maka diputar ke kiri
    if(DT_state != encoder->currentCLK)
    {
      (*counter)++;
    }
    else{
      (*counter)--;
    }
  }

  // cek penekanan button
  if(SW_state == 0)
  {
    delay(500);
    EncoderDitekan = true;
  }
  
  // simpan last state
  encoder->lastCLK = encoder->currentCLK;
  return EncoderDitekan;
}