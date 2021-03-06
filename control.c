#include "NU32.h"          // config bits, constants, funcs for startup and UART
// include other header files here
#include <stdio.h>
#include "encoder.h"
#include "isense.h"
#include "utilities.h"

#define BUF_SIZE 200
#define VOLTS_PER_COUNT (3.3/1024)
#define CORE_TICK_TIME 25    // nanoseconds between core ticks
#define SAMPLE_TIME 10       // 10 core timer ticks = 250 ns
#define DELAY_TICKS 20000000 // delay 1/2 sec, 20 M core ticks, between messages



#define PLOTPTS 200
#define DECIMATION 10



static volatile int ref[NUMSAMPS];
static volatile int StoringData = 0;
static volatile float kp=1.30, ki=0.06;
// for cubic
// static volatile float kpm=60, kim = 0.02 ,kdm =  200;
// for step
static volatile float kpm=27, kim = 0.01 ,kdm =  500;
static volatile float err = 0;
static volatile float errint = 0;
static volatile float angleerr = 0;
static volatile float angleerrint = 0, prevangle = 0;
static volatile float anglemotor = 0;
static volatile float angleuser = 0;
static volatile int test_done = 0;
static volatile float u = 0;
static volatile float unew = 0;
static volatile float um = 0;
static volatile float unewm = 0;
static volatile int i=0;
static volatile int adcval=0;
static volatile int counter = 0;
volatile float pwm = 0;
static volatile int trackcount =0, tracklength=0;
static volatile float trackanglemotor[1000], trackangleuser[1000];
static volatile float umtrack = 0, trackangleerr=0;





static volatile int plotind=0;
static volatile decctr=0;




typedef enum {IDLE, PWM,ITEST,HOLD,TRACK} mode;

static volatile mode MODE;

void setMODE(mode newMODE) {
  MODE = newMODE;
}

mode getMODE() {
  return MODE;
}

// isr for 5khz


