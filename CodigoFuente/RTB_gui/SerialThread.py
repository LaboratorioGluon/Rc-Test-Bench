import time
from PyQt5.QtCore import *
import pyqtgraph

import serial
from serial.serialutil import Timeout
import struct
import serial.tools.list_ports

import re


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
                    lines=localbuf.split("\n")
                    for l in lines[:-1]:
                        print(l)
                        #Tenemos las lineas buenas
                        search=re.search("\[(\w+)\]:(.*)", l)
                        todo=search.group(0)
                        tag = search.group(1)
                        content = search.group(2)

                        if "TEST_DATA" in tag:
                            data = content.split(":")
                            data2 = [int(d) for d in data]
                            print("To plot:" + str(data2))
                            self.signals.newValues.emit(data2)

                    localbuf = lines[-1]


            except:
                print("Error:")
                localbuf = ""

            
            time.sleep(0.05)

        print("End Thread!")
        self.running = False

    def sendCmd(self, cmd, data=0):
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
        return outports