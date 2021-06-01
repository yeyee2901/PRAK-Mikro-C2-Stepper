// LIBRARIES ---------------------------------------
#include <Arduino.h>
#include <driver_LCD16x2.hpp>
#include <RotaryEncoder.hpp>


// MACROS ------------------------------------------
#ifndef TRUE
  #define TRUE  1
  #define FALSE 0
#endif

// status codes
#define CW            1
#define CCW           0
#define HALF_STEPPING 1
#define FULL_STEPPING 0



// GLOBAL VARS -------------------------------------
// pembuatan struct, namun yang digunakan nantinya hanyalah
// address nya saja (pass by reference)
RotaryEncoder Encoder;

// variabel global untuk penampungan setting user
// pilihan speed ada 3, dapat diubah sesuai keperluan
const int SPEED_CHOICES = 5;
const int SPEEDS[SPEED_CHOICES] = {70, 50, 30, 10, 5};

// jumlah step max
const int HALF_STEP_LIMIT = 400;
const int FULL_STEP_LIMIT = 200;

// array untuk menampung kombinasi stepping
const uint8_t CCW_halfstep[8] = 
{
  0b0001, 0b0011,
  0b0010, 0b0110,
  0b0100, 0b1100,
  0b1000, 0b1001
};

const int CW_halfstep[8] = 
{
  0b1000, 0b1100,
  0b0100, 0b0110,
  0b0010, 0b0011,
  0b0001, 0b1001
};

const uint8_t CCW_fullstep[4] = 
{
  0b0001, 0b0010,
  0b0100, 0b1000
};

const uint8_t CW_fullstep[4] = 
{
  0b1000, 0b0100,
  0b0010, 0b0001
};

int modeStep, numStep, speedStep, arahStep;
char sprintf_buffer[10];


// FUNCTION PROTOTYPE ------------------------------

void menu_pilihanMode(void);
void menu_pilihanStep(void);
void menu_pilihanSpeed(void);
void menu_pilihanArah(void);
bool menu_konfirmasi(void);
void runStepper(int mode, int steps, int speed, int arah);




// MAIN PROGRAM ------------------------------------

void setup() 
{

  // init LCD
  lcd_init(&PORTC, &PORTD, &DDRC, &DDRD, PD0, PD1);
  lcd_command(CLEAR_DISPLAY);

  // init Encoder
  // menggunakan PORTB
  initEncoder(&Encoder, &PORTB, &DDRB, &PINB);

  // init PORT output untuk stepper motor
  // PA0 - PA3
  DDRA  = 0x0F;
  PORTA = 0x00;

  lcd_setpos(0,0);
  lcd_string(" STEPPER MOTOR");
  lcd_setpos(1,0);
  lcd_string("CONTROLLER -Y2");
  
  delay(4000);
}

void loop() 
{
  //set nilai default
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0); lcd_string(" [CLEARING]:");
  lcd_setpos(1,0); lcd_string("all variables..");
  modeStep  = HALF_STEPPING;
  numStep   = 0;
  speedStep = 0;
  arahStep  = CW;
  delay(2000);
  menu_pilihanMode();
  menu_pilihanStep();
  menu_pilihanSpeed();
  menu_pilihanArah();

  bool konfirmasi = menu_konfirmasi();
  if(konfirmasi == true) runStepper(modeStep, numStep, SPEEDS[speedStep], arahStep);

  delay(2000);
}



/*!
  @brief Menu untuk pemilihan mode stepper motor. Pilihan ada 2:
  "HalfStepping" artinya 1 putaran = 400 step , step angle = 0.9 --
  "FullStepping" artinya 1 putaran = 200 step , step angle = 1.8
  Tekan encoder untuk lanjut ke menu selanjutnya
  @param . None
  @return None
*/
void menu_pilihanMode()
{
  bool encoderDitekan = false;
  int lastMode;
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [MENU] Mode:");

  // selama encoder belum ditekan, maka baca encoder secara
  // terus menerus
  while(!encoderDitekan)
  {
    encoderDitekan = readEncoder(&Encoder, &modeStep);
    if(modeStep > 1) modeStep = 1;
    if(modeStep < 0) modeStep = 0;

    // hanya update LCD ketika ada perubahan nilai
    if(modeStep != lastMode)
    {
      lcd_setpos(1,0);
      lcd_string("              ");

      if(modeStep == 1)
      {
        lcd_setpos(1,0);
        lcd_string("Half stepping");
        modeStep = HALF_STEPPING;
      }
      else{
        lcd_setpos(1,0);
        lcd_string("Full stepping");          
        modeStep = FULL_STEPPING;
      }
    }

    lastMode = modeStep;
  }


  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [OK] Mode:");
  lcd_setpos(1,0);
  if(modeStep == HALF_STEPPING) lcd_string("Half stepping");
  else                          lcd_string("Full stepping");
  delay(2000);
}

/*!
  @brief Menu untuk pemilihan jumlah step. Untuk half-stepping range
  step adalah 0-400, sedangkan untuk full-stepping range step adalah 0-200.
  Tekan encoder untuk lanjut ke menu selanjutnya
  @param . None
  @return None
*/
void menu_pilihanStep()
{
  bool encoderDitekan = false;
  int lastNumStep, stepLimit;
  if(modeStep == HALF_STEPPING) stepLimit = HALF_STEP_LIMIT;
  else                          stepLimit = FULL_STEP_LIMIT;
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [MENU] Step:");

  // selama encoder belum ditekan, maka belum keluar menu
  while(!encoderDitekan)
  {
    encoderDitekan = readEncoder(&Encoder, &numStep);
    if(numStep <  0 )       numStep = 0;
    if(numStep > stepLimit) numStep = stepLimit;

    if(numStep != lastNumStep)
    {
      lcd_setpos(1,0);
      lcd_string("              ");

      sprintf(sprintf_buffer, "%d", numStep);
      lcd_setpos(1,0);
      lcd_string(sprintf_buffer);
    }

    lastNumStep = numStep;
  }
  
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [OK] Step:");
  lcd_string(sprintf_buffer);
  delay(2000);
}

