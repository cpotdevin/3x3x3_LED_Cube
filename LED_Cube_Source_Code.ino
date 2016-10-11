#include <SPI.h>

int dataPin = 13;//data
int clockPin = 11;//clock
int latchPin = 8;//latch
int statePin = 3;//state
int animationPin = 2;//animation playing

volatile int state = 0;
volatile boolean animation = false;
volatile boolean changeOccurred = false;

int plane[3][3] = {{0,0,0},{0,0,0},{0,0,0}};//disco plane

int rainDropPosition[3][3] = {{4,4,4},{4,4,4},{4,4,4}};//rain drop positions

int fireFlamePosition[3][3] = {{-1,-1,-1},{-1,-1,-1},{-1,-1,-1}};//fire flame positions
int flameIntensity[3][3] = {{0,0,0},{0,0,0},{0,0,0}};//flame intensities

int cubeLocation = 0;

int beeLocation[3] ={0,0,0};
int beeLocation2[3] ={0,0,0};

int shift_out;

byte plane0[6], plane1[6], plane2[6], plane3[6];

int level = 0;
int anodeLevel = 0;
int BAM_Bit, BAM_Counter = 0;

unsigned long start;

void setup()
{
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  noInterrupts();
  
  TCCR1A = B00000000;
  TCCR1B = B00001011;
  
  TIMSK1 = B00000010;
  OCR1A = 30;
  
  plane0[0] = B00000000;
  plane0[2] = B00000000;
  plane0[4] = B00000000;
  
  plane1[0] = B00000000;
  plane1[2] = B00000000;
  plane1[4] = B00000000;
  
  plane2[0] = B00000000;
  plane2[2] = B00000000;
  plane2[4] = B00000000;
  
  plane3[0] = B00000000;
  plane3[2] = B00000000;
  plane3[4] = B00000000;
  
  plane0[1] = B00000010;
  plane0[3] = B00000100;
  plane0[5] = B00001000;
  
  plane1[1] = B00000010;
  plane1[3] = B00000100;
  plane1[5] = B00001000;
  
  plane2[1] = B00000010;
  plane2[3] = B00000100;
  plane2[5] = B00001000;
  
  plane3[1] = B00000010;
  plane3[3] = B00000100;
  plane3[5] = B00001000;
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  SPI.begin();
  
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(latchPin, 1);
  digitalWrite(latchPin, 0);
  
  attachInterrupt(0, changeAnimation, RISING);
  attachInterrupt(1, animationState, CHANGE);
  
  if(digitalRead(statePin) == HIGH)
  {
    animation = true;
  }
  
  Serial.begin(9600);
  
  randomSeed(analogRead(0));
  
  interrupts();
}

//++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++
//-----------------------------------------------------------------
//Void loop
//-----------------------------------------------------------------
//++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++

void loop()
{
  if(animation)
  {
    allOn();
  }else {
    playAnimations();
  }
}

