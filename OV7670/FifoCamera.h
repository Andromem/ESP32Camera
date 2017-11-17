#include "Arduino.h"
#include "Fifo.h"

template<class I2C, int RRST, int WRST, int RCK, int WR, int D0, int D1, int D2, int D3, int D4, int D5, int D6, int D7>
class FifoCamera
{
  static const int ADDR = 0x42;

  static const int REG_GAIN = 0x00;
  static const int REG_BLUE = 0x01;
  static const int REG_RED = 0x02;
  static const int REG_COM1 = 0x04;
  static const int REG_VREF = 0x03;
  static const int REG_COM4 = 0x0d;
  static const int REG_COM5 = 0x0e;
  static const int REG_COM6 = 0x0f;
  static const int REG_AECH = 0x10;
  static const int REG_CLKRC = 0x11;
  static const int REG_COM7 = 0x12;
    static const int COM7_RGB = 0x04;
  static const int REG_COM8 = 0x13;
    static const int COM8_FASTAEC = 0x80;    // Enable fast AGC/AEC
    static const int COM8_AECSTEP = 0x40;    // Unlimited AEC step size
    static const int COM8_BFILT = 0x20;    // Band filter enable
    static const int COM8_AGC = 0x04;    // Auto gain enable
    static const int COM8_AWB = 0x02;    // White balance enable
    static const int COM8_AEC = 0x0;
  static const int REG_COM9 = 0x14;
  static const int REG_COM10 = 0x15;
  static const int REG_COM14 = 0x3E;
  static const int REG_SCALING_DCWCTR = 0x72;
  static const int REG_SCALING_PCLK_DIV = 0x73;
  static const int REG_COM11 = 0x3B;
  static const int COM11_NIGHT = 0x80;
  static const int COM11_NMFR = 0x60;
  static const int COM11_HZAUTO = 0x10;
  static const int COM11_50HZ = 0x08;
  static const int COM11_EXP = 0x0;
  static const int REG_TSLB = 0x3A;
  static const int REG_RGB444 = 0x8C;
  static const int REG_COM15 = 0x40;
    static const int COM15_RGB565 = 0x10;
    static const int COM15_R00FF = 0xc0;
  static const int REG_HSTART = 0x17;
  static const int REG_HSTOP = 0x18;
  static const int REG_HREF = 0x32;
  static const int REG_VSTART = 0x19;
  static const int REG_VSTOP = 0x1A;
  static const int REG_COM3 = 0x0C;
  static const int REG_MVFP = 0x1E;
  static const int REG_COM13 = 0x3d;
    static const int COM13_UVSAT = 0x40;
  static const int SCALING_DCWCTR = 0x72;
  static const int SCALING_PCLK_DIV = 0x73;
  static const int REG_BD50MAX = 0xa5;
  static const int REG_BD60MAX = 0xab;
  static const int REG_AEW = 0x24;
  static const int REG_AEB = 0x25;
  static const int REG_VPT = 0x26;
  static const int REG_HAECC1 = 0x9f;
  static const int REG_HAECC2 = 0xa0;
  static const int REG_HAECC3 = 0xa6;
  static const int REG_HAECC4 = 0xa7;
  static const int REG_HAECC5 = 0xa8;
  static const int REG_HAECC6 = 0xa9;
  static const int REG_HAECC7 = 0xaa;
  static const int REG_COM12 = 0x3c;
  static const int REG_GFIX = 0x69;
  static const int REG_COM16 = 0x41;
  static const int COM16_AWBGAIN = 0x08;
  static const int REG_EDGE = 0x3f;
  static const int REG_REG76 = 0x76;
  static const int ADCCTR0 = 0x20;

  bool readingFrame = false;
  bool writingFrame = false;

  I2C &i2c;
  Fifo<RRST, WRST, RCK, WR, D0, D1, D2, D3, D4, D5, D6, D7> fifo;
  
  public:
  FifoCamera(I2C &_i2c)
    :i2c(_i2c)
  { 
  }

  void init()
  {
    fifo.init();
    
    //reset all registers to default
    i2c.writeRegister(ADDR, REG_COM7, 0x80);
  }

  void testImage()
  {
    //i2c.writeRegister(ADDR, 0x70, 0x4A | 0x80);
    i2c.writeRegister(ADDR, 0x71, 0x35 | 0x80);
  }

  void prepareCapture()
  {
    fifo.writeReset();
  }

