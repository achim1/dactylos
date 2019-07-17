"""
Control a CAEN HV power supply.
"""

import serial
import time
import tqdm
import re
import numpy as np

from .loggers import get_logger

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

def setget(parameter, getter_only = False, doc=None):
    """
    Shortcut to construct property objects to wrap getters and setters
    for a number of settings

    Args:
        parameter: The parameter name to get/set. Get will be a query

    Returns:
        property object
    """
    if getter_only:
        return property(lambda self: self._get_parameter(parameter), doc=doc)
    else:
        return property(lambda self: self._get_parameter(parameter),\
                        lambda self, value: self._set_parameter(parameter,value), doc=doc)


class CAENN1471ChannelCommands(object):
    RAMP = "RAMP"
    KILL = "KILL"
    LOW  = "LOW"
    HIGH = "HIGH"

class CAENN1471BoardCommands(object):
    CLOSED = "CLOSED"
    OPEN = "OPEN"

class Channel(object):
    """
    Namespace to access the individual channels of
    the CAENN1471 HV power supply
    """
    def __init__(self, channel, board, loglevel=30):
        """
        Keyword Args:
            loglevel (int) : 10 dbg, 20 info, 30 warn
        """
        self.channel = channel
        self.board = board
        self.logger = get_logger(loglevel)
    # per channel commands, set parameters
    def _set_parameter(self, parameter, value):
        command = "$BD:{:02d},CMD:SET,CH:{},PAR:{},VAL:{}".format(self.board.board,self.channel,parameter, str(value))
        self.board._send(command)
        response = self.board._listen()
        self.logger.debug("Set parameter for channel {} succesful {}".format(self.channel, response))    

    # get channel values
    def _get_parameter(self, parameter):
        command = "$BD:{:02d},CMD:MON,CH:{},PAR:{}".format(self.board.board,self.channel,parameter)
        self.board._send(command)
        response = self.board._listen()
        self.logger.debug("Retrieved parameter {} for channel {}".format(response, self.channel, response))    
        return response    
   
    def activate(self):
        """
        Set channel to on 
        """
        self.logger.info("Activating channel {}".format(self.channel))
        command = "$BD:{:02d},CMD:SET,CH:{},PAR:ON".format(self.board.board, self.channel)
        self.board._send(command)
        self.board._listen()
        # wait till it is ramped-up
        rate = self.ramp_up
        delta_v = np.array(self.voltage_as_set) - np.array(self.voltage_as_is)
        wait = int(delta_v/rate)
        for k in tqdm.tqdm(range(wait)):
            time.sleep(1)
        

    def take_iv_curve(self, voltages, time_interval=1):
        """
        Take iv curve. Get voltages and currents for the given 
        number of voltages.

        Args:
            voltages (iterable): array of voltages

        Keyword Args:
            time_interval (float): time [in sec] between measurments

        """
        command = "$BD:{:02d},CMD:SET,CH:{},PAR:ON".format(self.board.board, self.channel)


        measured_voltages = []
        measured_currents = []
        for volt in tqdm.tqdm(voltages):
            try:
                self.voltage_as_set = volt
                self.board._send(command)
                measured_voltages.append(self.voltage_as_is)
                measured_currents.append(self.current_as_is)
                time.sleep(time_interval)
            except Exception as e:
                print (e)
                time.sleep(2*time_interval)

        measured_voltages = np.array(measured_voltages)
        measured_currents = np.array(measured_currents)
        return measured_voltages, measured_currents

    def deactivate(self):
        """
        Set channel to off 
        """
        command = "$BD:{:02d},CMD:SET,CH:{},PAR:OFF".format(self.board.board, self.channel)
        self.board._send(command)
        self.board._listen()
        rate = self.ramp_down
        delta_v =  np.array(self.voltage_as_is)
        wait = int(delta_v/rate)
        for k in tqdm.tqdm(range(wait)):
            time.sleep(1)

    @property
    def status(self):
        command = "$BD:{:02d},CMD:MON,CH:{},PAR:STAT".format(self.board.board, self.channel)
        self.board._send(command)
        response = self.board._listen()
        return response
        


    # setters/getters
    voltage_as_set = setget("VSET", doc = "The desired voltage")
    current_as_set = setget("ISET", doc = "The desired current")
    max_set_voltage = setget("MAXV")
    ramp_up = setget("RUP")
    ramp_down = setget("RDW")
    trip_time = setget("TRIP")
    # can be either KILL or RAMP
    power_down_ramp_or_kill = setget("PDWN")
    # can be either LOW or HIGH
    imon_range_low_or_high = setget("IMRANGE")

    # getters only
    voltage_as_is = setget("VMON", getter_only = True, doc = "Get current [true] voltage")
    max_set_voltage = setget("VMAX", getter_only = True, doc = "Get max vset value")
    min_set_voltage = setget("VMIN", getter_only = True, doc = "Get min vset value")
    voltage_n_decimal_digits = setget("VDEC", getter_only = True)

    current_as_is = setget("IMON", getter_only = True, doc = "Get curretn [true] current")
    max_set_current = setget("IMAX", getter_only = True)
    min_set_current = setget("IMIN", getter_only = True)
    current_n_decimal_digits = setget("ISDEC", getter_only=True)

    polarity = setget("POL", getter_only=True, doc="channel polarity, readout only") 


