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
  circularDisp.clearScreenDirect(0x0000)    # this is green

  # select which test to run
  test =2
  
  # some timing to see how fast it can work
  starttime = datetime.datetime.now()

  if(test==1):

    # do the AA line testing
    for centre in range (10):
      for radius in range(240):
        circularDisp.DrawLineIntMaths(12*centre,12*centre,radius,0,radius<<11)
        circularDisp.DrawLineIntMaths(12*centre,12*centre,0,radius,(radius<<5)&0x07E0)
        circularDisp.DrawLineIntMaths(12*centre,12*centre,radius,239,radius&0x1F)
        circularDisp.DrawLineIntMaths(12*centre,12*centre,239,radius,0xffff>>centre)
      #circularDisp.ScreenUpdate()


  if(test==2):

    # do the AA line testing
    for centre in range (10):
      for radius in range(240):
        circularDisp.DrawLineAA(12*centre,12*centre,radius,0,radius<<11)
        circularDisp.DrawLineAA(12*centre,12*centre,0,radius,(radius<<5)&0x07E0)
        circularDisp.DrawLineAA(12*centre,12*centre,radius,239,radius&0x1F)
        circularDisp.DrawLineAA(12*centre,12*centre,239,radius,0xffff>>centre)
      #circularDisp.ScreenUpdate()

  if(test==3):

    # do the wide line testing
    for centre in range (10):
      for radius in range(240):
        circularDisp.DrawLineWideAA(12*centre,12*centre,radius,0,radius<<11,10)
        circularDisp.DrawLineWideAA(12*centre,12*centre,0,radius,(radius<<5)&0x07E0,10)
        circularDisp.DrawLineWideAA(12*centre,12*centre,radius,239,radius&0x1F,10)
        circularDisp.DrawLineWideAA(12*centre,12*centre,239,radius,0xffff>>centre,10)
      #circularDisp.ScreenUpdate()
      
  if (test ==0):
    circularDisp.DrawLineAA(120,120,239,120,0xFFFF)
    circularDisp.DrawLineAA(120,120,0,120,0xFFFF)
    circularDisp.DrawLineAA(120,120,120,0,0xFFFF)
    circularDisp.DrawLineAA(120,120,120,239,0xFFFF)

    circularDisp.DrawLineAA(110,120,239,120,0xFFFF)
    circularDisp.DrawLineAA(110,120,-1,120,0xf800)
    circularDisp.DrawLineAA(110,120,120,-1,0x07E0)
    circularDisp.DrawLineAA(110,120,120,239,0x07E0)


  # some timing to see how fast it can work
  now = datetime.datetime.now()
  print("lines time End: ")
  print(str(now-starttime))

  circularDisp.ScreenUpdate()


  circularDisp.exitBCMHardware()
else:
  print ("failed to imitialise the hardware - probably not running as root")

print ("done")

