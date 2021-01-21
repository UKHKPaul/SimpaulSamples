// variation of the C standalone code to be used from Python
//
// This is code provides aC library which can be called from Python to allow fast(er) access to the SPI controller display
// initial build is for use with a 1.28" circular display using a GC9A01 controller
// Part can be found at https://www.aliexpress.com/item/1005001321857930.html
//
// Developed by P.Worman Jan 2021
//
// version 1.0
// 
// compile and link the bcm library with
//
// gcc -shared -o bcm_direct_c2py.so -fPIC bcm_direct_c2py.c -l bcm2835
//
// for more details see http://simpaul.com/round_display



#include <bcm2835.h>    // if this gives errors check the above website for installation instructions
#include <stdio.h>
#include <stdbool.h> 
#include <string.h>
#include <math.h>
#include "bcm_direct_c2py.h"

// pin numbers are hte BCM values
#define CIR_SCL  11     // this has to remain the same
#define CIR_SDA  10     // this has to remain the same
#define CIR_RES  25     // change this as needed
#define CIR_DC   24     // change this as needed also
#define CIR_CS   8      // possible to change, but will need similar config in the BCM setup


#define USE_HORIZONTAL 3  // Set the display direction 0,1,2,3    four directions
                          // 0 top of screen is 90 CCW from connector
                          // 1 top of screen is 90 CW from connector
                          // 2 top of screen is by connector 
                          // 3 connector at bottom of screen

// global pointer for the rendering workspace
// this maintains a memory to build a screen before it is updated on the device
// this allows for faster updates of complex images
unsigned short * RenderSpace = NULL; 
unsigned short * ReferenceSpace = NULL;


// main this should not be used directly as this is a library
int main(int argc, char **argv)
{
    printf("bcm_direct_c2py is a library for use with Python\n");
      
    return 0;
}


// BCM hardware initialisation
// for now this configures the SPI port and the relevant pins for the circular display
// future updates will add more flexibility to this
// note due to the low lever BCM driver, if the code is not called as root then the init and SPI begin will fail
bool initBCMHardware(void)
{
    if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
      return (false);
    }

    if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return (false);
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);        // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                     // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4);       // about 32Mhz  Seems to work on PI3 and PI0 with no noticable issues
                                                                    // if issues occur reduce this to DIVIDER_8 or even 16 
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                        // The default - possible to change, but not tested
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);        // the default - active low chip select (this is most common)


    // setup the DC and RES pins  
    bcm2835_gpio_fsel(CIR_RES, BCM2835_GPIO_FSEL_OUTP);             // for the Ciruclar display a manual reset is needed
    bcm2835_gpio_fsel(CIR_DC,  BCM2835_GPIO_FSEL_OUTP);             // and a separate pin for Command/Data selection

    bcm2835_gpio_write(CIR_RES, LOW);                               //reset the chip
    bcm2835_gpio_write(CIR_DC, HIGH); 

    bcm2835_delay(20);
    bcm2835_gpio_write(CIR_RES, HIGH);                              // release the reset to the chip
    bcm2835_delay(20);
    return (true);
}


// exitBCMHardware 
// this releases any allocated memory and clearnly shuts down the SPI driver and low level BCM control
void exitBCMHardware(void)
{
    //printf("Exiting hardware\n");
    bcm2835_spi_end();
    bcm2835_close();


    //printf("RenderSpace about to be freed\n");
    //printf("renderspace = %p\n",RenderSpace);

    if (RenderSpace !=NULL)
    {
        free(RenderSpace);
        RenderSpace = NULL;
        //printf("RenderSpace was freed\n");
    }
    //else
        //printf("RenderSpace was not created\n");


    if (ReferenceSpace !=NULL)
    {
        free(ReferenceSpace);
        ReferenceSpace = NULL;
        //printf("ReferenceSpace was freed\n");
    }
}


