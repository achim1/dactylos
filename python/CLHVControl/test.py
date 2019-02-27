#! /usr/bin/env python3

"""
Control a CAEN HV power supply.
"""


from clhvcontrol import CAENN1471HV

if __name__ == "__main__":

    import argparse
    import numpy as np

    parser = argparse.ArgumentParser(description="Control the CAEN1471HV power supply")
    parser.add_argument('--port', dest='port', type=str,
                        default='/dev/caen1471',
                        help='the serial port where the hv supply is listening. Typically, this might be /dev/ttyUSB0, however udev rules might change that. The default is set to [/dev/caen1471] with the specific udev rule in place. If there is no udev rule in place, the opening of the port might fail and then root permissions are required.')

    parser.add_argument('--debug', dest='debug',
                        default=False, action='store_true',
                        help='Set the loglevel to 10, that is debug')

    args = parser.parse_args()
    port = args.port
    debug = args.debug
    loglevel = 30
    if args.debug:
        loglevel = 10
    #port = '/dev/ttyUSB0'
    #port = '/dev/caen1471'
    hv = CAENN1471HV(port=port, loglevel=loglevel, time_delay=0.5)
    print (hv)
    hv.all_channels.activate()
    try:
        hv.clear_alarm_signal()
    except Exception as e:
        print (e)
    hv.channel0.voltage_as_set = 2.5
    print ("Set voltage for ch 0 {}, observed: {}".format(hv.channel0.voltage_as_set, hv.channel0.voltage_as_is))
    #hv._send("$BD:00,CMD:MON,CH:4,PAR:VSET\r\n")
    try:
        print (hv._listen())
    except Exception as e:
        print (e)
    hv.channel1.voltage_as_set = 3.3
    print ("Set voltage for ch 1 {}, observed: {}".format(hv.channel1.voltage_as_set, hv.channel1.voltage_as_is))
    volts = np.linspace(0,30,31)
    m_volts, m_curr = hv.channel1.take_iv_curve(volts, time_interval=0.1)
    print (m_volts, m_curr)
    hv.all_channels.deactivate()