void LED(int level, int row, int column, byte light)
{
  if(level < 0 || level > 2)
  {
    return;
  }
  if(row < 0 || row > 2)
  {
    return;
  }
  if(column < 0 || column > 2)
  {
    return;
  }
  if(light < 0)
  {
    light = 0;
  }
  if(light > 15)
  {
    light = 15;
  }
  
  int whichbyte = row*3 + column;
  
  if(whichbyte == 8)
  {
    switch(level)
    {
      case 0:
        bitWrite(plane0[1], 0, bitRead(light, 0));
        bitWrite(plane1[1], 0, bitRead(light, 1));
        bitWrite(plane2[1], 0, bitRead(light, 2));
        bitWrite(plane3[1], 0, bitRead(light, 3));
        break;
      case 1:
        bitWrite(plane0[3], 0, bitRead(light, 0));
        bitWrite(plane1[3], 0, bitRead(light, 1));
        bitWrite(plane2[3], 0, bitRead(light, 2));
        bitWrite(plane3[3], 0, bitRead(light, 3));
        break;
      case 2:
        bitWrite(plane0[5], 0, bitRead(light, 0));
        bitWrite(plane1[5], 0, bitRead(light, 1));
        bitWrite(plane2[5], 0, bitRead(light, 2));
        bitWrite(plane3[5], 0, bitRead(light, 3));
        break;
    }
  }
  else
  {
    switch(level)
    {
      case 0:
        bitWrite(plane0[0], whichbyte, bitRead(light, 0));
        bitWrite(plane1[0], whichbyte, bitRead(light, 1));
        bitWrite(plane2[0], whichbyte, bitRead(light, 2));
        bitWrite(plane3[0], whichbyte, bitRead(light, 3));
        break;
      case 1:
        bitWrite(plane0[2], whichbyte, bitRead(light, 0));
        bitWrite(plane1[2], whichbyte, bitRead(light, 1));
        bitWrite(plane2[2], whichbyte, bitRead(light, 2));
        bitWrite(plane3[2], whichbyte, bitRead(light, 3));
        break;
      case 2:
        bitWrite(plane0[4], whichbyte, bitRead(light, 0));
        bitWrite(plane1[4], whichbyte, bitRead(light, 1));
        bitWrite(plane2[4], whichbyte, bitRead(light, 2));
        bitWrite(plane3[4], whichbyte, bitRead(light, 3));
        break;
    }
  }
}

ISR(TIMER1_COMPA_vect)
{
  if(BAM_Counter == 3)
  {
    BAM_Bit++;
  }
  if(BAM_Counter == 9)
  {
    BAM_Bit++;
  }
  if(BAM_Counter == 21)
  {
    BAM_Bit++;
  }
  
  BAM_Counter++;
  
  SPI.transfer(0);
  SPI.transfer(0);
  
  digitalWrite(latchPin, 1);
  digitalWrite(latchPin, 0);
  
  switch(BAM_Bit)
  {
    case 0:
      SPI.transfer(plane0[1 + level]);
      SPI.transfer(plane0[0 + level]);
      break;
    case 1:
      SPI.transfer(plane1[1 + level]);
      SPI.transfer(plane1[0 + level]);
      break;
    case 2:
      SPI.transfer(plane2[1 + level]);
      SPI.transfer(plane2[0 + level]);
      break;
    case 3:
      SPI.transfer(plane3[1 + level]);
      SPI.transfer(plane3[0 + level]);
      if(BAM_Counter == 45)
      {
        BAM_Counter = 0;
        BAM_Bit = 0;
      }
      break;
  }
  
  digitalWrite(latchPin, 1);
  digitalWrite(latchPin, 0);
  
  level +=2;
  
  if(level > 4)
  {
    level = 0;
  }
}

//++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Animations
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=

void allOn()
{
  int z, x, y;
  {
    for( z = 0; z < 3; z++)
    {
      for( x = 0; x < 3; x++)
      {
        for( y = 0; y < 3; y++)
        {
          LED( z, x, y, 15);
        }
      }
    }
  }
}

void allOff()
{
  int z, x, y;
  
  for( z = 0; z < 3; z++)
  {
    for( x = 0; x < 3; x++)
    { 
      for( y = 0; y < 3; y++)
      {
        LED( z, x, y, 0);
      }
    }
  }
}

void playAnimations()
{
  switch (state) {
    case 0:
      sineWave1();
      sineWave3();
      disco();
      tornado();
      rain();
      fire();
      cube();
      bee();
      break;
    case 1:
      sineWave1();
      break;
    case 2:
      sineWave3();
      break;
    case 3:
      disco();
      break;
    case 4:
      tornado();
      break;
    case 5:
      rain();
      break;
    case 6:
      fire();
      break;
    case 7:
      cube();
      break;
    case 8:
      bee();
      break;
  }
  allOff();
  changeOccurred = false;
}