  void startCapture()
  {
    fifo.writeEnable();
  }
  
  void stopCapture()
  {
    fifo.writeDisable();
  }

  void inline readFrame(unsigned char *frame, const int xres, const int yres, const int bytes)
  {
    fifo.readReset();
    int i = 0;
    for(int y = 0; y < yres; y++)
      for(int x = 0; x < xres; x++)
        for(int b = 0; b < bytes; b++)        
          frame[i++] = fifo.readByte();
  }
  
  void writeRegisters(const unsigned char data[][2])
  {
    for(int i = 0; data[i][0] != 0xff; i++)
      i2c.writeRegister(ADDR, data[i][0], data[i][1]);
  }

 void commonRegisterSetup()
 {
const unsigned char common_reglist_ov7670[][2] = {//from the linux driver

  { REG_COM3, 0 }, { REG_COM14, 0 },
  // Mystery scaling numbers 
  { 0x70, 0x3a },   { 0x71, 0x35 },
  { 0x72, 0x11 },   { 0x73, 0xf0 },
  { 0xa2, 1},{ REG_COM10, 0x0 },
   //
  { 0x7a, 0x20 },   { 0x7b, 0x10 },
  { 0x7c, 0x1e },   { 0x7d, 0x35 },
  { 0x7e, 0x5a },   { 0x7f, 0x69 },
  { 0x80, 0x76 },   { 0x81, 0x80 },
  { 0x82, 0x88 },   { 0x83, 0x8f },
  { 0x84, 0x96 },   { 0x85, 0xa3 },
  { 0x86, 0xaf },   { 0x87, 0xc4 },
  { 0x88, 0xd7 },   { 0x89, 0xe8 },
/*  // AGC and AEC parameters.  Note we start by disabling those features,
  // then turn them only after tweaking the values. 
  { REG_COM8, COM8_FASTAEC | COM8_AECSTEP },
  { REG_GAIN, 0 }, { REG_AECH, 0 },
  { REG_COM4, 0x40 }, // magic reserved bit
  { REG_COM9, 0x18 }, // 4x gain + magic rsvd bit
  { REG_BD50MAX, 0x05 }, { REG_BD60MAX, 0x07 },
  { REG_AEW, 0x95 }, { REG_AEB, 0x33 },
  { REG_VPT, 0xe3 }, { REG_HAECC1, 0x78 },
  { REG_HAECC2, 0x68 },  { 0xa1, 0x03 }, // magic 
  { REG_HAECC3, 0xd8 },  { REG_HAECC4, 0xd8 },
  { REG_HAECC5, 0xf0 },  { REG_HAECC6, 0x90 },
  { REG_HAECC7, 0x94 },
  { REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_AGC|COM8_AEC },
    {0x30,0},{0x31,0},//disable some delays
  // Almost all of these are magic "reserved" values. 
  { REG_COM5, 0x61 },  { REG_COM6, 0x4b },
  { 0x16, 0x02 },   { REG_MVFP, 0x07 },
  { 0x21, 0x02 },   { 0x22, 0x91 },
  { 0x29, 0x07 },   { 0x33, 0x0b },
  { 0x35, 0x0b },   { 0x37, 0x1d },
  { 0x38, 0x71 },   { 0x39, 0x2a },
  { REG_COM12, 0x78 }, { 0x4d, 0x40 },
  { 0x4e, 0x20 },   { REG_GFIX, 0 },
  //{ 0x6b, 0x4a },   
  { 0x74,0x10},
  { 0x8d, 0x4f },   { 0x8e, 0 },
  { 0x8f, 0 },    { 0x90, 0 },
  { 0x91, 0 },    { 0x96, 0 },
  { 0x9a, 0 },    { 0xb0, 0x84 },
  { 0xb1, 0x0c },   { 0xb2, 0x0e },
  { 0xb3, 0x82 },   { 0xb8, 0x0a },

  // More reserved magic, some of which tweaks white balance 
  { 0x43, 0x0a },   { 0x44, 0xf0 },
  { 0x45, 0x34 },   { 0x46, 0x58 },
  { 0x47, 0x28 },   { 0x48, 0x3a },
  { 0x59, 0x88 },   { 0x5a, 0x88 },
  { 0x5b, 0x44 },   { 0x5c, 0x67 },
  { 0x5d, 0x49 },   { 0x5e, 0x0e },
  { 0x6c, 0x0a },   { 0x6d, 0x55 },
  { 0x6e, 0x11 },   { 0x6f, 0x9e }, // it was 0x9F "9e for advance AWB"
  { 0x6a, 0x40 },   { REG_BLUE, 0x40 },
  { REG_RED, 0x60 },
  { REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_AGC|COM8_AEC|COM8_AWB },

  // Matrix coefficients 
  { 0x4f, 0x80 },   { 0x50, 0x80 },
  { 0x51, 0 },    { 0x52, 0x22 },
  { 0x53, 0x5e },   { 0x54, 0x80 },
  { 0x58, 0x9e },

  { REG_COM16, COM16_AWBGAIN },  { REG_EDGE, 0 },
  { 0x75, 0x05 },   { REG_REG76, 0xe1 },
  { 0x4c, 0 },    { 0x77, 0x01 },
  { REG_COM13, 0x48 }, { 0x4b, 0x09 },
  { 0xc9, 0x60 },   //{ OV7670_REG_COM16, 0x38 },
  { 0x56, 0x40 },

  { 0x34, 0x11 },   { REG_COM11, COM11_EXP|COM11_HZAUTO },
  { 0xa4, 0x82 },   { 0x96, 0 },
  { 0x97, 0x30 },   { 0x98, 0x20 },
  { 0x99, 0x30 },   { 0x9a, 0x84 },
  { 0x9b, 0x29 },   { 0x9c, 0x03 },
  { 0x9d, 0x4c },   { 0x9e, 0x3f },
  { 0x78, 0x04 },

  // Extra-weird stuff.  Some sort of multiplexor register 
  { 0x79, 0x01 },   { 0xc8, 0xf0 },
  { 0x79, 0x0f },   { 0xc8, 0x00 },
  { 0x79, 0x10 },   { 0xc8, 0x7e },
  { 0x79, 0x0a },   { 0xc8, 0x80 },
  { 0x79, 0x0b },   { 0xc8, 0x01 },
  { 0x79, 0x0c },   { 0xc8, 0x0f },
  { 0x79, 0x0d },   { 0xc8, 0x20 },
  { 0x79, 0x09 },   { 0xc8, 0x80 },
  { 0x79, 0x02 },   { 0xc8, 0xc0 },
  { 0x79, 0x03 },   { 0xc8, 0x40 },
  { 0x79, 0x05 },   { 0xc8, 0x30 },
  { 0x79, 0x26 },/**/
  
  { 0xff, 0xff }  // END MARKER
};
    writeRegisters(common_reglist_ov7670);  
 }
 
//
  void saturation(int s)  //-2 to 2
  {
    //color matrix values
    i2c.writeRegister(ADDR, 0x4f, 0x80 + 0x20 * s);
    i2c.writeRegister(ADDR, 0x50, 0x80 + 0x20 * s);
    i2c.writeRegister(ADDR, 0x51, 0x00);
    i2c.writeRegister(ADDR, 0x52, 0x22 + (0x11 * s) / 2);
    i2c.writeRegister(ADDR, 0x53, 0x5e + (0x2f * s) / 2);
    i2c.writeRegister(ADDR, 0x54, 0x80 + 0x20 * s);
    i2c.writeRegister(ADDR, 0x58, 0x9e);  //matrix signs
  }
  