// initCircularDisp
// this function configures the circular display itself.
// the code is based on a sample found online which has a number of undocumented settings
// where page numbers and details are given the information has been taken from the GalaxyCore GC9A01 Rev1.0 datasheet
// see the Simpaul.com web page for download/link
// note I have commented out as many of the undocumented commands as I can and most seem to ahve no effect on the output
// however the 0x66 and 0x67 commands certainly do affect the output
void initCircularDisp(void)
{

    // create some memory space for a render buffer
    // this is used to allow multiple display changes to be done without having multiple screen updates
    // this means the updated image can be created and then sent to the display in one update
    // this gives a much smoother output without obvious on screen drawing. 
    RenderSpace = (unsigned short *) malloc (240*240*2);
    if (RenderSpace ==NULL)
        printf("ERROR - RenderSpace was not created\n");

    //sdoCmdU8(0xEB);    //*** not listed
    //sdoDataU8(0x14);
    
    sdoCmdU8(0xFE);    //inter register enable 1 P172         
    sdoCmdU8(0xEF);    //inter register enable 2 P173
/*
    sdoCmdU8(0xEB);    //*** not listed
    sdoDataU8(0x14);

    sdoCmdU8(0x84);    //*** not listed     
    sdoDataU8(0x40);

    sdoCmdU8(0x85);    //*** not listed       
    sdoDataU8(0xFF);

    sdoCmdU8(0x86);    //*** not listed   
    sdoDataU8(0xFF);

    sdoCmdU8(0x87);    //*** not listed             
    sdoDataU8(0xFF);

    sdoCmdU8(0x88);           
    sdoDataU8(0x0A);

    sdoCmdU8(0x89);           
    sdoDataU8(0x21);

    sdoCmdU8(0x8A);           
    sdoDataU8(0x00);

    sdoCmdU8(0x8B);           
    sdoDataU8(0x80);

    sdoCmdU8(0x8C);           
    sdoDataU8(0x01);

    sdoCmdU8(0x8D);           
    sdoDataU8(0x01);

    sdoCmdU8(0x8E);           
    sdoDataU8(0xFF);

    sdoCmdU8(0x8F);           
    sdoDataU8(0xFF);
*/

    sdoCmdU8(0xB6);    //Display function control P158
    sdoDataU8(0x00);   //Must be zero         
    sdoDataU8(0x00);   //Defines shift register directions

    sdoCmdU8(0x36);    // Memory access control datasheet P127 
    if(USE_HORIZONTAL==0)
      sdoDataU8(0xE8); // sample code showed this a 18 but that didn't work
    if(USE_HORIZONTAL==1)
      sdoDataU8(0x28);
    if(USE_HORIZONTAL==2)
      sdoDataU8(0x48);
    if(USE_HORIZONTAL==3)
      sdoDataU8(0x88);

    sdoCmdU8(0x3A);    //COLMOD Pixel Fomrat Set  P135        
    sdoDataU8(0x55);   //RGB Mode Ignored, MCU Mode set to 16 Bits per Pixel
                            //colour is 5 Red, 6 Green, 5 Blue

/*
    sdoCmdU8(0x90);    //*** not listed            
    sdoDataU8(0x08);
    sdoDataU8(0x08);
    sdoDataU8(0x08);
    sdoDataU8(0x08);

    sdoCmdU8(0xBD);    //*** not listed             
    sdoDataU8(0x06);

    sdoCmdU8(0xBC);    //*** not listed         
    sdoDataU8(0x00);   

    sdoCmdU8(0xFF);    //*** not listed           
    sdoDataU8(0x60);
    sdoDataU8(0x01);
    sdoDataU8(0x04);
*/
    sdoCmdU8(0xC3);    //Power Control 2 P 168          
    sdoDataU8(0x13);
    sdoCmdU8(0xC4);    //Power Control 3 P 169        
    sdoDataU8(0x13);

    sdoCmdU8(0xC9);    //Power Control 4 P 170        
    sdoDataU8(0x22);
/*
    sdoCmdU8(0xBE);    //*** not listed           
    sdoDataU8(0x11);

    sdoCmdU8(0xE1);    //*** not listed       
    sdoDataU8(0x10);
    sdoDataU8(0x0E);

    sdoCmdU8(0xDF);    //*** not listed     
    sdoDataU8(0x21);
    sdoDataU8(0x0c);
    sdoDataU8(0x02);
*/
    sdoCmdU8(0xF0);    //SET_GAMMA1  P 174
    sdoDataU8(0x45);
    sdoDataU8(0x09);
    sdoDataU8(0x08);
    sdoDataU8(0x08);
    sdoDataU8(0x26);
    sdoDataU8(0x2A);

    sdoCmdU8(0xF1);    //SET_GAMMA2  P 176 
    sdoDataU8(0x43);
    sdoDataU8(0x70);
    sdoDataU8(0x72);
    sdoDataU8(0x36);
    sdoDataU8(0x37); 
    sdoDataU8(0x6F);


    sdoCmdU8(0xF2);    //SET_GAMMA3  P 178 
    sdoDataU8(0x45);
    sdoDataU8(0x09);
    sdoDataU8(0x08);
    sdoDataU8(0x08);
    sdoDataU8(0x26);
    sdoDataU8(0x2A);

    sdoCmdU8(0xF3);    //SET_GAMMA4  P 180
    sdoDataU8(0x43);
    sdoDataU8(0x70);
    sdoDataU8(0x72);
    sdoDataU8(0x36);
    sdoDataU8(0x37);
    sdoDataU8(0x6F);
/*
    sdoCmdU8(0xED);    //*** not listed      
    sdoDataU8(0x1B);
    sdoDataU8(0x0B);

    sdoCmdU8(0xAE);    //*** not listed          
    sdoDataU8(0x77);

    sdoCmdU8(0xCD);    //*** not listed           
    sdoDataU8(0x63);       


    sdoCmdU8(0x70);    //*** not listed           
    sdoDataU8(0x07);
    sdoDataU8(0x07);
    sdoDataU8(0x04);
    sdoDataU8(0x0E);
    sdoDataU8(0x0F);
    sdoDataU8(0x09);
    sdoDataU8(0x07);
    sdoDataU8(0x08);
    sdoDataU8(0x03);
*/
    sdoCmdU8(0xE8);    //Frame Rate P164           
    sdoDataU8(0x34);   //4 dot inversion
/*
    sdoCmdU8(0x62);    //*** not listed          
    sdoDataU8(0x18);
    sdoDataU8(0x0D);
    sdoDataU8(0x71);
    sdoDataU8(0xED);
    sdoDataU8(0x70);
    sdoDataU8(0x70);
    sdoDataU8(0x18);
    sdoDataU8(0x0F);
    sdoDataU8(0x71);
    sdoDataU8(0xEF);
    sdoDataU8(0x70);
    sdoDataU8(0x70);

    sdoCmdU8(0x63);    //*** not listed             
    sdoDataU8(0x18);
    sdoDataU8(0x11);
    sdoDataU8(0x71);
    sdoDataU8(0xF1);
    sdoDataU8(0x70);
    sdoDataU8(0x70);
    sdoDataU8(0x18);
    sdoDataU8(0x13);
    sdoDataU8(0x71);
    sdoDataU8(0xF3);
    sdoDataU8(0x70);
    sdoDataU8(0x70);

    sdoCmdU8(0x64);    //*** not listed             
    sdoDataU8(0x28);
    sdoDataU8(0x29);
    sdoDataU8(0xF1);
    sdoDataU8(0x01);
    sdoDataU8(0xF1);
    sdoDataU8(0x00);
    sdoDataU8(0x07);

*/

    sdoCmdU8(0x66);    //*** not listed            
    sdoDataU8(0x3C);   // seem to affect how pixels are displayed
    sdoDataU8(0x00);
    sdoDataU8(0xCD);
    sdoDataU8(0x67);
    sdoDataU8(0x45);
    sdoDataU8(0x45);
    sdoDataU8(0x10);
    sdoDataU8(0x00);
    sdoDataU8(0x00);
    sdoDataU8(0x00);

    

    sdoCmdU8(0x67);    //*** not listed              
    sdoDataU8(0x00);   // seem to affect how pixels are displayed 
    sdoDataU8(0x3C);       
    sdoDataU8(0x00);
    sdoDataU8(0x00);
    sdoDataU8(0x00);
    sdoDataU8(0x01);
    sdoDataU8(0x54);    
    sdoDataU8(0x10);
    sdoDataU8(0x32);    
    sdoDataU8(0x98);

/*

    sdoCmdU8(0x74);    //*** not listed            
    sdoDataU8(0x10);   
    sdoDataU8(0x85);   
    sdoDataU8(0x80);
    sdoDataU8(0x00);
    sdoDataU8(0x00);
    sdoDataU8(0x4E);
    sdoDataU8(0x00);         

    sdoCmdU8(0x98);    //*** not listed             
    sdoDataU8(0x3e);
    sdoDataU8(0x07);
*/
    sdoCmdU8(0x35);    //Tearing Effect Line ON P125
    sdoDataU8(0x01);   //turned on.

    sdoCmdU8(0x21);    //  Invert screen colours

    sdoCmdU8(0x11);    //Sleep Out Mode P103   - turns off sleep mode - i.e. turn on the screen
    bcm2835_delay(120);
    sdoCmdU8(0x29);    //Display On P 110
    bcm2835_delay(20);

    //printf ("circular setup complete\n");

}

