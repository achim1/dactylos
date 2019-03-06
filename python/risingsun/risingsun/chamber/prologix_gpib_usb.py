import serial
from time import sleep

class PrologixUsbGPIBController(object):


    def __init__(self, port='/dev/ttyUSB0', gpib_adress=6, stopbits=2, set_auto_mode=True):
        self.conn = serial.Serial(port=port, stopbits=stopbits)    
        self.conn.write(f"++addr {gpib_adress}\n".encode())
        self.conn.write("++eos 0\n".encode())
        if set_auto_mode:
            self.conn.write("++auto 1\n".encode())


    def query(self, command):
        self.conn.write(f"{command}\n\n".encode())
        sleep(0.3) 
        resp = self.conn.read_all()
        return resp.decode().rstrip("\n")

    def write(self, command):
        self.conn.write(f"{command}\n\n".encode())
        return None

    

