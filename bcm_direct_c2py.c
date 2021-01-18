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
    else
        printf("trying to write outside screen\n");
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

    // first the two horrizonal lines
    DrawLine(Xstart,Ystart,Xend,Ystart,colour);
    DrawLine(Xstart,Yend,Xend,Yend,colour);
    // then the two vertical ones
    DrawLine(Xstart,Ystart,Xstart,Yend,colour);
    DrawLine(Xend,Ystart,Xend,Yend,colour);

}


// DrawLine
// simple line drawing in the render space
// allows for lines on and off screen
void DrawLine(short Xstart,short Ystart, short Xend, short Yend, unsigned short colour)
{
    short Xerr=0;
    short Yerr=0;
    short distance=0;
    short delta_x=Xend-Xstart; 
    short delta_y=Yend-Ystart;
    short uRow=Xstart;
    short uCol=Ystart;
    short incx,incy;
    if(delta_x>0)
      incx=1;
    else 
        if (delta_x==0)
            incx=0;
        else
        {
            incx=-1;
            delta_x=-delta_x;
        }

    if(delta_y>0)
        incy=1;
    else
        if (delta_y==0)
            incy=0;
        else
        {
            incy=-1;
            delta_y=-delta_y;
        }

    if(delta_x>delta_y)
        distance=delta_x;
    else
        distance=delta_y;
    for (int t=0;t<=(distance+1);t++)
    {
        SetPixel(uRow,uCol,colour);
        Xerr+=delta_x;
        Yerr+=delta_y;
        if(Xerr>distance)
        {
            Xerr-=distance;
            uRow+=incx;
        }
        if(Yerr>distance)
        {
            Yerr-=distance;
            uCol+=incy;
        }
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

