# This is sample code specifically for blog post
# the code itself does work but needs the right setup and cabling
# see https://simpaul.com/round-display/



import RPi.GPIO as gpio
from time import time, sleep

import datetime

import array as arr

from PIL import Image



# values are for RPI pin number

# these values are the BCM eqivalents
CIR_SCL = 11
CIR_SDA = 10
CIR_RES = 25
CIR_DC  = 24
CIR_CS  = 8


USE_HORIZONTAL = 0  # Set the display direction 0,1,2,3    four directions
					# 1 top of screen is 90 CW from connector
					# 2 top of screen is by connector 
					# 3 connector at bottom of screen

class circularDisp ():
  def __init__(self):
    print ("circular class init")


  def setup(self):
    print ("circular setup start")

    gpio.setmode(gpio.BCM)  

    # setup the reset and pull it low to put it into reset state
    gpio.setup(CIR_RES, gpio.OUT)
    gpio.output(CIR_RES, gpio.LOW)

    #setup the clock pin as output and default to low
    gpio.setup(CIR_SCL, gpio.OUT)
    gpio.output(CIR_SCL, gpio.LOW)

    #setup the data pin as output and default to low
    gpio.setup(CIR_SDA, gpio.OUT)
    gpio.output(CIR_SDA, gpio.LOW)

    #setup the chip select pin as output and default to high (not selected)
    gpio.setup(CIR_CS, gpio.OUT)
    gpio.output(CIR_CS, gpio.HIGH)

    #setup the Data/Command select pin as output and default to high (Data)
    gpio.setup(CIR_DC, gpio.OUT)
    gpio.output(CIR_DC, gpio.HIGH)

    sleep(0.02)    #20ms 
    gpio.output(CIR_RES, gpio.HIGH)
    sleep(0.12)    #120ms 

 #************* Start Initial Sequence **********// 
    #self.sdoCmdU8(0xEF)		# inter register enable P172  ***  not needed
 
    self.sdoCmdU8(0xEB)		# *** not listed
    self.sdoDataU8(0x14) 
    
    self.sdoCmdU8(0xFE) 	# inter register enable 1 P172         
    self.sdoCmdU8(0xEF) 	# inter register enable 2 P173

    self.sdoCmdU8(0xEB)    	# *** not listed
    self.sdoDataU8(0x14) 

    self.sdoCmdU8(0x84)   	# *** not listed     
    self.sdoDataU8(0x40) 

    self.sdoCmdU8(0x85)     # *** not listed       
    self.sdoDataU8(0xFF) 

    self.sdoCmdU8(0x86)     # *** not listed   
    self.sdoDataU8(0xFF) 

    self.sdoCmdU8(0x87)     # *** not listed             
    self.sdoDataU8(0xFF)

    self.sdoCmdU8(0x88)            
    self.sdoDataU8(0x0A)

    self.sdoCmdU8(0x89)            
    self.sdoDataU8(0x21) 

    self.sdoCmdU8(0x8A)            
    self.sdoDataU8(0x00) 

    self.sdoCmdU8(0x8B)            
    self.sdoDataU8(0x80) 

    self.sdoCmdU8(0x8C)            
    self.sdoDataU8(0x01) 

    self.sdoCmdU8(0x8D)            
    self.sdoDataU8(0x01) 

    self.sdoCmdU8(0x8E)            
    self.sdoDataU8(0xFF) 

    self.sdoCmdU8(0x8F)            
    self.sdoDataU8(0xFF) 


    self.sdoCmdU8(0xB6)		# Display function control P158
    self.sdoDataU8(0x00)    # Must be zero         
    self.sdoDataU8(0x00) 	# Defines shift register directions

    self.sdoCmdU8(0x36)		#Memory access control datasheet P127 
    if(USE_HORIZONTAL==0):
      self.sdoDataU8(0xE8)	#sample code showed this a 18 but that didn't work
    if(USE_HORIZONTAL==1):
      self.sdoDataU8(0x28)
    if(USE_HORIZONTAL==2):
      self.sdoDataU8(0x48)
    if(USE_HORIZONTAL==3):
      self.sdoDataU8(0x88)

    self.sdoCmdU8(0x3A)  	# COLMOD Pixel Fomrat Set  P135        
    self.sdoDataU8(0x55) 	# RGB Mode Ignored, MCU Mode set to 16 Bits per Pixel
    						# colour is 5 Red, 6 Green, 5 Blue


    self.sdoCmdU8(0x90)     # *** not listed            
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x08) 

    self.sdoCmdU8(0xBD)  	# *** not listed             
    self.sdoDataU8(0x06)

    self.sdoCmdU8(0xBC)   	# *** not listed         
    self.sdoDataU8(0x00)    

    self.sdoCmdU8(0xFF)   	# *** not listed           
    self.sdoDataU8(0x60)
    self.sdoDataU8(0x01)
    self.sdoDataU8(0x04)

    self.sdoCmdU8(0xC3) 	# Power Control 2 P 168          
    self.sdoDataU8(0x13)
    self.sdoCmdU8(0xC4)     # Power Control 3 P 169        
    self.sdoDataU8(0x13)

    self.sdoCmdU8(0xC9)     # Power Control 4 P 170        
    self.sdoDataU8(0x22)

    self.sdoCmdU8(0xBE)   	# *** not listed           
    self.sdoDataU8(0x11) 

    self.sdoCmdU8(0xE1)     # *** not listed       
    self.sdoDataU8(0x10)
    self.sdoDataU8(0x0E)

    self.sdoCmdU8(0xDF)  	# *** not listed     
    self.sdoDataU8(0x21)
    self.sdoDataU8(0x0c)
    self.sdoDataU8(0x02)

    self.sdoCmdU8(0xF0)   	# SET_GAMMA1  P 174
    self.sdoDataU8(0x45)
    self.sdoDataU8(0x09)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x26)
    self.sdoDataU8(0x2A)

    self.sdoCmdU8(0xF1)   	# SET_GAMMA2  P 176 
    self.sdoDataU8(0x43)
    self.sdoDataU8(0x70)
    self.sdoDataU8(0x72)
    self.sdoDataU8(0x36)
    self.sdoDataU8(0x37)  
    self.sdoDataU8(0x6F)


    self.sdoCmdU8(0xF2)   	# SET_GAMMA3  P 178 
    self.sdoDataU8(0x45)
    self.sdoDataU8(0x09)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x26)
    self.sdoDataU8(0x2A)

    self.sdoCmdU8(0xF3)  	# SET_GAMMA4  P 180
    self.sdoDataU8(0x43)
    self.sdoDataU8(0x70)
    self.sdoDataU8(0x72)
    self.sdoDataU8(0x36)
    self.sdoDataU8(0x37) 
    self.sdoDataU8(0x6F)

    self.sdoCmdU8(0xED) 	# *** not listed      
    self.sdoDataU8(0x1B) 
    self.sdoDataU8(0x0B) 

    self.sdoCmdU8(0xAE)  	# *** not listed          
    self.sdoDataU8(0x77)

    self.sdoCmdU8(0xCD)  	# *** not listed           
    self.sdoDataU8(0x63)        


    self.sdoCmdU8(0x70)  	# *** not listed           
    self.sdoDataU8(0x07)
    self.sdoDataU8(0x07)
    self.sdoDataU8(0x04)
    self.sdoDataU8(0x0E) 
    self.sdoDataU8(0x0F) 
    self.sdoDataU8(0x09)
    self.sdoDataU8(0x07)
    self.sdoDataU8(0x08)
    self.sdoDataU8(0x03)

    self.sdoCmdU8(0xE8) 	# Frame Rate P164           
    self.sdoDataU8(0x34)	# 4 dot inversion

    self.sdoCmdU8(0x62)  	# *** not listed          
    self.sdoDataU8(0x18)
    self.sdoDataU8(0x0D)
    self.sdoDataU8(0x71)
    self.sdoDataU8(0xED)
    self.sdoDataU8(0x70) 
    self.sdoDataU8(0x70)
    self.sdoDataU8(0x18)
    self.sdoDataU8(0x0F)
    self.sdoDataU8(0x71)
    self.sdoDataU8(0xEF)
    self.sdoDataU8(0x70) 
    self.sdoDataU8(0x70)

    self.sdoCmdU8(0x63) 	# *** not listed             
    self.sdoDataU8(0x18)
    self.sdoDataU8(0x11)
    self.sdoDataU8(0x71)
    self.sdoDataU8(0xF1)
    self.sdoDataU8(0x70) 
    self.sdoDataU8(0x70)
    self.sdoDataU8(0x18)
    self.sdoDataU8(0x13)
    self.sdoDataU8(0x71)
    self.sdoDataU8(0xF3)
    self.sdoDataU8(0x70) 
    self.sdoDataU8(0x70)

    self.sdoCmdU8(0x64) 	# *** not listed             
    self.sdoDataU8(0x28)
    self.sdoDataU8(0x29)
    self.sdoDataU8(0xF1)
    self.sdoDataU8(0x01)
    self.sdoDataU8(0xF1)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x07)

    self.sdoCmdU8(0x66)  	# *** not listed            
    self.sdoDataU8(0x3C)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0xCD)
    self.sdoDataU8(0x67)
    self.sdoDataU8(0x45)
    self.sdoDataU8(0x45)
    self.sdoDataU8(0x10)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x00)

    self.sdoCmdU8(0x67)		# *** not listed              
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x3C)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x00)
    self.sdoDataU8(0x01)
    self.sdoDataU8(0x54)
    self.sdoDataU8(0x10)
    self.sdoDataU8(0x32)
    self.sdoDataU8(0x98)

    self.sdoCmdU8(0x74)  	# *** not listed            
    self.sdoDataU8(0x10)    
    self.sdoDataU8(0x85)    
    self.sdoDataU8(0x80)
    self.sdoDataU8(0x00) 
    self.sdoDataU8(0x00) 
    self.sdoDataU8(0x4E)
    self.sdoDataU8(0x00)                    

    self.sdoCmdU8(0x98) 	# *** not listed             
    self.sdoDataU8(0x3e)
    self.sdoDataU8(0x07)

    self.sdoCmdU8(0x35)   	# Tearing Effect Line ON P125
    self.sdoDataU8(0x01) 	# turned on.

    self.sdoCmdU8(0x21)		#   Invert screen colours

    self.sdoCmdU8(0x11)		# Sleep Out Mode P103	- turns off sleep mode
    sleep(0.120)
    self.sdoCmdU8(0x29)		# Display On P 110
    sleep(0.020)

    print ("circular setup complete")



    #some test cases
    #self.sdoCmdU8(0x21)		# invert screen
    

    #self.sdoCmdU8(0x51) 	# Display brightness   - doesn't seem to do anything         
    #self.sdoDataU8(0xF0)


  # basic bit banging for the 8 but write
  def  sdoDataU8(self,n):
    #should alrady be output but just in case  
    #gpio.setup(CIR_SDA, gpio.OUT)
    gpio.output(CIR_CS, gpio.LOW)
    gpio.output(CIR_DC, gpio.HIGH)

    for bitcount in range (0,8):
      gpio.output(CIR_SCL, gpio.LOW)
      if (n & 0x80):
        gpio.output(CIR_SDA, gpio.HIGH)
      else:
        gpio.output(CIR_SDA, gpio.LOW)
      
      n = n <<1
      gpio.output(CIR_SCL, gpio.HIGH)

    #set the chip select pin high as end case
    gpio.output(CIR_CS, gpio.HIGH)


 # refence code is 13 seconds to run
 # hard coding and no loops saves at most 1 second

  def  sdoDataU16(self,n):
    #should already be output but just in case  
    #gpio.setup(CIR_SDA, gpio.OUT)
    gpio.output(CIR_CS, gpio.LOW)
    gpio.output(CIR_DC, gpio.HIGH)
    
    for bitcount in range (0,16):
      gpio.output(CIR_SCL, gpio.LOW)
      if (n & 0x8000):
        gpio.output(CIR_SDA, gpio.HIGH)
      else:
        gpio.output(CIR_SDA, gpio.LOW)
      
      n = n <<1
      gpio.output(CIR_SCL, gpio.HIGH)

    #set the chip select pin high as end case
    gpio.output(CIR_CS, gpio.HIGH)




  def  sdoCmdU8(self,n):
    #should alrady be output but just in case  
    #gpio.setup(CIR_SDA, gpio.OUT)
    gpio.output(CIR_CS, gpio.LOW)
    gpio.output(CIR_DC, gpio.LOW)

    for bitcount in range (0,8):
      gpio.output(CIR_SCL, gpio.LOW)
      if (n & 0x80):
        gpio.output(CIR_SDA, gpio.HIGH)
      else:
        gpio.output(CIR_SDA, gpio.LOW)
      
      n = n <<1
      gpio.output(CIR_SCL, gpio.HIGH)

    #set the chip select pin high as end case
    gpio.output(CIR_CS, gpio.HIGH)


  def clearScreen(self, bColor):
    self.SetPos(0,0,239,239) # 240x240
    for i in range (0,(240*240)):
      self.sdoDataU16(bColor)

  def ShadedScreen(self):

    self.SetPos(0,0,239,239) # 240x240

    gpio.output(CIR_CS, gpio.LOW)
    gpio.output(CIR_DC, gpio.HIGH)
    for i in range (0,(240*240)):
      n = i*3
      for bitcount in range (0,16):
        gpio.output(CIR_SCL, gpio.LOW)
        if (n & 0x8000):
          gpio.output(CIR_SDA, gpio.HIGH)
        else:
          gpio.output(CIR_SDA, gpio.LOW)
      
        n = n <<1
        gpio.output(CIR_SCL, gpio.HIGH)

    #set the chip select pin high as end case
    gpio.output(CIR_CS, gpio.HIGH)
      
	
  def rectangletest(self, bColor):

    self.SetPos(50,100,100,120) # 240x240
    for i in range (0,(51*21)):
      self.sdoDataU16(bColor)

    self.SetPos(101,101,150,120) # 240x240
    for i in range (0,(50*20)):
      self.sdoDataU16(0x001F)

    self.SetPos(101,151,150,127) # 240x240
    for i in range (0,(50*20)):
      self.sdoDataU16(0x07E0)


  #defines an area to write in the screen
  #the co-ordinates are INCLUSIVE  i.e. include both start and end pixels
  def SetPos(self,Xstart,Ystart,Xend,Yend):
    self.sdoCmdU8(0x2a)				# select Column address P111
    self.sdoDataU16(Xstart)
    self.sdoDataU16(Xend)

    self.sdoCmdU8(0x2b)   			# select Row Address Set P113
    self.sdoDataU16(Ystart)
    self.sdoDataU16(Yend)

    self.sdoCmdU8(0x2c) 			# Memory Write P 115
    								# sets the display to receive data


  def DrawPoint(self, x, y, colour):
    self.SetPos(x,y,x,y)
    self.sdoDataU16(colour)

  def DrawLine(self, x1, y1, x2, y2, colour):
    xerr=0
    yerr=0
    distance=0
    delta_x=x2-x1 
    delta_y=y2-y1
    uRow=x1
    uCol=y1
    if(delta_x>0):
      incx=1 
    else:
      if (delta_x==0):
        incx=0
      else:
        incx=-1
        delta_x=-delta_x

    if(delta_y>0):
      incy=1
    else:
      if (delta_y==0):
        incy=0
      else:
        incy=-1
        delta_y=-delta_x

    if(delta_x>delta_y):
      distance=delta_x  
    else:
      distance=delta_y
    for t in range (0,distance+1):
      self.DrawPoint(uRow,uCol,colour)
      xerr+=delta_x
      yerr+=delta_y
      if(xerr>distance):
        xerr-=distance
        uRow+=incx
      if(yerr>distance):
        yerr-=distance
        uCol+=incy


  def DrawRectangle(self, x1, y1, x2, y2, colour):
    self.DrawLine(x1,y1,x2,y1,colour)
    self.DrawLine(x1,y1,x1,y2,colour)
    self.DrawLine(x1,y2,x2,y2,colour)
    self.DrawLine(x2,y1,x2,y2,colour)


  def DrawCircle(self, x0, y0, r, colour):
    a=0
    b=r	  
    while(a<=b):
      self.DrawPoint(x0-b,y0-a,colour)
      self.DrawPoint(x0+b,y0-a,colour)                     
      self.DrawPoint(x0-a,y0+b,colour)                          
      self.DrawPoint(x0-a,y0-b,colour)             
      self.DrawPoint(x0+b,y0+a,colour)                         
      self.DrawPoint(x0+a,y0-b,colour)          
      self.DrawPoint(x0+a,y0+b,colour)          
      self.DrawPoint(x0-b,y0+a,colour)          
      a+=1
      if((a*a+b*b)>(r*r)):
        b-=1

  # does the calcs but no actual writes
  def DrawCircleTX(self, x0, y0, r, colour):
    img = Image.new( 'RGB', (240,240), "black") # Create a new black image
    pixels = img.load() # Create the pixel map
    a=0
    b=r	  
    while(a<=b):
      pixels[x0-b,y0-a]=(0,0,colour)
      pixels[x0+b,y0-a]=(0,0,colour)                   
      pixels[x0-a,y0+b]=(0,0,colour)                         
      pixels[x0-a,y0-b]=(0,0,colour)           
      pixels[x0+b,y0+a]=(0,0,colour)                        
      pixels[x0+a,y0-b]=(0,0,colour)       
      pixels[x0+a,y0+b]=(0,0,colour)         
      pixels[x0-b,y0+a]=(0,0,colour)         
      a+=1
      if((a*a+b*b)>(r*r)):
        b-=1


  def CreateImage(self):
    img = Image.new( 'RGB', (240,240), "black") # Create a new black image
    pixels = img.load() # Create the pixel map
    for i in range(img.size[0]):    # For every pixel:
        for j in range(img.size[1]):
            pixels[i,j] = (i, j, 100) # Set the colour accordingly

    img.show()
 



#main code - test


now = datetime.datetime.now()
print("Current date and time: ")
print(str(now))
display = circularDisp()
display.setup()
display.clearScreen(0xF800)
#display.ShadedScreen()

display.rectangletest(0xFFFF)
display.DrawCircle(120,120,100,0xffff)
display.DrawLine(0,0,240,240,0x0000)
display.DrawRectangle(80,40,160,80,0x0000)
now = datetime.datetime.now()
print("Current date and time: ")
print(str(now))
display.CreateImage()
now = datetime.datetime.now()
print("Current date and time: ")
print(str(now))


now = datetime.datetime.now()
print("Circle start: ")
print(str(now))
for radius in range(100):
	display.DrawCircle(120,120,radius+10,radius)
	now = datetime.datetime.now()
print("Circle End: ")
print(str(now))

now = datetime.datetime.now()
print("Circle calc start: ")
print(str(now))
for radius in range(100):
	display.DrawCircleTX(120,120,radius+10,radius)
	now = datetime.datetime.now()
print("Circle calc End: ")
print(str(now))