class CAENN1471HV(object):
    #command_string = 
    ERROR_STRING = re.compile('#BD:[0-9]{2},(?P<errcode>[A-Z]{2,3}):ERR')
    SUCCESS_STRING = re.compile('#BD:[0-9]{2},CMD:OK(,VAL:)*(?P<values>[0-9A-Za-z;.]*)')

    def __init__(self, port='/dev/caen1471',
                 board=0,
                 time_delay=1,
                 loglevel=30):
        """
        Set up a connection to a CAEN1471HV module via usb/serial connection.

        Keyword Args:
            port (str):  unix device file, e.g. /dev/ttyUSB0 or /dev/caen1471
            board (int): board number
            loglevel (int) : 10 dbg, 20 info, 30 warn
        """
        self.logger = get_logger(loglevel)
        self.logger.info('Opening connection to {}'.format(port))
        self.connection = CAENN1471HV.open_connection(port)
        self.board = board
        self.last_command = None
        # give the hv module some time to respond
        # wait self.time_delay after each write command 
        # on the interface
        self.time_delay = time_delay
        self.channels = dict()
        # the channel number 0-3 are the 4 channels
        # channel 4 is all channels together
        for i in range(int(self.nchannels[0]) + 1):
            thischan = Channel(i, board=self, loglevel=loglevel)
            if i < 4:
                setattr(self, "channel{}".format(i),thischan)
            if i == 4:
                setattr(self, "all_channels", thischan)
            self.channels[i] = thischan

    def __del__(self):
        self.connection.close()

    def __repr__(self):
        return "<CAENN1471HV: {} board no: {} serial number {}>".format(self.boardname[0], self.board, self.serial_number)

    @staticmethod
    def open_connection(port):
        conn = serial.Serial(port=port, xonxoff=True)   
        for __ in tqdm.tqdm(range(10)):
            time.sleep(0.05)
        return conn

    # low level read/write
    def _send(self,command):
        if not command.endswith("\r\n"):
            command += "\r\n"
        command = command.encode()
        self.last_command = command
        self.logger.debug("Sending command {}".format(command))
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
            elif errcode == "VAL":
                raise ValueException("Wrong set value (<Min or >Max) {}".format(self.last_command))
            elif errcode == "LOC":
                raise LocalModeError("Command SET with module in LOCK mode {}".format(self.last_command))
            else:
                raise UnknownException("Some unknown problem occured with command {} - got response {}".format(self.last_command, response))

    def _listen(self):
        response = self.connection.read_all().decode()
        self.check_response(response)
        success = CAENN1471HV.SUCCESS_STRING.match(response)
        if success is not None:
            self.logger.debug("Got repsonse {}".format(response))
            if 'values' in success.groupdict():
                self.logger.debug("Got values {}".format(success.groupdict()['values']))
                values = success.groupdict()['values']
                if ';' in values:
                    values = [k for k in values.split(';')]
                elif values:
                    values = [values]
                else:
                    return None
                try:
                    values = [float(k) for k in values]
                except ValueError: #it is a string then
                    pass
                return values
            return None
        else:
            raise UnknownException("Something went wrong, got garbagae response {}".format(response)) 

    def _get_parameter(self,parameter):
        command = "$BD:{:02d},CMD:MON,PAR:{}".format(self.board, parameter)
        self._send(command)
        response = self._listen()
        return response

    def _set_parameter(self,parameter,value):
        command = "$BD:{:02d},CMD:MON,PAR:{},VAL:{}".format(self.board, parameter, value)
        self._send(command)
        response = self._listen()
        return response

    def clear_alarm_signal(self):
        command = "$BD:{:02d},CMD:SET,PAR:BDCLR".format(self.board)
        self._send(command)
        response = self._listen()

    interlock_mode = setget("BDILKM", doc="Either OPEN or CLOSED")
    boardname = setget("BDNAME", getter_only=True, doc="Name of the board")
    nchannels = setget("BDNCH", getter_only=True, doc="Number of provided channels")
    firmware_release = setget("BDFREL", getter_only=True, doc="Firmware release version number") 
    serial_number = setget("BDSNUM", getter_only=True, doc="Serial number of the board")
    interlock_status = setget("BDILK", getter_only=True, doc="Interlock status, either YES or NO") 
    control_mode = setget("BDCTR", getter_only=True, doc="Control mode, either REMOTE or LOCAL")
    local_bus_termination_status  = setget("BDTERM", getter_only=True, doc="Local bus termination status, either ON or OFF")
    board_alarm_bitmask = setget("BDALARM", getter_only=True, doc="Board alarm bitmask, needs to be decoded") 
      