// low level driver using SPI direct to write 8 bit value out as a command
// as it's a command, make sure to set the D/C pin low = Command
void sdoCmdU8( unsigned char byteval)
{
    bcm2835_gpio_write(CIR_DC, LOW);    
    bcm2835_spi_transfer(byteval);
}

// low level driver using SPI direct to write 16 bit value out as Data
// as it's a data, make sure to set the D/C pin high = data
void sdoDataU16( unsigned short intval)
{
    bcm2835_gpio_write(CIR_DC, HIGH);    
    bcm2835_spi_transfer(intval>>8);
    bcm2835_spi_transfer(intval&0xff);
}
// low level driver using SPI direct to write 8 bit value out as Data
// as it's a data, make sure to set the D/C pin high = data
void sdoDataU8( unsigned char byteval)
{
    bcm2835_gpio_write(CIR_DC, HIGH);    
    bcm2835_spi_transfer(byteval);
}



// low level ClearScreen
// 
// this optimised version can do over 600 full screen updated per minute when using the 
// BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       /*!< 4 = 62.5MHz on Rpi2, 100MHz on RPI3. Dont expect this speed to work reliably. */
// on RPI3 this seems totally stable
//
//
// This now supports the render space to allow background updates to occur prior to screen updates
//
void clearScreenDirect(unsigned short bcolour)
{
    int i;
    unsigned short * sourcePtr;

    if (RenderSpace !=NULL)
    {
        sourcePtr = RenderSpace;
        for (i=0;i<(240*240);i++)
        {
            *(sourcePtr++) = bcolour;
        }
    }
    ScreenUpdate();
}


