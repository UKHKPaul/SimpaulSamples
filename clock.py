# simple application to show the functionality of the C driver on the circular display
#
#
#  P.Worman Jan 2021
#
#
#  see https://simpaul.com/round_display for details
#

import datetime
import math

# load in the ability to use c variable types 
from ctypes import *

# load the Shared Library for the direct I/O 
circularDisp = CDLL("./bcm_direct_c2py.so")


# initialise the hardware driver
if (circularDisp.initBCMHardware()):
  # then send the commands to configure the display
  circularDisp.initCircularDisp()


  #load a sample 240x240 BMP for the background
  file = open("./watch.bmp","rb")
  data = file.read()
  file.close()


  # load the BMP image data directly
  circularDisp.RGB240x240Direct(data,1)

  # then tell the driver to save the current image as the reference
  circularDisp.SetRefernceImage()

  # use the try to allow clean exit on CTRL C
  try:
    while(1):
        
      now = datetime.datetime.now()

      # useful commands if you want to see the raw time data
      # print(str(now))
      # print ("hour   {}".format(now.hour))
      # print ("Minute {}".format(now.minute))
      # print ("Sec    {}".format(now.second))
      # print ("ms     {}".format(now.microsecond))

      # get the seconds, including the fractions of the second to make for a smooth second hand
      Secf = float(now.second)+((float)(now.microsecond)/1000000.0)

      # calculate the relevant X and Y points for the end of the second hand
      Secx =  110*math.sin(Secf*math.pi/30.0)  +120.0
      Secy = -110*math.cos(Secf*math.pi/30.0)  +120.0

      # not work out the minutes, again use the seconds to give a smooth moving minute hand
      Minf = float(now.minute)+((float)(now.second)/60.0)

      Minx =  90*math.sin(Minf*math.pi/30.0)  +120.0
      Miny = -90*math.cos(Minf*math.pi/30.0)  +120.0

      # finaly do the hours based on Hours and minutes for smooth movement
      Hrf = float(now.hour)+((float)(now.minute)/60.0)

      Hrx =  70*math.sin(Hrf*math.pi/6.0)  +120.0
      Hry = -70*math.cos(Hrf*math.pi/6.0)  +120.0

      # note that the DrawLine commands only write to the render buffer and are not immediatly visible
      # this gives a better display update for this type of application

      # as I used a white clock face, then the main hands will be black

      handcolour = circularDisp.RGBto16bit(0,0,0)   #black
      # these lines give a thick centre with a single pixel point
      circularDisp.DrawLine(120,120,(int)(Hrx),(int)(Hry),handcolour)
      circularDisp.DrawLine(121,120,(int)(Hrx),(int)(Hry),handcolour)
      circularDisp.DrawLine(119,120,(int)(Hrx),(int)(Hry),handcolour)
      circularDisp.DrawLine(120,121,(int)(Hrx),(int)(Hry),handcolour)
      circularDisp.DrawLine(120,119,(int)(Hrx),(int)(Hry),handcolour)

      circularDisp.DrawLine(120,120,(int)(Minx),(int)(Miny),handcolour)
      circularDisp.DrawLine(121,120,(int)(Minx),(int)(Miny),handcolour)
      circularDisp.DrawLine(119,120,(int)(Minx),(int)(Miny),handcolour)
      circularDisp.DrawLine(120,121,(int)(Minx),(int)(Miny),handcolour)
      circularDisp.DrawLine(120,119,(int)(Minx),(int)(Miny),handcolour)

      # for the second hand, use a Blue hand colour but make both centre and end wide
      handcolour = circularDisp.RGBto16bit(0,0,255)   #Blue
      circularDisp.DrawLine(120,120,(int)(Secx),(int)(Secy),handcolour)
      circularDisp.DrawLine(121,120,(int)(Secx)+1,(int)(Secy),handcolour)
      circularDisp.DrawLine(119,120,(int)(Secx)-1,(int)(Secy),handcolour)
      circularDisp.DrawLine(120,121,(int)(Secx),(int)(Secy)+1,handcolour)
      circularDisp.DrawLine(120,119,(int)(Secx),(int)(Secy)-1,handcolour)

      # tell the driver to write the image data to the screen
      circularDisp.ScreenUpdate()

      # now reset the background for the next cycle
      circularDisp.RestoreReferenceImage()
     
  except:
    print("Exiting")

  # make sure any clean up is done  
  circularDisp.exitBCMHardware()
else:
  print ("failed to imitialise the hardware - probably not running as root")