void __ISR(_TIMER_4_VECTOR, IPL4SOFT) motorController(void)
{


  switch (getMODE()) {
    case HOLD:
    {
         // sample and convert pin 14
      anglemotor = (encoder_counts() - 32768) * 0.2;
      angleerr = anglemotor - angleuser;
      angleerrint += kim*angleerr;
      um = kpm*(angleerr) + angleerrint + kdm*(anglemotor - prevangle);
      if(um > 400.0){um = 400.0;}
      else if(um < -400){um = -400;}
      prevangle = anglemotor;
      break;
    }
    case TRACK:
    {
      if(trackcount<tracklength)
      {
        trackanglemotor[trackcount] = (encoder_counts() - 32768) * 0.2;
        trackangleerr = trackanglemotor[trackcount] - trackangleuser[trackcount];
        angleerrint += kim*trackangleerr;
        umtrack = kpm*(trackangleerr) + angleerrint + kdm*(trackanglemotor[trackcount] - prevangle);
        if(umtrack > 400.0){umtrack = 400.0;}
        else if(umtrack < -400){umtrack = -400;}
        prevangle = trackanglemotor[trackcount];
        trackcount++;
      }
      else
      {
        trackcount = 0;
        angleuser = (float)trackangleuser[tracklength-1];
        angleerrint = 0;
        prevangle = angleuser;
        setMODE(HOLD);
      }
      break;
    }

    default:
    {
      break;
    }
  }
    IFS0bits.T4IF = 0;                // clear CT int flag
  }

  void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void)
  {

        adcval =2* adc_sample_convert(7) - 1023;    // sample and convert pin 14

        switch (getMODE()) {
          case IDLE:
          {
           OC1RS =0;
           angleerr = 0;

           anglemotor = 0;
           err = 0;
           break;
         }

         case HOLD:
         {
          err = adcval - um;
          errint += err;
          u = kp*(err) + ki*(errint);
          unew = u;
          if(unew > 100.0){unew = 100.0;}
          else if(unew < -100){unew = -100;}
          pwm = unew;
          OC1RS = (unsigned int)abs(pwm) * 20;
          if(pwm<0){
          LATEbits.LATE0 = 1; // setting motor direction, phase
        }
        else {
          LATEbits.LATE0 = 0;
        }
        break;
      }

      case  ITEST:
      {
        err = - Waveform[counter] + adcval;
        errint += err;
        u = kp*(err) + ki*(errint);
        unew = u;
        if(unew > 100.0){unew = 100.0;}
        else if(unew < -100){unew = -100;}

        pwm = unew;

        OC1RS = (unsigned int)abs(pwm) * 20;
        if(pwm<0){
          LATEbits.LATE0 = 1; // setting motor direction, phase
        }
        else {
          LATEbits.LATE0 = 0;
        }

        if (counter < NUMSAMPS)
        {
          ref[counter] = adcval;
          counter++;
          
        }
        else
        {
          counter =0;
          setMODE(IDLE);
        }
        break;
      }

      case TRACK:
      {

        err = adcval - umtrack;
        errint += err;
        u = kp*(err) + ki*(errint);
        unew = u;


        if(unew > 100.0){unew = 100.0;}
        else if(unew < -100){unew = -100;}

        pwm = unew;
        OC1RS = (unsigned int)abs(pwm) * 20;
        if(pwm<0){
        LATEbits.LATE0 = 1; // setting motor direction, phase
      }
      else {
        LATEbits.LATE0 = 0;
      }
      break;
    }
    default:
    {
     break;
   }
 }
    IFS0bits.T2IF = 0;                // clear CT int flag
  }




  int main()
  {
    int j;
    float kptemp=0, kitemp=0;
    float kpmtemp = 0, kdmtemp = 0, kimtemp=0;
    unsigned int a = 0;
    char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  encoder_init();
  adc_init();
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;


  __builtin_disable_interrupts();
  T2CONbits.TCKPS = 0;     // Timer2 prescaler 3 1:4 5kHz
  PR2 = 15999;
  TMR2 = 0;


  IPC2bits.T2IP = 5;                // step 4: interrupt priority
  IFS0bits.T2IF = 0;                // step 5: clear CT interrupt flag
  IEC0bits.T2IE = 1;                // step 6: enable core timer interrupt
  T2CONbits.ON = 1;        // turn on Timer2



// 200hz timer
  T4CONbits.TCKPS = 4;
  TMR4 = 0;
  PR4 = 24999;
  IEC0bits.T4IE = 1;
  IFS0bits.T4IF = 0;
  IPC4bits.T4IP = 4;
  T4CONbits.ON =1;
  __builtin_enable_interrupts();



  OC1CONbits.OCTSEL = 1;   //enable timer 3   1:1  20kHz
  T3CONbits.TCKPS = 1;     // Timer3 prescaler N=4 (1:4)
  PR3 = 1999;              // period = (PR2+1) * N * 12.5 ns = 100 us, 10 kHz
  TMR3 = 0;                // initial TMR2 count is 0
  OC1CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
  OC1RS = 500;             // duty cycle = OC1RS/(PR2+1) = 25%
  OC1R = 500;              // initialize before turning OC1 on; afterward it is read-only
  T3CONbits.ON = 1;        // turn on Timer3
  OC1CONbits.ON = 1;       // turn on OC1
  TRISEbits.TRISE0 = 0; //configure pin as output
  LATEbits.LATE0 = 0;  // set default value, phase pin
  TRISEbits.TRISE1 = 0; //motor enable to pin E1





  int deg;
  int temp;
  OC1RS =0;

  makeWaveform();

  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a':
      {
        a = adc_sample_convert(7);
        sprintf(buffer,"%d\r\n", a);
        NU32_WriteUART3(buffer);
        break;
      }
      case 'b':
      {
        a = adc_sample_convert(7);
        temp = 2*a - 1023;
        sprintf(buffer,"%d\r\n", temp);
        NU32_WriteUART3(buffer);
        break;
      }
      case 'c':
      {
        sprintf(buffer,"%d\r\n", encoder_counts());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'd':
      {
        deg = (encoder_counts() - 32768) * 0.2;
        sprintf(buffer,"%d\r\n", deg);
        NU32_WriteUART3(buffer); // send encoder count to client
        break;
      }
      case 'e':
      {
        sprintf(buffer,"%d\r\n", encoder_reset());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'f':
      {
        setMODE(PWM);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer,"%f", &pwm);
        OC1RS = (unsigned int)abs(pwm) * 20;
        if(pwm<0){
          LATEbits.LATE0 = 0;
        }
        else {
          LATEbits.LATE0 = 1;
        }
        sprintf(buffer,"%f\r\n",pwm);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'g':
      {
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f %f", &kptemp, &kitemp);
        __builtin_disable_interrupts();
        errint=0;
        kp=kptemp;
        ki=kitemp;
        __builtin_enable_interrupts();
        break;
      }
      case 'h':
      {
        sprintf(buffer, "%f %f\r\n", kp, ki);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'i':
      {
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f %f %f", &kpmtemp,&kdmtemp, &kimtemp);
        __builtin_disable_interrupts();
        errint=0;
        kpm=kpmtemp;
        kim=kimtemp;
        kdm = kdmtemp;
        __builtin_enable_interrupts();
        break;
      } 

      case 'j':
      {
        sprintf(buffer, "%f %f %f\r\n", kpm, kdm, kim);
        NU32_WriteUART3(buffer);
        break;
      }


      case 'k':
      {
        counter = 0;
        setMODE(ITEST);
        sprintf(buffer,"%d\n",NUMSAMPS);
        NU32_WriteUART3(buffer);
        while (counter < 99){;}
        for (j = 0; j < NUMSAMPS; j++)
        {
          sprintf(buffer, "%d %d\n",Waveform[j],ref[j]);
          NU32_WriteUART3(buffer);
        }
        break;
      }

      case 'l':
      {
        __builtin_disable_interrupts();
        NU32_ReadUART3(buffer, BUF_SIZE);
        sscanf(buffer, "%f", &angleuser);
        angleerrint = 0;
        setMODE(HOLD);
        __builtin_enable_interrupts();
        break;
      }
      case 'm':
      {

        NU32_ReadUART3(buffer, BUF_SIZE);
        sscanf(buffer,"%d",&tracklength);
        for(j=0;j<tracklength;j++)
        {
          trackangleuser[j]= 0;
          trackanglemotor[j]=0;
          
        }
        for(j=0;j<tracklength;j++)
        {
          NU32_ReadUART3(buffer, BUF_SIZE);
          sscanf(buffer,"%f",&trackangleuser[j]);
        }

        break;
      }

      case 'n':
      {

        NU32_ReadUART3(buffer, BUF_SIZE);
        sscanf(buffer,"%d",&tracklength);
        for(j=0;j<tracklength;j++)
        {
          trackangleuser[j]= 0;
          trackanglemotor[j]=0;
          
        }
        for(j=0;j<tracklength;j++)
        {
          NU32_ReadUART3(buffer, BUF_SIZE);
          sscanf(buffer,"%f",&trackangleuser[j]);
        }

        break;
      }

      case 'o':
      {
        angleerrint = 0;
        errint=0;
        trackcount =0;
        setMODE(TRACK);
        while(getMODE()==4){;}
        sprintf(buffer,"%d\n",tracklength);
        NU32_WriteUART3(buffer);
        for(j=0;j<tracklength;j++)
        {
          sprintf(buffer, "%f %f\r\n", trackangleuser[j] , trackanglemotor[j]);
          NU32_WriteUART3(buffer);
        }
        break;
      }

      case 'p':
      {
        setMODE(IDLE);
        LATEbits.LATE0 = 0;
        OC1RS = 0;
        sprintf(buffer,"%d\r\n",OC1RS);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'q':
      {
        setMODE(IDLE);
        break;
      }
      case 'r':
      {
        sprintf(buffer, "%d\r\n", getMODE());
        NU32_WriteUART3(buffer);
        break;
      }

      default:
      {
        NU32_LED2 = 0;  // turn on LED2 to indicate an error
        break;
      }
    }
  }
  return 0;
}