// writing a BMP 24bit 240x240 image from the raw data
// outputs covnert each 24 bit value to 16 bit combined and then updates the render space
// when done, full screen update is performed
void RGB240x240Direct(unsigned char * rawdata,bool update  )
{
  unsigned char * counter;
  unsigned short colour;
  unsigned int offset;
  if (      (rawdata[10]!=54)                       // data in the right place
         || (rawdata[14]!=40)                       // right type of header
         || (rawdata[18]!=240) || (rawdata[19]!=0)  // right x size
         || (rawdata[22]!=240) || (rawdata[23]!=0)  // right y size
         || (rawdata[28]!=24))                      // right bits per pixel                 
    printf("not a valid 240x240 image\n");
  else
  {
    //printf("valid file format\n");
    
    counter = rawdata + 54; //  skip past the headers
    //note BMP has 1st pixel as lower left which is the line 239 on the display
    for (short y=240;y>0;y--)
    {
      offset = (y-1)*240;
      for (short x=0; x<240;x++)
      {
        colour  = RGBto16bit(*(counter+2), *(counter+1), *(counter));
        // BMP has colour with Green first, then Blue then Red
        RenderSpace[x+offset]=colour;
       
        counter=counter+3;
      }
    }
    if(update)
        ScreenUpdate();
  }
}

// makes a copy of the render space to a buffer
// only created the reference image space if this is used 
// as most time the python code would not need it
void SetRefernceImage(void)
{
    if(ReferenceSpace ==NULL)
    {
        ReferenceSpace = (unsigned short *)malloc(240*240*2);
        if(ReferenceSpace ==NULL)
        {
            printf("Error unable to create reference space\n");
        }
    }
         
    if ((RenderSpace !=NULL) && (ReferenceSpace !=NULL))
    {
        memcpy(ReferenceSpace,RenderSpace,240*240*2);
    }
}


