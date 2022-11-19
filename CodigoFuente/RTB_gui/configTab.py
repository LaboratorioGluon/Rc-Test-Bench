from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5 import uic

import struct

class configTab:


    def __init__(self):
        self.maxPowerPercent = 10
        self.minVoltage = 7.80

        self.lipoToVoltage = [3.75, 7.49, 11.24, 14.99]
        self.isConfigReady = False
        self.lastValidConfig = [0, 0]
        self.dataOk = [False, False]
        

    def Init(self, mainUI):
        mainUI.enviarConfig.clicked.connect(self.sendConfig)
        #self.btConnect.clicked.connect(self.onPushConnect)

        mainUI.lipoSSComboBox.currentIndexChanged.connect(self.comboBoxChanged)
        #mainUI.powerOffVoltageLineEdit.textChanged.connect(self.minVoltageChanged)
        mainUI.powerOffVoltageLineEdit.textEdited.connect(self.minVoltageChanged)

        mainUI.maxPowerLineEdit.textChanged.connect(self.maxPowerChanged)
        self.mainUI = mainUI   
        mainUI.powerOffVoltageLineEdit.setText(str(self.lipoToVoltage[0]))
        self.minVoltageChanged(str(self.lipoToVoltage[0]))
        


    def maxPowerChanged(self, newValue):
        try:
            fValue = float(newValue)
        except:
            self.mainUI.maxPowerLineEdit.setStyleSheet("color:#ff0000")
            self.dataOk[0] = False
        else:
            if fValue > 100 or fValue < 5:
                self.mainUI.maxPowerLineEdit.setStyleSheet("color:#ff0000")
                self.dataOk[0] = False
            else:
                self.mainUI.maxPowerLineEdit.setStyleSheet("color:#000000")
                self.dataOk[0] = True

    def minVoltageChanged(self, newVoltage):
        try:
            fValue = float(newVoltage)
        except:
            self.mainUI.powerOffVoltageLineEdit.setStyleSheet("color:#ff0000")
            self.dataOk[1] = False
        else:
            if fValue < 3.75:
                self.mainUI.powerOffVoltageLineEdit.setStyleSheet("color:#ff0000")
                self.dataOk[1] = False
            else:
                self.mainUI.powerOffVoltageLineEdit.setStyleSheet("color:#000000")
                self.dataOk[1] = True

            if fValue in self.lipoToVoltage:
                self.mainUI.lipoSSComboBox.setCurrentIndex(
                    self.lipoToVoltage.index(fValue)
                )
            else:
                self.mainUI.lipoSSComboBox.setCurrentIndex(4)


    def comboBoxChanged(self, newIndex):
        if newIndex < len(self.lipoToVoltage):
            self.mainUI.powerOffVoltageLineEdit.setText(str(self.lipoToVoltage[newIndex]))
            self.minVoltageChanged(self.lipoToVoltage[newIndex])
        #else:
        #    self.mainUI.powerOffVoltageLineEdit.setText("")

    def sendConfig(self):
        self.maxPowerPercent = self.mainUI.maxPowerLineEdit.text()
        self.minVoltage = self.mainUI.powerOffVoltageLineEdit.text()

        print("== SEND CONFIG DEBUG == ")
        print(f"Is Data ok: {self.dataOk}")
        print(f"Max power(%) {self.maxPowerPercent}")
        print(f"Voltage Power Off(V) {self.minVoltage}")

        if self.dataOk[0] == True and self.dataOk[1] == True:
            fMaxPowerPerc = float(self.maxPowerPercent)
            fMinVoltage = float(self.minVoltage)
            if self.__sendConfigUart([fMaxPowerPerc, fMinVoltage]) == True:
                self.isConfigReady = True
                self.lastValidConfig = [fMaxPowerPerc, fMinVoltage]
            else:
                self.isConfigReady = False
                self.lastValidConfig = [0, 0]
            

    """ return: True -> If all ok """
    def __sendConfigUart(self, config):
        print("Enviando datos al UART")
        maxpower = int(config[0])
        minvolt = config[1]
        minvoltMv = int(minvolt*100)
        self.mainUI.configMaxPowerLabel.setText(f"Max Power: {str(maxpower)}%")
        self.mainUI.configMinVoltageLabel.setText(f"Min Voltage: {str(minvoltMv/100)}V")
        

        val = struct.pack("=BBII", 0xBB, 10, maxpower, minvoltMv)
        
        return True