  void QQVGARGB()
  {
    
    i2c.writeRegister(ADDR, REG_COM7, 0b10000000);  //all registers default
    //delay(300);
    //commonRegisterSetup();
    
    i2c.writeRegister(ADDR, REG_CLKRC, 0b10000000); //double clock
    i2c.writeRegister(ADDR, REG_COM11, 0b1000 | 0b10); //enable auto 50/60Hz detect + exposure timing can be less...
    i2c.writeRegister(ADDR, REG_TSLB, 0b100); //sequence UYVY

    i2c.writeRegister(ADDR, REG_COM7, 0b100); //RGB
    i2c.writeRegister(ADDR, REG_COM15, 0b11000000 | 0b010000); //RGB565

    i2c.writeRegister(ADDR, REG_HSTART, 0x16);
    i2c.writeRegister(ADDR, REG_HSTOP, 0x04);
    i2c.writeRegister(ADDR, REG_HREF, 0x24);
    
    i2c.writeRegister(ADDR, REG_VSTART, 0x02);
    i2c.writeRegister(ADDR, REG_VSTOP, 0x7a);
    i2c.writeRegister(ADDR, REG_VREF, 0x0a);
    
    i2c.writeRegister(ADDR, REG_COM10, 0x02);
    i2c.writeRegister(ADDR, REG_COM3, 0x04);
    i2c.writeRegister(ADDR, REG_MVFP, 0x2b);
        
    //i2c.writeRegister(ADDR, REG_MVFP, 0);  //no mirror or flip

    //i2c.writeRegister(ADDR, ADCCTR0, 0b1000); //ADC settings

    //80x60 (1/8)
    //i2c.writeRegister(ADDR, REG_COM14, 0x1b); 
    //i2c.writeRegister(ADDR, 0x72, 0x33); 
    //i2c.writeRegister(ADDR, 0x73, 0xf3);

    //160x120 (1/4)
    i2c.writeRegister(ADDR, REG_COM14, 0x1a); 
    i2c.writeRegister(ADDR, SCALING_DCWCTR, 0x22); 
    i2c.writeRegister(ADDR, SCALING_PCLK_DIV, 0xf2);     

    i2c.writeRegister(ADDR, 0xb0, 0x84);// no clue what this is but it's most important for colors
    saturation(0);
    i2c.writeRegister(ADDR, 0x13, 0xe7); //AWB on
    i2c.writeRegister(ADDR, 0x6f, 0x9f); // Simple AWB
  }
  