//restore the reference to the render space
void RestoreReferenceImage(void)
{
    if ((RenderSpace !=NULL) && (ReferenceSpace !=NULL))
    {
        memcpy(RenderSpace,ReferenceSpace,240*240*2);
    }
}




// SetScreenWriteArea
// define the start and end pixels to be updated
// note these are inclusive values
// i.e. writing will be from Xstart up to and including Xend and same for the Y axis
// hence full screen is 0,0 to 239,239
void SetScreenWriteArea(unsigned char Xstart,unsigned char Ystart,unsigned char Xend,unsigned char Yend)
{
    if((Xend<240) && (Yend<240) &&(Xstart<=Xend)&& (Ystart<=Yend))
    {
        sdoCmdU8(0x2a);             // select Column address P111
        sdoDataU16(Xstart);
        sdoDataU16(Xend);

        sdoCmdU8(0x2b);             // select Row Address Set P113
        sdoDataU16(Ystart);
        sdoDataU16(Yend);

        sdoCmdU8(0x2c);            // Memory Write P 115
                                   // sets the display to receive data
    }
    else
    {
        // invalid co-ordinates
        printf("SetScreenWriteArea Invalid Co-ordinates\n");
    }
}


// SetPixelDirect
// individual pixel setting, this is direct and updates the screen immediatle
// use this for updating direct to the screen (use for small changes)
// work on the Render space and do screen update for large changes
void  SetPixelDirect(unsigned short xpos, unsigned short ypos, unsigned short colour)
{
    if((xpos>239) || (ypos>239))
        printf("SetPixel writing to invalid\n");
    else
    {
        sdoCmdU8(0x2a);             // select Column address P111
        sdoDataU16(xpos);
        sdoDataU16(xpos);

        sdoCmdU8(0x2b);             // select Row Address Set P113
        sdoDataU16(ypos);
        sdoDataU16(ypos);

        sdoCmdU8(0x2c);             // write the data value
        sdoDataU16(colour);

        // this makes sure the render image is kept in sync with the direct updates
        RenderSpace[(xpos)+(ypos)*240]=colour;
    }
}


// SetPixel
// updates a pixel in the renderspace with checking to make sure the co-ordinates are valid
// this prevents screen wrap round or invalid memory access
void SetPixel(short xpos, short ypos, unsigned short colour)
{
    if((xpos>=0)&&(xpos<240)&&(ypos>=0)&&(ypos<240))
        RenderSpace[xpos+ypos*240]=colour;
   // else
   //     printf("trying to write outside screen\n");
}


// DrawCircle
//
// indirect circle writing
// this stores the data in the Render space only
// use the ScreenUpdate to output this and any other updates
// note use of shorts for x,y to allow for circle origins outside of the screen 
void DrawCircle (short x0, short y0, short r, unsigned short colour)
{
int a=0;
int b=r;     
    while(a<=b)
    {
        SetPixel((x0-b),(y0-a),colour);
        SetPixel((x0+b),(y0-a),colour);                   
        SetPixel((x0-a),(y0+b),colour);                         
        SetPixel((x0-a),(y0-b),colour);           
        SetPixel((x0+b),(y0+a),colour);                        
        SetPixel((x0+a),(y0-b),colour);      
        SetPixel((x0+a),(y0+b),colour);         
        SetPixel((x0-b),(y0+a),colour);
        a+=1;
        if((a*a+b*b)>(r*r))
            b-=1;
    }
}


// DrawRectangle
// draw a box in the render space
void DrawRectangle(short Xstart,short Ystart, short Xend, short Yend, unsigned short colour)
{

    // note we can use integer maths routines here as the lines are by defintion horizontal or vertical.

    // first the two horrizonal lines
    DrawLineIntMaths(Xstart,Ystart,Xend,Ystart,colour);
    DrawLineIntMaths(Xstart,Yend,Xend,Yend,colour);
    // then the two vertical ones
    DrawLineIntMaths(Xstart,Ystart,Xstart,Yend,colour);
    DrawLineIntMaths(Xend,Ystart,Xend,Yend,colour);

}


