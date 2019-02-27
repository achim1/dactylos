#! /usr/bin/env python3

"""
Control a CAEN HV power supply.
"""

import serial
import time
import tqdm
import re

class UnknownException(Exception):
    pass

class CommandException(Exception):
    pass

class ChannelException(Exception):
    pass

class ParameterException(Exception):
    pass

class ValueException(Exception):
    pass

class LocalModeError(Exception):
    pass

class CAENN1471HV(object):
    #command_string = 
    ERROR_STRING = re.compile('#BD:[0-9]{2},(?P<errcode>[A-Z]{2,3}):ERR')
    SUCCESS_STRING = re.compile('#BD:[0-9]{2},CMD:OK(,VAL:)*(?P<values>[0-9;.]*)')

    def __init__(self, port='/dev/caen1471', board=0):
        """
        Set up a connection to a CAEN1471HV module via usb/serial connection.

        Keyword Args:
            port (str):  unix device file, e.g. /dev/ttyUSB0 or /dev/caen1471
            board (int): board number
        """
        self.connection = CAENN1471HV.open_connection(port)
        self.board = board
        self.last_command = None
        # give the hv module some time to respond
        # wait self.time_delay after each write command 
        # on the interface
        self.time_delay = 1.5

    def __del__(self):
        self.connection.close()

    @staticmethod
    def open_connection(port):
        conn = serial.Serial(port=port, xonxoff=True)   
        print ('Opening connection to {}'.format(port))
        for __ in tqdm.tqdm(range(10)):
            time.sleep(0.05)
        return conn

    # low level read/write
    def _send(self,command):
        if not command.endswith("\r\n"):
            command += "\r\n"
        command = command.encode()
        self.last_command = command
        print (command)
        self.connection.write(command)
        time.sleep(self.time_delay)

    def check_response(self,response):
        err = CAENN1471HV.ERROR_STRING.match(response)
        if err is not None:
            errcode = err.groupdict()["errcode"]
            if errcode == "CMD":
                raise CommandException("Wrong command format or command not recongized {}".format(self.last_command))
            elif errcode == "CH":
                raise ChannelException("Channel field not present or wrong channel value {}".format(self.last_command))
            elif errcode == "PAR":
                raise ParameterException("Field parameter not present or parameter not recognized {}".format(self.last_command))
            elif errcode == "Val":
                raise ValueExceptionException("Wrong set value (<Min or >Max) {}".format(self.last_command))
            elif errcode == "LOC":
                raise LocalModeError("Command SET with module in LOCK mode {}".format(self.last_command))
            else:
                raise UnknownException("Some unknown problem occured with command {}".format(self.last_command))

    def _listen(self):
        response = self.connection.read_all().decode()
        self.check_response(response)
        success = CAENN1471HV.SUCCESS_STRING.match(response)
        if success is not None:
            print (response)
            if 'values' in success.groupdict():
                print (success.groupdict()['values'])
                values = success.groupdict()['values']
                if ';' in values:
                    values = [float(k) for k in values.split(';')]
                elif values:
                    values = [float(values)]
                else:
                    return None
                return values
            return None
        else:
            raise UnknownException("Something went wrong, got garbagae response {}".format(response)) 

    # per channel commands, set parameters
    def _set_channel_parameter(self, channel, parameter, value):
        command = "$BD:{:02d},CMD:SET,CH:{}:PAR:{},VAL:{}".format(self.board,channel,parameter, str(value))
        self._send(command)
        response = self._listen()
        print ("Set parameter for channel {} succesful {}".format(channel, response))    

    def set_channel_on(self,channel):
        """
        Set channel to on 
        """
        command = "$BD:{:02d},CMD:SET,CH:{},PAR:ON".format(self.board, channel)
        self._send(command)
        self._listen()

    def set_channel_off(self,channel):
        """
        Set channel to off 
        """
        command = "$BD:{:02d},CMD:SET,CH:{},PAR:OFF".format(self.board, channel)
        self._send(command)
        self._listen()

    def set_channel_voltage(self, channel, voltage):
        self._set_channel_parameter(channel, "VSET", voltage)

    def set_channel_current(self, channel, current):
        self._set_channel_parameter(channel, "ISET", current)
    
    def set_channel_max_set_voltage(self, channel, maxvset):
        self._set_channel_parameter(channel, "MAXV", maxvset)

    def set_channel_ramp_up(self, channel, ramp_up):
        self._set_channel_parameter(channel, "RUP", ramp_up)

    def set_channel_ramp_down(self, channel, ramp_down):
        self._set_channel_parameter(channel, "RDW", current)

    def set_channel_trip_time(self, channel, trip_time):
        self._set_channel_parameter(channel, "IRIP", trip_time)

    def set_channel_power_down_ramp(self, channel):
        self._set_channel_parameter(channel, "PDWN", "RAMP")

    def set_channel_power_down_kill(self, channel):
        self._set_channel_parameter(channel, "PWDN", "KILL")

    def set_channel_imon_range_low(self, channel):
        self._set_channel_parameter(channel, "IMRANGE", "LOW")

    def set_channel_imon_range_high(self, channel):
        self._set_channel_parameter(channel, "IMRANGE", "HIGH")

    # board wide settings
    def set_interlock_mode_closed(self):
        command = "$BD:{:02d},CMD:SET,PAR:BDILKM,VAL:CLOSED".format(self.board)
        self._send(command)
        response = self._listen()

    def set_interlock_mode_open(self):
        command = "$BD:{:02d},CMD:SET,PAR:BDILKM,VAL:OPEN".format(self.board)
        self._send(command)
        response = self._listen()

    def clear_alarm_signal(self):
        command = "$BD:{:02d},CMD:SET,PAR:BDCLR".format(self.board)
        self._send(command)
        response = self._listen()
       
if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser(description="Control the CAEN1471HV power supply")
    parser.add_argument('--port', dest='port', type=str,
                        default='/dev/caen1471',
                        help='the serial port where the hv supply is listening. Typically, this might be /dev/ttyUSB0, however udev rules might change that. The default is set to [/dev/caen1471] with the specific udev rule in place. If there is no udev rule in place, the opening of the port might fail and then root permissions are required.')

    args = parser.parse_args()
    port = args.port
    #port = '/dev/ttyUSB0'
    #port = '/dev/caen1471'
    hv = CAENN1471HV()
    hv.set_channel_on(0) 
    try:
        hv.clear_alarm_signal()
    except Exception as e:
        print (e)
    hv.set_channel_voltage(4, 2.5)
    #hv._send("$BD:00,CMD:MON,CH:4,PAR:VSET\r\n")
    try:
        print (hv._listen())
    except Exception as e:
        print (e)
    hv.set_channel_voltage(1, 3.3)
    hv.set_channel_power_down_ramp(4)
    hv.set_channel_off(0)
