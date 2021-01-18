# simple application to show the functionality of the C driver on the circular display
#
#
#  P.Worman Jan 2021
#
#
#  see https://simpaul.com/round_display for details
#

import datetime

# load in the ability to use c variable types 
from ctypes import *




# draw a circle in python, but use the individual pixel setting function in the C code
def DrawCirclePython(x0, y0, r, colour):
  a=0
  b=r	  
  while(a<=b):
    circularDisp.SetPixelDirect(x0-b,y0-a,colour)
    circularDisp.SetPixelDirect(x0+b,y0-a,colour)                   
    circularDisp.SetPixelDirect(x0-a,y0+b,colour)                         
    circularDisp.SetPixelDirect(x0-a,y0-b,colour)           
    circularDisp.SetPixelDirect(x0+b,y0+a,colour)                        
    circularDisp.SetPixelDirect(x0+a,y0-b,colour)       
    circularDisp.SetPixelDirect(x0+a,y0+b,colour)         
    circularDisp.SetPixelDirect(x0-b,y0+a,colour)         
    a+=1
    if((a*a+b*b)>(r*r)):
      b-=1


# load the Shared Library for the direct I/O 
circularDisp = CDLL("./bcm_direct_c2py.so")


# simple python code to show a selection of functions in the C library
print ("testing")

# initialise the hardware driver
if (circularDisp.initBCMHardware()):
  # then send the commands to configure the display
  circularDisp.initCircularDisp()

  # clear the screen with a full screen solid colour
  circularDisp.clearScreenDirect(0x07E0)    # this is green


  # some timing to see how fast it can work
  now = datetime.datetime.now()
  print("Circle calc start: ")
  print(str(now))

  # draw 100 circles using python code
  for radius in range(100):
  	DrawCirclePython(120,120,radius+10,radius<<5)
  
  # some timing to see how fast it can work
  now = datetime.datetime.now()
  print("Circle calc End: ")
  print(str(now))

  # now do a number of circles direct using the C library
  for offset in range(10):
    for radius in range(100):
      circularDisp.DrawCircle(120,120,radius+10,radius<<offset)
    # only update the screen after each 100 circles are drawn  
    circularDisp.ScreenUpdate()


  #  for offset in range(10):
  #    for radius in range(200):
  #      circularDisp.DrawCircle(radius,radius,radius+10,radius<<offset)
  #      if(radius&0x0f==0):
  #        circularDisp.ScreenUpdate()


  # a few simple line drawing
  for pos in range(0,240):
    circularDisp.DrawLine(pos,0,0,240-pos,pos)
    if(pos&0x07==0):
      # only do screen update every 8th line
      circularDisp.ScreenUpdate()

  # a rectange test
  for pos in range(0,71):
    circularDisp.DrawRectangle(pos+50,pos+50,190-pos,190-pos,pos)
    if(pos&0x07==0):
      # only do screen update every 8th line
      circularDisp.ScreenUpdate()

  # anotehr update just to make sure everything is updated on screen
  circularDisp.ScreenUpdate()

  # now lets try a bitmap
  file = open("./book240x240x24.bmp","rb")

  data = file.read()

  # this code does the bitmap image but with the decoding and individual bit setting done in python
  # it's slower but could be useful if you want to try some other formats
  #
  #  if ((data[10]!=54) or (data[14]!=40) or (data[18]!=240) or (data[22]!=240) or (data[19]!=0) or (data[23]!=0) or (data[28]!=24)):
  #    print("not a 240x240 image")
  #  else:
  #    print("valid file format")
  #    counter = 54
  #    for y in range (0,240):
  #      for x in range( 0,240):
  #        colour  = circularDisp.RGBto16bit(data[counter+2],data[counter+1],data[counter]);
  #        circularDisp.SetPixel(x,239-y,colour)
  #        counter=counter+3
  #    circularDisp.ScreenUpdate()

  file.close()

  # Clear the screen again this time RED
  circularDisp.clearScreenDirect(0xF800)

  # now do a screen update with the BMP data and display it
  circularDisp.RGB240x240Direct(data,1)


  circularDisp.exitBCMHardware()
else:
  print ("failed to imitialise the hardware - probably not running as root")

print ("done")