// pixel mixing, used to do the anti-alising on the lines.
// note the intensity is a short with 256 being eqivalent to 100% intensity
void updatePixel(short x, short y, unsigned short colour, unsigned short intensity)
{
short current;
short red0,green0,blue0;
short red1,green1,blue1;
unsigned short oldintensity;
    // check it's valid space
    if( (x>=0)&&(x<240)&&(y>=0)&&(y<240))     
    {
        if (intensity>245)  // if more than 95% just assume 100%
        {
            RenderSpace[x+y*240]=colour;
        }
        else
        {
            // work out the reletive intesities of red green and blue from the old and new pixels.
            oldintensity = 256 - intensity;

            //printf("new,old  intensity is %d     %d\n",newintensity,oldintensity);

            current = RenderSpace[x+y*240];

            red0 =   (current&0xf800)>>11;
            green0 = (current&0x07E0)>>5;
            blue0 =  (current&0x001F);

            // update the colour but only if different
            red1 = (colour&0xf800)>>11;
            green1 = (colour&0x07E0)>>5;
            blue1 = (colour&0x001F);

            //mix them
            red0 =   ((red1   * intensity) + (red0   * oldintensity) +128)>>8;
            if (red0>0x1F)
                red0 = 0x1F;
            green0 = ((green1 * intensity) + (green0 * oldintensity) +128)>>8;
            if (green0>0x3F)
                green0 = 0x3F;
            blue0 =  ((blue1  * intensity) + (blue0  * oldintensity) +128)>>8;
            if (blue0>0x1F)
                blue0 = 0x1F;
            //write it back

            RenderSpace[x+y*240]=(red0<<11)+(green0<<5)+(blue0);
        }
    }
}



// support functions for the floating point Drawline algorythm
// fractional part of x
float fpart(float x)
{
    return (x - floor(x));
}

float rfpart(float x)
{
    return (1 - fpart(x));
}


// these are equivalent to the above but are based on integer maths with a fix position decimal at the 16bit boundary
unsigned int fpartI(int x)      // x is 16 bits to 1
{
    return ((unsigned int)((x&0xffff)>>8));
}

unsigned int rfpartI(int x)     // x is 16 bits to 1
{
    return (256 - (unsigned int)((x&0xffff)>>8));
}


// python easy call for access using shorts.  See below for actual algorythm
void DrawLineAA(short x0,short y0, short x1, short y1, unsigned short colour)
{
    DrawLineFloat(x0, y0, x1, y1,colour, false);
}