  void QQVGARGB565()
  {
    i2c.writeRegister(ADDR, REG_COM7, 0x80); /* reset to default values */
    i2c.writeRegister(ADDR, REG_CLKRC, 0x80);
    i2c.writeRegister(ADDR, REG_COM11, 0x0A);
    i2c.writeRegister(ADDR, REG_TSLB, 0x04);

    i2c.writeRegister(ADDR, REG_COM7, 0x04); /* output format: rgb */

    i2c.writeRegister(ADDR, REG_RGB444, 0x00); /* disable RGB444 */
    i2c.writeRegister(ADDR, REG_COM15, 0xD0); /* set RGB565 */
    //i2c.writeRegister(ADDR, REG_COM15, 0b110000); /* set RGB565 */

    /* not even sure what all these do, gonna check the oscilloscope and go
     * from there... */
    i2c.writeRegister(ADDR, REG_HSTART, 0x16);
    i2c.writeRegister(ADDR, REG_HSTOP, 0x04);
    i2c.writeRegister(ADDR, REG_HREF, 0x24);
    i2c.writeRegister(ADDR, REG_VSTART, 0x02);
    i2c.writeRegister(ADDR, REG_VSTOP, 0x7a);
    i2c.writeRegister(ADDR, REG_VREF, 0x0a);
    i2c.writeRegister(ADDR, REG_COM10, 0x02);
    i2c.writeRegister(ADDR, REG_COM3, 0x04);
    i2c.writeRegister(ADDR, REG_MVFP, 0x3f);

    //ov7670_set(REG_MVFP, 0x27);

    //ov7670_set(REG_COM14, 0x1b); // divide by 8
    //ov7670_set(0x72, 0x33); // downsample by 8
    //ov7670_set(0x73, 0xf3); // divide by 8


    /* 160x120, i think */
    i2c.writeRegister(ADDR, REG_COM14, 0x1a); // divide by 4
    i2c.writeRegister(ADDR, SCALING_DCWCTR, 0x22); // downsample by 4
    i2c.writeRegister(ADDR, SCALING_PCLK_DIV, 0xf2); // divide by 4

    /* 320x240: */
    //i2c.writeRegister(ADDR, REG_COM14, 0x19);
    //i2c.writeRegister(ADDR, SCALING_DCWCTR, 0x11);
    //i2c.writeRegister(ADDR, SCALING_PCLK_DIV, 0xf1);

  }