void sineWave1()//animation 1 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  int z, x, y;
  int zM = 1;
  int frameTime = 300;
  
  start = millis();
  while(millis()-start < 10000)
  {
    zM = 1;
    
    for( z = 0; z < 3; z++)
    {
      for( x = 0; x < 3; x++)
      {
        for( y = 0; y < 3; y++)
        {
          if(y == 0)
          {
            LED( z, x, y, 15);
          }
          if(y == 2)
          {
            LED( 2-z, x, y, 15);
          }
          LED( abs(zM-z), 0, 1, 15);
          LED( abs(zM-z), 1, 1, 15);
          LED( abs(zM-z), 2, 1, 15);
        }
      }
      delay(frameTime);
      allOff();
      if(changeOccurred)
      {
        return;
      }
    }
    for( x = 0; x < 3; x++)
    {
      for( y = 0; y < 3; y++)
      {
        if(y == 0)
        {
          LED( 1, x, y, 15);
        }
        if(y == 2)
        {
          LED( 1, x, y, 15);
        }
        LED( abs(zM-z), 0, 1, 15);
        LED( abs(zM-z), 1, 1, 15);
        LED( abs(zM-z), 2, 1, 15);
      }
    }
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}
/*
void sineWave2()
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int zM = 1;
  int frameTime = 300;
  
  start = millis();
  while(millis()-start < 10000)
  {
    zM = 1;
    
    for( z = 0; z < 3; z++)
    {
      for( x = 0; x < 3; x++)
      {
        for( y = 0; y < 3; y++)
        {
          if( (x == 0 && y == 2) || (x == 2 && y == 0) )
          {
            LED( z, x, y, 15);
          }
          if( (x == 0 && y == 0) || (x == 2 && y == 2) )
          {
            LED( 2-z, x, y, 15);
          }
          LED( abs(zM-z), 1, 1, 15);
        }
      }
      delay(frameTime);
      allOff();
      if(changeOccurred)
      {
          return;
      }
    }
    for( x = 0; x < 3; x++)
    {
      for( y = 0; y < 3; y++)
      {
        if( (x == 0 && y == 2) || (x == 2 && y == 0) )
        {
          LED( 1, x, y, 15);
        }
        if( (x == 0 && y == 0) || (x == 2 && y == 2) )
        {
          LED( 1, x, y, 15);
        }
        LED( 2, 1, 1, 15);
      }
    }
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
        return;
    }
  }
}
*/
void sineWave3()//animation 2 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 300;
  
  start = millis();
  while(millis()-start < 10000)
  {
    int zM = 1;
    for(z = 0; z < 4; z++)
    {
      for(x = 0; x < 3; x++)
      {
        for(y = 0; y < 3; y++)
        {
          if(x != 1 && y != 1)
          {
            if(z == 3)
            {
              LED( 1, x, y, 15);
            }else {
              LED( z, x, y, 15);
            }
          }else {
            if(x != 1 || y != 1)
            {
              LED( abs(2-abs(1-z)), x, y, 15);
            }
          }
          if(z == 3)
          {
            LED( 1, 1, 1, 15);
          }else {
            LED( abs(2-z), 1, 1, 15);
          }
        }
      }
      delay(frameTime);
      allOff();
      if(changeOccurred)
      {
          return;
      }
    }
  }
}

void disco()//animation 3 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 300;
  
  start = millis();
  while(millis()-start < 10000)
  {
    int numOfColumnsToChange = random(0, 2);
    for(int i = 0; i <= numOfColumnsToChange; i++)
    {
      int randomX = random(0, 3);
      int randomY = random(0, 3);
      int randomChange = random( 0, 2);
      if((randomChange == 0 && plane[randomX][randomY] != -1) || (randomChange == 1 && plane[randomX][randomY] != 2))
      {
        if(randomChange == 0)
        {
          plane[randomX][randomY]--;
        }else {
          plane[randomX][randomY]++;
        }
      }
    }
    
    for(z = 0; z < 3; z++)
    {
      for(x = 0; x < 3; x++)
      {
        for(y = 0; y < 3; y++)
        {
          if(plane[x][y] >= z)
          {
            LED( z, x, y, 15);
          }else {
            LED( z, x, y, 0);
          }
        }
      }
    }
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}