//version of the line drawing routine that uses floating point mathematics including anti aliasing
// this code is based on Xiaolin Wu's algorythm, details of which can be found at:
// https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
// this is slower than the raw integer version shown below, but does look much cleaner on the display
void DrawLineFloat(float x0,float y0, float x1, float y1, unsigned short colour,bool fill)
{
bool steep =false;
float temp;
float deltax,deltay;
int gradient ;
float xend,yend;
float xgap;
float xpxl1,ypxl1,xpxl2,ypxl2;
int x;
int intery;

    if(abs(y1 - y0) > abs(x1 - x0))
        steep = true;
    
    if (steep)      // Y or X as the main direction Steep = true if more Y than X change
    {
        temp = y0;
        y0 = x0;
        x0 = temp;

        temp = y1;
        y1 = x1;
        x1 = temp;
    }

    if (x0 > x1) 
    {
        temp = x1;
        x1 = x0;
        x0 = temp;

        temp = y1;
        y1 = y0;
        y0 = temp;
    }
    
    deltax = x1 - x0;
    deltay = y1 - y0;

    if (deltax == 0.0)
        gradient  = 65536;      // 16 bit nominal 1.0
    else
        gradient  = (int)((deltay/deltax)*65536.0);

    // handle first endpoint
    xend = round(x0);
    yend = y0 + ((float)gradient)/65536.0  * (xend - x0);
    xgap = rfpart(x0 + 0.5);
    xpxl1 = xend;     // this will be used in the main loop
    ypxl1 = floor(yend);

    yend = yend*65536;
    if (steep)
    {
        updatePixel(ypxl1,   xpxl1,  colour, rfpartI(yend) * xgap);
        updatePixel(ypxl1+1, xpxl1,  colour, fpartI (yend) * xgap);
    }
    else
    {
        updatePixel(xpxl1, ypxl1  ,  colour, rfpartI(yend) * xgap);
        updatePixel(xpxl1, ypxl1+1,  colour, fpartI(yend) * xgap);
    }
    intery = (int)(yend + gradient); // first y-intersection for the main loop
    
    // handle second endpoint
    xend = round(x1);
    yend = y1 + ((float)gradient)/65536.0 * (xend - x1);
    xgap = fpart(x1 + 0.5);
    xpxl2 = xend; //this will be used in the main loop
    ypxl2 = floor(yend);

    yend = yend*65536;
    if (steep)
    {
        updatePixel(ypxl2  , xpxl2, colour, rfpartI(yend) * xgap);
        updatePixel(ypxl2+1, xpxl2, colour, fpartI(yend) * xgap);
    }
    else
    {
        updatePixel(xpxl2, ypxl2,   colour, rfpartI(yend) * xgap);
        updatePixel(xpxl2, ypxl2+1, colour, fpartI(yend) * xgap);
    }
    
    if (fill)
    {
        // main loop  but for fill case
        if (steep)
        {
            //printf("steep line\n");
            for (x = xpxl1 + 1; x < xpxl2; x++)
            {
                SetPixel(intery>>16  , x, colour);
                SetPixel((intery>>16)+1, x, colour);
                intery = intery + gradient;
            }
        }
        else
        {
            //printf("not steep line\n");
            for (x = xpxl1 + 1;x < xpxl2; x++)
            { 
                SetPixel(x, intery>>16,  colour);
                SetPixel(x, (intery>>16)+1,colour);
                intery = intery + gradient;
            } 
        
        }
    }
    else
    {
        // main loop  but for single line case
        if (steep)
        {
            //printf("steep line\n");
            for (x = xpxl1 + 1; x < xpxl2; x++)
            {
                updatePixel(intery>>16  , x, colour, rfpartI(intery));
                updatePixel((intery>>16)+1, x, colour, fpartI(intery));
                intery = intery + gradient;
            }
        }
        else
        {
            //printf("not steep line\n");
            for (x = xpxl1 + 1;x < xpxl2; x++)
            { 
                updatePixel(x, intery>>16,  colour, rfpartI(intery));
                updatePixel(x, (intery>>16)+1,colour, fpartI(intery));
                intery = intery + gradient;
            } 
        
        }
    }
}


// DrawLine
// simple line drawing in the render space
// allows for lines on and off screen
// Bresenham's Line Drawing Algorithm is used for this, which provides a fast integer only drawing routine
// https://www.includehelp.com/computer-graphics/bresenhams-line-drawing-algorithm.aspx explains the logic behind it
// this has however been optimised for direction and for X or Y priority
void DrawLineIntMaths(short Xstart,short Ystart, short Xend, short Yend, unsigned short colour)
{
short Xerr,Yerr;
short Xdelta,Ydelta,Step;
short incX,incXY,incY;
short x,y;
short dir;


    //printf("OLD CODE\n");
    Xdelta = Xend-Xstart;
    Ydelta = Yend-Ystart;


    if (abs(Xdelta)>abs(Ydelta))
    {
        // X direction is larger or equal  

        // make sure the lines always go from lowX to high X
        // makes the rest of the code simpler
        if(Xstart>Xend)
        {
            short temp=Xend;
            Xend = Xstart;
            Xstart=temp;

            temp = Yend;
            Yend= Ystart;
            Ystart = temp;
        }

        Xdelta = Xend-Xstart;
        Ydelta = Yend-Ystart;

        x = Xstart;
        y = Ystart;

        if (Yend<Ystart)
        {
            dir =-1;
            Ydelta = -Ydelta;
        }
        else
        {
            dir=1;       
        }
        incX  = 2 * Ydelta;
        incXY = 2 * (Ydelta - Xdelta);
        Step  = 2 * Ydelta - Xdelta;

        // draw the first pixel and possibly the only one
        SetPixel(x,y,colour);

        while (x<Xend)
        {
            if (Step<=0)
            {
                Step+=incX;
                x++;
            }
            else
            {
                Step+=incXY;
                x++;
                y+=dir;
            }
            SetPixel(x,y,colour);
        }
    }
    else
    {
        // y Direction is the mayority
        if(Ystart>Yend)
        {
            short temp=Xend;
            Xend = Xstart;
            Xstart=temp;

            temp = Yend;
            Yend= Ystart;
            Ystart = temp;
        }

        Xdelta = Xend-Xstart;
        Ydelta = Yend-Ystart;

        x = Xstart;
        y = Ystart;

        if (Xend<Xstart)
        {
            dir =-1;
            Xdelta = -Xdelta;
        }
        else
        {
            dir=1;       
        }
        incY  = 2 * Xdelta;
        incXY = 2 * (Xdelta - Ydelta);
        Step  = 2 * Xdelta - Ydelta;

        // draw the first pixel and possibly the only one
        SetPixel(x,y,colour);

        while (y<Yend)
        {
            if (Step<=0)
            {
                Step+=incY;
                y++;
            }
            else
            {
                Step+=incXY;
                y++;
                x+=dir;
            }
            SetPixel(x,y,colour);
        }
    }
}

