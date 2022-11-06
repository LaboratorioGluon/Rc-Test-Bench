import time
from PyQt5.QtCore import *
import pyqtgraph

import serial
from serial.serialutil import Timeout
import struct
import serial.tools.list_ports


class SerialReaderSignals(QObject):
    newData = pyqtSignal(object)
    newValues = pyqtSignal(object)

class SerialReader(QRunnable):

    def __init__(self):
        super(SerialReader, self).__init__()
        self.signals = SerialReaderSignals()        
        self.running = False
        self.hasToRun = False

        self.timer = QTimer()
        self.timer.timeout.connect(self.sendPing)  # execute `display_time`
        self.timer.setInterval(250)  # 1000ms = 1s
        


    def isRunning(self):
        return self.running

    def Connect(self, port):
        self.ser = serial.Serial(port,115200,timeout=3)
        self.timer.start()

    def sendPing(self):
        #print("Enviando ping")
        self.sendCmd(0xAA)

    def run(self):
        self.running = True
        self.hasToRun = True
        print("Thread start!")
        
        localbuf = ""
        while self.hasToRun:
            try:
                serialdata=self.ser.read_all().decode('utf-8')
                self.signals.newData.emit(serialdata)
                localbuf = localbuf + serialdata

                if '\n' in localbuf:
                    localbuf = localbuf.replace("[TEST_DATA]:","")
                    data = localbuf.replace("\n","").split(":")
                    data2 = [int(d) for d in data]
                    print("To plot:" + str(data2))
                    self.signals.newValues.emit(data2)
                    localbuf = ""


            except:
                print("Error:")
                localbuf = ""

            
            time.sleep(0.05)

        print("End Thread!")
        self.running = False

    def sendCmd(self, cmd, data=0):
        print("Enviando Cmd:"+str(cmd))
        outdata = struct.pack("BBB",cmd,1,data)
        self.ser.write(outdata)

    def close(self):
        self.hasToRun = False
        self.ser.close()

    def getListOfPorts(self):
        ports = serial.tools.list_ports.comports()
        outports = {}
        for port, desc, hwid in sorted(ports):
                outports[port] = desc
                #print("{}: {} [{}]".format(port, desc, hwid))
        return outports