void tornado()//animation 4 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 300;
  
  start = millis();
  while(millis()-start < 10000)
  {
    LED( 0, 0, 0, 15);
    LED( 0, 2, 2, 15);
    
    LED( 1, 1, 0, 15);
    LED( 1, 1, 2, 15);
    
    LED( 2, 2, 0, 15);
    LED( 2, 0, 2, 15);
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
    
    LED( 0, 0, 1, 15);
    LED( 0, 2, 1, 15);
    
    LED( 1, 0, 0, 15);
    LED( 1, 2, 2, 15);
    
    LED( 2, 1, 0, 15);
    LED( 2, 1, 2, 15);
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
    
    LED( 0, 0, 2, 15);
    LED( 0, 2, 0, 15);
    
    LED( 1, 0, 1, 15);
    LED( 1, 2, 1, 15);
    
    LED( 2, 0, 0, 15);
    LED( 2, 2, 2, 15);
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
    
    LED( 0, 1, 2, 15);
    LED( 0, 1, 0, 15);
    
    LED( 1, 0, 2, 15);
    LED( 1, 2, 0, 15);
    
    LED( 2, 0, 1, 15);
    LED( 2, 2, 1, 15);
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}

void rain()//animation 5 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 100;
  
  start = millis();
  while(millis()-start < 10000)
  {
    int numOfColumnsToChange = random(0, 2);
    for(int i = 0; i <= numOfColumnsToChange; i++)
    {
      int randomX = random(0, 3);
      int randomY = random(0, 3);
      int randomNum = random( 0, 6);
      if(rainDropPosition[randomX][randomY] == 4 && randomNum == 0)
      {
        rainDropPosition[randomX][randomY]--;
      }
    }
    
    for(x = 0; x < 3; x++)
    {
      for(y = 0; y < 3; y++)
      {
        if(rainDropPosition[x][y] < 4)
        {
          rainDropPosition[x][y]--;
        }
        if(-1 < rainDropPosition[x][y] && rainDropPosition[x][y] < 3)
        {
          LED( rainDropPosition[x][y], x, y, 15);
        }else if(rainDropPosition[x][y] == -1)
        {
          LED( 0, x, y, 8);
        }else if(rainDropPosition[x][y] == -2)
        {
          LED( 0, x, y, 3);
        }else if(rainDropPosition[x][y] == -3)
        {
          rainDropPosition[x][y] = 4;
        }
      }
    }
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}

void fire()//animation 6 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 140;
  
  start = millis();
  while(millis()-start < 10000)
  {
    int numOfColumnsToChange = random(0, 2);
    for(int i = 0; i <= numOfColumnsToChange; i++)
    {
      int randomX = random(0, 3);
      int randomY = random(0, 3);
      int randomNum = random( 0, 2);
      if(fireFlamePosition[randomX][randomY] == -1 && randomNum == 0)
      {
        fireFlamePosition[randomX][randomY]++;
        flameIntensity[randomX][randomY] = random(12, 16);
      }
    }
    
    for(x = 0; x < 3; x++)
    {
      for(y = 0; y < 3; y++)
      {
        int randomIntensity = random(4, 16);
        LED( 0, x, y, randomIntensity);
      }
    }
    
    for(x = 0; x < 3; x++)
    {
      for(y = 0; y < 3; y++)
      {
        if(fireFlamePosition[x][y] > -1)
        {
          LED( fireFlamePosition[x][y], x, y, flameIntensity[x][y]);
          flameIntensity[x][y] /= 1.81;
          if(random(0, 2) == 0)
          {
            fireFlamePosition[x][y]++;
          }
        }
        if(fireFlamePosition[x][y] == 3)
        {
          fireFlamePosition[x][y] = -1;
        }
      }
    }
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}