// routines for drawing lines wider than 1 pixel
// the first provides a fast integer parameter version which is good for calling in python
// the second is the one that does the actual work.
void DrawLineWideAA(short x0,short y0, short x1, short y1, unsigned short colour,unsigned short width)
{
    DrawLineWideFloat (x0,y0 , x1,y1, colour, width);
}

void  DrawLineWideFloat(float x0,float y0, float x1, float y1, unsigned short colour,unsigned short width)
{                            
float dx,dy,sx,sy;
float length,err;
float xoff,yoff;
float xend,yend;
float step = 1.5;

    dx = (x1-x0);
    dy = (y1-y0);

    length = sqrt(dx*dx+dy*dy);        

    step = (abs(dx) + abs (dy))/length;                   

    if (width <= 1 || length == 0) 
    {
        DrawLineFloat(x0,y0, x1,y1,colour,false);                    
    }
    else
    {
        dx = dx/(length) / step;
        dy = -dy/(length) / step;    //flip by 90deg

        width = width * step;

        xoff = -dy*(width)/2 +dy;
        yoff = -dx*(width)/2 +dx;
        for (int line = 0;line<(width-1);line++)
        {
            //DrawLineFloat(x0+xoff,y0+yoff,x1+xoff,y1+yoff,colour);
            DrawLineFloat(x0+xoff,y0+yoff,x1+xoff,y1+yoff,colour,true);
            xoff += dy;
            yoff += dx;
        }
        // run antialiased edges
        xoff = -dy*(width)/2;
        yoff = -dx*(width)/2;
        
        xend = +dy*(width)/2;
        yend = +dx*(width)/2;
        //draw long edges
        DrawLineFloat(x0+xoff,y0+yoff,x1+xoff,y1+yoff,colour,false);
        DrawLineFloat(x0+xend,y0+yend,x1+xend,y1+yend,colour,false);

        //DrawLineFloat(x0+xoff,y0+yoff,x0+xend,y0+yend,colour,false);
        //DrawLineFloat(x1+xoff,y1+yoff,x1+xend,y1+yend,colour,false);
    
   }
}



// ScreenUpdate
// routine to do the write to the screen as a memory dump from the render space
void ScreenUpdate(void)
{
    int i;
    unsigned short * sourcePtr;

    if (RenderSpace !=NULL)
    {
        sourcePtr = RenderSpace;
        SetScreenWriteArea(0,0,239,239);   // 240x240
        bcm2835_gpio_write(CIR_DC, HIGH);  
        for (i=0;i<(240*240);i++)
        {
            bcm2835_spi_transfer((*sourcePtr)>>8);
            bcm2835_spi_transfer((*sourcePtr)&0xff);
            sourcePtr++;
        }
    }
}

// convert a 8 byte set of R G B values into the 16 bit combined 5 Red 6 Green and 5 Blue 
// patten that is used by the display chip
unsigned short RGBto16bit(unsigned char Red, unsigned char Green, unsigned char Blue)
{
    return ( ((Red>>3)<<11) | ((Green>>2)<<5) | (Blue>>3));
}

