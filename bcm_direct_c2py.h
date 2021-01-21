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


// setup and exit commands

bool initBCMHardware(void);
void initCircularDisp(void);
void exitBCMHardware(void);


// direct screen update commands 
void clearScreenDirect(unsigned short bcolour);
void SetPixelDirect(unsigned short xpos, unsigned short ypos, unsigned short colour);

//display a 240x240 bmp encoded 24 bit image
void RGB240x240Direct(unsigned char * rawdata,bool update );


// the following are useful if you are having background image and then overlaying details (such as watch hands etc)
// copies the refernce current render space to a refernce space
void SetRefernceImage(void);
//restore the reference to the render space
void RestoreReferenceImage(void);


// offline (to render space) graphics commands
// co-ordinates can be on or off screen and the visible parts will still be shown
void DrawCircle (short x0, short y0, short r, unsigned short colour);
void DrawRectangle(short Xstart,short Ystart, short Xend, short Yend, unsigned short colour);


void SetPixel(short xpos, short ypos, unsigned short colour);


// two line drawing routines, the first is for integer maths but give jagged lines
// the second uses anti-aliasing for smooth edges, but at the expense of speed

void DrawLineIntMaths (short Xstart,short Ystart, short Xend, short Yend, unsigned short colour);
void DrawLineAA       (short x0,short y0, short x1, short y1, unsigned short colour);

// this is the actual routine, but passing floats seems to have complications in Python so, best to use the above.
void DrawLineFloat    (float Xstart,float Ystart, float Xend, float Yend, unsigned short colour,bool fill);

//wide line drawing routines, the first being used by Python and the second the actual routine
void DrawLineWideAA(short x0,short y0, short x1, short y1, unsigned short colour,unsigned short width);
void DrawLineWideFloat(float x0,float y0, float x1, float y1, unsigned short colour,unsigned short width);

// update the screen with the changes to the renderspace
void ScreenUpdate(void);


// utility to convert the 8bit indiviual RGB values to a 16 bit combined value
unsigned short RGBto16bit(unsigned char Red, unsigned char Green, unsigned char Blue);


// low level commands used for direct access
// these are only needed if the functions above do not allow the features you need

void SetScreenWriteArea(unsigned char Xstart,unsigned char Ystart,unsigned char Xend,unsigned char Yend);
void sdoCmdU8( unsigned char byteval);
void sdoDataU16( unsigned short intval);
void sdoDataU8( unsigned char byteval);