void cube()//animation 7 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 200;
  
  start = millis();
  while(millis()-start < 10000)
  {
    
    int i = random(1,4);
    switch (cubeLocation) {
      case 0:
        LED( 0, 0, 0, 15);
        LED( 0, 1, 0, 15);
        LED( 0, 0, 1, 15);
        LED( 0, 1, 1, 15);
        LED( 1, 0, 0, 15);
        LED( 1, 1, 0, 15);
        LED( 1, 0, 1, 15);
        LED( 1, 1, 1, 15);
        if(i == 1)
        {
          cubeLocation = 1;
        }else if(i == 2)
        {
          cubeLocation = 2;
        }else if(i == 3)
        {
          cubeLocation = 4;
        }
        break;
      case 1:
        LED( 0, 0, 1, 15);
        LED( 0, 1, 1, 15);
        LED( 0, 0, 2, 15);
        LED( 0, 1, 2, 15);
        LED( 1, 0, 1, 15);
        LED( 1, 1, 1, 15);
        LED( 1, 0, 2, 15);
        LED( 1, 1, 2, 15);
        if(i == 1)
        {
          cubeLocation = 0;
        }else if(i == 2)
        {
          cubeLocation = 3;
        }else if(i == 3)
        {
          cubeLocation = 5;
        }
        break;
      case 2:
        LED( 0, 1, 0, 15);
        LED( 0, 2, 0, 15);
        LED( 0, 1, 1, 15);
        LED( 0, 2, 1, 15);
        LED( 1, 1, 0, 15);
        LED( 1, 2, 0, 15);
        LED( 1, 1, 1, 15);
        LED( 1, 2, 1, 15);
        if(i == 1)
        {
          cubeLocation = 0;
        }else if(i == 2)
        {
          cubeLocation = 3;
        }else if(i == 3)
        {
          cubeLocation = 6;
        }
        break;
      case 3:
        LED( 0, 1, 1, 15);
        LED( 0, 2, 1, 15);
        LED( 0, 1, 2, 15);
        LED( 0, 2, 2, 15);
        LED( 1, 1, 1, 15);
        LED( 1, 2, 1, 15);
        LED( 1, 1, 2, 15);
        LED( 1, 2, 2, 15);
        if(i == 1)
        {
          cubeLocation = 1;
        }else if(i == 2)
        {
          cubeLocation = 2;
        }else if(i == 3)
        {
          cubeLocation = 7;
        }
        break;
      case 4:
        LED( 1, 0, 0, 15);
        LED( 1, 1, 0, 15);
        LED( 1, 0, 1, 15);
        LED( 1, 1, 1, 15);
        LED( 2, 0, 0, 15);
        LED( 2, 1, 0, 15);
        LED( 2, 0, 1, 15);
        LED( 2, 1, 1, 15);
        if(i == 1)
        {
          cubeLocation = 5;
        }else if(i == 2)
        {
          cubeLocation = 6;
        }else if(i == 3)
        {
          cubeLocation = 0;
        }
        break;
      case 5:
        LED( 1, 0, 1, 15);
        LED( 1, 1, 1, 15);
        LED( 1, 0, 2, 15);
        LED( 1, 1, 2, 15);
        LED( 2, 0, 1, 15);
        LED( 2, 1, 1, 15);
        LED( 2, 0, 2, 15);
        LED( 2, 1, 2, 15);
        if(i == 1)
        {
          cubeLocation = 4;
        }else if(i == 2)
        {
          cubeLocation = 7;
        }else if(i == 3)
        {
          cubeLocation = 2;
        }
        break;
      case 6:
        LED( 1, 1, 0, 15);
        LED( 1, 2, 0, 15);
        LED( 1, 1, 1, 15);
        LED( 1, 2, 1, 15);
        LED( 2, 1, 0, 15);
        LED( 2, 2, 0, 15);
        LED( 2, 1, 1, 15);
        LED( 2, 2, 1, 15);
        if(i == 1)
        {
          cubeLocation = 4;
        }else if(i == 2)
        {
          cubeLocation = 7;
        }else if(i == 3)
        {
          cubeLocation = 2;
        }
        break;
      case 7:
        LED( 1, 1, 1, 15);
        LED( 1, 2, 1, 15);
        LED( 1, 1, 2, 15);
        LED( 1, 2, 2, 15);
        LED( 2, 1, 1, 15);
        LED( 2, 2, 1, 15);
        LED( 2, 1, 2, 15);
        LED( 2, 2, 2, 15);
        if(i == 1)
        {
          cubeLocation = 5;
        }else if(i == 2)
        {
          cubeLocation = 6;
        }else if(i == 3)
        {
          cubeLocation = 3;
        }
        break;
    }
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}