/*!
  @brief Menu untuk pengaturan speed. Speed yang dimaksudkan disini adalah
  untuk pengaturan delay antar step. Pilihan yang tersedia: 70ms, 50ms, 30ms,
  10ms, dan yang paling cepat 5ms.
  @param . None
  @return None
*/
void menu_pilihanSpeed()
{
  bool encoderDitekan = false;
  int lastSpeed;
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [MENU] Speed:");

  // selama encoder belum ditekan, maka belum keluar menu
  while(!encoderDitekan)
  {
    encoderDitekan = readEncoder(&Encoder, &speedStep);
    if(speedStep < 0) speedStep = 0;
    if(speedStep >= SPEED_CHOICES) speedStep = SPEED_CHOICES - 1;

    // hanya update LCD ketika ada perubahan nilai
    if(speedStep != lastSpeed)
    {
      lcd_setpos(1,0);
      lcd_string("              ");

      sprintf(sprintf_buffer, "%dms", SPEEDS[speedStep]);
      lcd_setpos(1,0);
      lcd_string(sprintf_buffer);
    }

    lastSpeed = speedStep;
  }
  
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [OK] Speed:");
  lcd_string(sprintf_buffer);
  delay(2000);
}

/*!
  @brief Menu untuk pengaturan arah putar. Pilihan hanya 2, clockwise atau
  counter-clockwise.
  @param . None
  @return None
*/
void menu_pilihanArah()
{
  bool encoderDitekan = false;
  int last_dir_selection, dir_selection = 0;
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [MENU] Arah:");

  while(!encoderDitekan)
  {
    encoderDitekan = readEncoder(&Encoder, &dir_selection);
    if(dir_selection > 1) dir_selection = 1;
    if(dir_selection < 0) dir_selection = 0;

    // hanya update LCD ketika ada perubahan nilai
    if(dir_selection != last_dir_selection)
    {
      lcd_setpos(1,0);
      lcd_string("              ");

      if(dir_selection == 1)
      {
        lcd_setpos(1,0);
        lcd_string("CW");
        arahStep = CW;
      }
      else{
        lcd_setpos(1,0);
        lcd_string("CCW");
        arahStep = CCW;
      }
    }

    last_dir_selection = dir_selection;
  }
  
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [OK] Arah:");
  if(arahStep == CW)  lcd_string("CW");
  else                lcd_string("CCW");

  delay(2000);
}

/*!
  @brief Menu untuk menanyakan konfirmasi user ingin menjalankan stepper
  sesuai setting yang diatur
  @param . None
  @return TRUE jika user memilih yes, FALSE jika memilih no.
*/
bool menu_konfirmasi()
{
  bool encoderDitekan = false;
  int last_choice_selection, choice_selection = 0;
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" Execute?");

  while(!encoderDitekan)
  {
    encoderDitekan = readEncoder(&Encoder, &choice_selection);
    if(choice_selection > 1) choice_selection = 1;
    if(choice_selection < 0) choice_selection = 0;

    // hanya update LCD ketika ada perubahan nilai
    if(choice_selection != last_choice_selection)
    {
      lcd_setpos(1,0);
      lcd_string("              ");

      if(choice_selection == 1)
      {
        lcd_setpos(1,0);
        lcd_string("YES");
      }
      else{
        lcd_setpos(1,0);
        lcd_string("NO");
      }
    }

    last_choice_selection = choice_selection;
  }
  
  return choice_selection;
}


/*!
  @brief Menjalankan stepper motor sesuai setting yang dimasukkan sebagai
  argumen input.
  @param mode  Mode menjalankan stepper, bisa full-step / half-step
  @param steps Jumlah step yang akan ditempuh stepper motor
  @param speed seberapa cepat motor berputar, masukkan delay dalam ms kesini
  @param arah  arah putar stepper motor
  @return None
*/
void runStepper(int mode, int steps, int speed, int arah)
{
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [RUN]");
  lcd_setpos(1,0);
  lcd_string("Please wait...");
  delay(3000);

  // run stepper motor sesuai setting
  if(mode == HALF_STEPPING) // HALF STEPPING MODE
  {
    while(steps > 0)
    {
      if(arah == CW)
      {
        for(int n = 0; n < 8; n++)
        {
          PORTA = CW_halfstep[n];
          delay(speed);
          steps--;
          if(steps <= 0) break;
        }
      }
      else{
        for(int n = 0; n < 8; n++)
        {
          PORTA = CCW_halfstep[n];
          delay(speed);
          steps--;
          if(steps <= 0) break;
        }
      }

    }
  }
  else{  // FULL STEPPING MODE
    while(steps > 0)
    {
      if(arah == CW)
      {
        for(int n = 0; n < 4; n++)
        {
          PORTA = CW_fullstep[n];
          delay(speed);
          steps--;
          if(steps <= 0) break;
        }
      }
      else{
        for(int n = 0; n < 4; n++)
        {
          PORTA = CCW_fullstep[n];
          delay(speed);
          steps--;
          if(steps <= 0) break;
        }
      }

    }
  }

  PORTA = 0;
  lcd_command(CLEAR_DISPLAY);
  lcd_setpos(0,0);
  lcd_string(" [DONE]");
}