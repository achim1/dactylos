#! /usr/bin/env python3
"""
Control SUN EC13 temperature chamber
"""

from .ni_gpib_usb import NI_GPIB_USB 
from .prologix_gpib_usb import PrologixUsbGPIBController

import numpy as np
import time 

try:
    import zmq 
except ImportError:
    print("Can not import zero MQ")

class SUNEC13Commands:
    STATUS = "STATUS?"
    ON     = "ON"
    OFF    = "OFF"
    TEMP3 = "C3?"
    TEMP4 = "C4?"


class  SunChamber(object):

    axis_label = "Temperature "
    temperature_unit = "C"
    status_dict = [{ "Y" : "Power ON", "N" : "Power OFF"},\
                   { "Y" : "Last command error", "N" : "Last command ok"},\
                   { "Y" : "Time out LED ON", "N" : "Time out LED OFF"},\
                   { "Y" : "Waiting for timeout", "N" : "Not waiting for timeout"},\
                   { "Y" : "Heat output is enabled", "N" : "Heat output is disabled"},\
                   { "Y" : "Cool output is enabled", "N" : "Cool output is disabled"},\
                   { "Y" : "Valid set temperature", "N" : "Invalid set temperatur"},\
                   { "Y" : "Deviation limit exceeded", "N" : "Deviation limit ok"},\
                   { "Y" : "Currently ramping", "N" : "Not currently ramping"},\
                   { "Y" : "Chamber temp < lower limit", "N" : "Chamber temp > lower limit"},\
                   { "Y" : "Chamber temp > upper limit", "N" : "Chamber temp < upper limit"},\
                   { "Y" : "Waiting at a BKPNT", "N" : "Not waiting at a BKPNT"},\
                   { "Y" : "In LP run mode", "N" : "Not in LP run mode"},\
                   { "Y" : "In LP remote store mode", "N" : "Not in LP remote store mode"},\
                   { "Y" : "In local edit LP mode", "N" : "Not in local edit LP mode"},\
                   { "Y" : "Waiting to run LP at TOD", "N" : "Not waiting to run LP as TOD"},\
                   { "Y" : "GPIB bus timeout", "N" : "No GPIB bus timeout"},\
                   { "Y" : "In local keyboard lockout mode", "N" : "Not in local keyboard lockout mode"},\
                   { "0" : "System self test was ok", "1" : "Battery RAM error found (check default settings)",\
                     "2" : "EE RAM error found (check default settings)", "3" : "ROM error found (call factory)"}]
    

    #def __init__(self,gpib_adress=6, port=9999, publish=False):
    def __init__(self, controller, port=9999, publish=False):
        
        assert (isinstance(controller, NI_GPIB_USB) or isinstance(controller, PrologixUsbGPIBController)), "The used controller has to be either the NI usb one or the prologix usb"
        
        self.chamber = controller
        self.is_running = False
        self.publish = publish
        self.port = port
        self._socket = None

        # sometimes the chamber needs a bit till it is responding
        # get the status a few times
        self.get_temperature()
        self.get_status()
        self.get_status()

        self.last_status = ""
        status = self.get_status()
        self.print_status(status)
        if publish:
            self._setup_port()

    def _setup_port(self):
        """
        Setup the port for publishing

        Returns:
            None
        """
        context = zmq.Context()
        self._socket = context.socket(zmq.PUB)
        self._socket.connect("tcp://0.0.0.0:%s" % int(self.port))
        return


    @property
    def ON(self):
        print ("Turning on chamber...")
        self.chamber.write(SUNEC13Commands.ON)

    @property
    def OFF(self):
        print ("Turning chamber off...")
        if self.is_running:
            print ("WARNING, will not turn off chamber whie it is operationg!!...")
            return
        self.chamber.write(SUNEC13Commands.OFF)

    def get_status(self):
        self.last_status = self.chamber.query(SUNEC13Commands.STATUS)
        return self.last_status

    @staticmethod
    def print_status(status): 
        print ("SUN EC13 chamber reporting status....")
        status = status.rstrip()
        for i,k in enumerate(status):
                print (SunChamber.status_dict[i][k])
        print ("----------------------------------")
    
    def show_status(self):
        status = self.get_status()
        self.print_status(status)

    def get_temperature(self, channel=0):
        """
        Channel 0,1
        """
        if channel == 0:
            temp = self.chamber.query(SUNEC13Commands.TEMP3)
        elif channel == 1:
            temp = self.chamber.query(SUNEC13Commands.TEMP4)
        else:
            raise ValueError("Channel has to be either 0 or 1!")
        print ("Got channel temp of {}".format(temp))
        
        try:
            temp = float(temp)
        except ValueError:
            print ("Problems digesting {}".format(temp))
            temp = np.nan

        if self.publish and (self._socket is None):
            self._setup_port()
        if self.publish:
            self._socket.send_string("{}::CH{} {}; {}".format("SUNEC13", channel, temp, self.temperature_unit))
  
        return temp 

    def measure_continously(self, npoints, interval):
        for n in range(npoints):
            temp1 = self.get_temperature(channel=0)
            temp2 = self.get_temperature(channel=1)
            if self.publish:
                self._socket.send_string("{}::CH{} {}; {}".format("SUNEC13", 0, temp1, self.temperature_unit))
                self._socket.send_string("{}::CH{} {}; {}".format("SUNEC13", 1, temp2, self.temperature_unit))
            time.sleep(interval)
            yield n*interval, (temp1,temp2)
    