  void RGBRaw()
  {
    const unsigned char RGBBeyerRAW[][2] = {{0x12, 0x80},
    
    {0x11, 0x80}, 
    //{0x11, 0x01},
    {0x3a, 0x04},
    {0x12, 0x01},
    {0x17, 0x12},
    {0x18, 0x00},
    {0x32, 0xb6},
    {0x19, 0x02},
    {0x1a, 0x7a},
    {0x03, 0x00},
    {0x0c, 0x00},
    {0x3e, 0x00},
    {0x70, 0x3a},
    {0x71, 0x35},
    {0x72, 0x11},
    {0x73, 0xf0},
    {0xa2, 0x02},
    
    {0x13, 0xe0},
    {0x00, 0x00},
    {0x10, 0x00},
    {0x0d, 0x40},
    {0x14, 0x38},
    {0xa5, 0x07},
    {0xab, 0x08},
    {0x24, 0x95},
    {0x25, 0x33},
    {0x26, 0xe3},
    {0x9f, 0x78},
    {0xa0, 0x68},
    {0xa1, 0x0b},
    {0xa6, 0xd8},
    {0xa7, 0xd8},
    {0xa8, 0xf0},
    {0xa9, 0x90},
    {0xaa, 0x94},
    {0x13, 0xe5},
    
    {0x0e, 0x61},
    {0x0f, 0x4b},
    {0x16, 0x02},
    {0x21, 0x02},
    {0x22, 0x91},
    {0x29, 0x07},
    {0x33, 0x03},
    {0x35, 0x0b},
    {0x37, 0x1c},
    {0x38, 0x71},
    {0x3c, 0x78},
    {0x3d, 0x08},
    {0x41, 0x3a},
    {0x4d, 0x40},
    {0x4e, 0x20},
    {0x69, 0x55},
    {0x6b, 0x4a},
    {0x74, 0x19},
    {0x76, 0x61},
    {0x8d, 0x4f},
    {0x8e, 0x00},
    {0x8f, 0x00},
    {0x90, 0x00},
    {0x91, 0x00},
    {0x96, 0x00},
    {0x9a, 0x80},
    {0xb0, 0x8c},
    {0xb1, 0x0c},
    {0xb2, 0x0e},
    {0xb3, 0x82},
    {0xb8, 0x0a},
    
    {0x43, 0x14},
    {0x44, 0xf0},
    {0x45, 0x34},
    {0x46, 0x58},
    {0x47, 0x28},
    {0x48, 0x3a},
    {0x59, 0x88},
    {0x5a, 0x88},
    {0x5b, 0x44},
    {0x5c, 0x67},
    {0x5d, 0x49},
    {0x5e, 0x0e},
    {0x6c, 0x0a},
    {0x6d, 0x55},
    {0x6e, 0x11},
    {0x6f, 0x9f},
    {0x6a, 0x40},
    {0x01, 0x40},
    {0x02, 0x40},
    {0x13, 0xe7},
    
    {0x34, 0x11},
    {0x92, 0x66},
    {0x3b, 0x0a},
    {0xa4, 0x88},
    {0x96, 0x00},
    {0x97, 0x30},
    {0x98, 0x20},
    {0x99, 0x20},
    {0x9a, 0x84},
    {0x9b, 0x29},
    {0x9c, 0x03},
    {0x9d, 0x4c},
    {0x9e, 0x3f},
    {0x78, 0x04},
    
    {0x79, 0x01},
    {0xc8, 0xf0},
    {0x79, 0x0f},
    {0xc8, 0x20},
    {0x79, 0x10},
    {0xc8, 0x7e},
    {0x79, 0x0b},
    {0xc8, 0x01},
    {0x79, 0x0c},
    {0xc8, 0x07},
    {0x79, 0x0d},
    {0xc8, 0x20},
    {0x79, 0x09},
    {0xc8, 0x80},
    {0x79, 0x02},
    {0xc8, 0xc0},
    {0x79, 0x03},
    {0xc8, 0x40},
    {0x79, 0x05},
    {0xc8, 0x30},
    {0x79, 0x26},
    {0xff, 0xff}};
    writeRegisters(RGBBeyerRAW);
      
/*    i2c.writeRegister(ADDR, REG_HSTART, 0x16);
    i2c.writeRegister(ADDR, REG_HSTOP, 0x04);
    i2c.writeRegister(ADDR, REG_HREF, 0x24);
    
    i2c.writeRegister(ADDR, REG_VSTART, 0x02);
    i2c.writeRegister(ADDR, REG_VSTOP, 0x7a);
    i2c.writeRegister(ADDR, REG_VREF, 0x0a);*/
    
    i2c.writeRegister(ADDR, REG_COM10, 0x02);
    i2c.writeRegister(ADDR, REG_COM3, 0x04);
    i2c.writeRegister(ADDR, REG_MVFP, 0x2b);

    i2c.writeRegister(ADDR, REG_COM14, 0x1a); // divide by 4
    i2c.writeRegister(ADDR, 0x72, 0x22); // downsample by 4
    i2c.writeRegister(ADDR, 0x73, 0xf2); // divide by 4*/
  }
};

