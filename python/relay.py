#This code is to turn on/off a relay from Numato System Pvt Ltd

################################################################
# available commands:                                          #
# ver: returns current firmware version                        #
# id get or id set xxxxxxxx (8 characters)                     #
# relay on/off/read x                                          #
# gpio set/clear/read x                                        #
# adc read x                                                   #
################################################################

import sys
import serial

if (len(sys.argv) < 3):
    print("Usage: relay.py relay 0 on/off or gpio 0 set/clear")
    sys.exit(0)
else:
    relayType = sys.argv[1];
    relayNum = sys.argv[2];
    relayCmd = sys.argv[3];

#Open port for communication
serPort = serial.Serial("/dev/ttyACM0", 19200, timeout=1)

#Send the command
#cmd = "relay off 1\r\n"
#cmd = "relay "+ str(relayCmd) +" "+ str(relayNum) + "\n\r"
cmd = str(relayType) +" "+ str(relayCmd) +" "+ str(relayNum) + "\n\r"
serPort.write(cmd.encode())

print("Command sent...")

#Close the port
serPort.close()
