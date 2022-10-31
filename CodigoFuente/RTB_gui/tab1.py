from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5 import uic


class Tab1Info:
    def __init__(self):
        pass


    def Init(self, mainUI):
        self.btConnect = mainUI.findChild(QPushButton, 'tb1Connect')
        self.btConnect.clicked.connect(self.onPushConnect)

        self.btRefresh = mainUI.findChild(QPushButton, 'btRefresh')
        self.btRefresh.clicked.connect(self.onPushRefresh)

        self.portList = mainUI.findChild(QComboBox,'comboPorts')
        self.portList.addItem('Test 1')

        self.sr = mainUI.sr
        self.mainUI = mainUI


    def onPushConnect(self):
        print(self.portList.currentData())
        self.sr.Connect(self.portList.currentData())
        if not self.sr.isRunning():
            self.mainUI.threadpool.start(self.sr)

    def onPushRefresh(self):
        self.portList.clear()
        puertos = self.sr.getListOfPorts()
        for puerto,desc in puertos.items():
            self.portList.addItem(str(puerto) + ": " + str(desc),str(puerto))