void bee()//animation 8 ++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=++=
{
  if(changeOccurred)
  {
    return;
  }
  
  int z, x, y;
  int frameTime = 140;
  int i = 0;
  
  start = millis();
  while(millis()-start < 10000)
  {
    LED( beeLocation[0], beeLocation[1], beeLocation[2], 15);
    LED( beeLocation2[0], beeLocation2[1], beeLocation2[2], 15);
    
    if(beeLocation[0] == 0)
    {
      i = random(0, 2);
      beeLocation[0] += i;
    }else if(beeLocation[0] == 1)
    {
      i = random(0, 3)-1;
      beeLocation[0] += i;
    }else if(beeLocation[0] = 2)
    {
      i = random(0, 2);
      beeLocation[0] -= i;
    }
    
    if(beeLocation[1] == 0)
    {
      i = random(0, 2);
      beeLocation[1] += i;
    }else if(beeLocation[1] == 1)
    {
      i = random(0, 3)-1;
      beeLocation[1] += i;
    }else if(beeLocation[1] = 2)
    {
      i = random(0, 2);
      beeLocation[1] -= i;
    }
    
    if(beeLocation[2] == 0)
    {
      i = random(0, 2);
      beeLocation[2] += i;
    }else if(beeLocation[2] == 1)
    {
      i = random(0, 3)-1;
      beeLocation[2] += i;
    }else if(beeLocation[2] = 2)
    {
      i = random(0, 2);
      beeLocation[2] -= i;
    }
    
    if(beeLocation2[0] == 0)
    {
      i = random(0, 2);
      beeLocation2[0] += i;
    }else if(beeLocation2[0] == 1)
    {
      i = random(0, 3)-1;
      beeLocation2[0] += i;
    }else if(beeLocation2[0] = 2)
    {
      i = random(0, 2);
      beeLocation2[0] -= i;
    }
    
    if(beeLocation2[1] == 0)
    {
      i = random(0, 2);
      beeLocation2[1] += i;
    }else if(beeLocation2[1] == 1)
    {
      i = random(0, 3)-1;
      beeLocation2[1] += i;
    }else if(beeLocation2[1] = 2)
    {
      i = random(0, 2);
      beeLocation2[1] -= i;
    }
    
    if(beeLocation2[2] == 0)
    {
      i = random(0, 2);
      beeLocation2[2] += i;
    }else if(beeLocation2[2] == 1)
    {
      i = random(0, 3)-1;
      beeLocation2[2] += i;
    }else if(beeLocation2[2] = 2)
    {
      i = random(0, 2);
      beeLocation2[2] -= i;
    }
    
    delay(frameTime);
    allOff();
    if(changeOccurred)
    {
      return;
    }
  }
}

void changeAnimation()
{
  state++;
  if(state == 9)
  {
    state = 0;
  }
  changeOccurred = true;
}
 
void animationState()
{
  int pinState = digitalRead(statePin);
  if(pinState == HIGH)
  {
    animation = true;
  }else {
    animation = false;
  }
  changeOccurred = true;
  state = 0;
}
