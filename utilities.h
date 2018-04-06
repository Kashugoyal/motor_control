#ifndef UTILITIES_H
#define UTILITIES_H

void makeWaveform(){
  int i = 0, center = 0, A = 200;
  for (i = 0; i < NUMSAMPS; ++i)
  {
    if (i<NUMSAMPS/4)
    {
      Waveform[i]=center+A;
    }
    else if(i<NUMSAMPS/2 && i>NUMSAMPS/4)
    {
      Waveform[i]=center-A;
    }
    else if(i<3*NUMSAMPS/4 && i>NUMSAMPS/2)
    {
      Waveform[i]=center+A;
    }
    else
    {
      Waveform[i]=center-A;
    }
  }
}

#endif