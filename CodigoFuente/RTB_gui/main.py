from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5 import uic

import sys

from SerialThread import SerialReader

from tab1 import Tab1Info
from configTab import configTab

import pyqtgraph as pg
pg.setConfigOption('background','w')
pg.setConfigOption('foreground','k')
pg.setConfigOption('antialias',False)

#QApplication.setGraphicsSystem('opengl')

# @TODO: Boton de reconectar

class Ui(QMainWindow):
    def __init__(self):
        super(Ui, self).__init__() # Call the inherited classes __init__ method
        uic.loadUi('MainGUI.ui', self) # Load the .ui file
        
        self.threadpool = QThreadPool()
        self.sr = SerialReader()
        self.sr.signals.newData.connect(self.updateSerial)
        self.sr.signals.newValues.connect(self.updatePlot)


        self.setpwm = self.findChild(QPushButton, 'setPwmButton')
        self.setpwm.clicked.connect(self.setPwm)

        self.startbutton = self.findChild(QPushButton, 'StartProcess')
        self.startbutton.clicked.connect(self.startRun)

        
        self.emergencyStop.clicked.connect(lambda data:self.sr.sendCmd(1,1))

        self.serialMonitor = self.findChild(QPlainTextEdit, 'serialMonitor')

        self.pwmvalue = self.findChild(QPlainTextEdit,'PwmValueText')
        self.pwmvalue.installEventFilter(self)
        
        self.tab1 = Tab1Info()
        self.tab1.Init(self)

        self.configTab = configTab()
        self.configTab.Init(self)


        # Init plot

        self.plotNumberOfCols = 2
        self.p1 = self.graphicsView.plotItem
        self.p2 = pg.ViewBox()
        self.p1.showAxis('right')
        self.p1.scene().addItem(self.p2)
        self.p1.getAxis('right').linkToView(self.p2)
        #self.p2.setXLink(self.p1)

        self.graphicsView.plot(title="Fuerza Generada")
        self.dataX = []
        self.dataY = [[], []]


        #self.pyqtwin = pg.GraphicsWindow()

        self.curvas = []
        self.curvas.append(self.p1.plot(pen=pg.mkPen(color='k',width=3)))
        self.curvas.append(pg.PlotCurveItem(pen=pg.mkPen(color='r',width=3)))
        #self.curvas.append(pg.PlotCurveItem(pen=pg.mkPen(color='g',width=3)))
        self.p2.addItem(self.curvas[-1])
        #self.curvas.append(self.graphicsView.plot([0]*self.plotNumberOfCols, [0]*self.plotNumberOfCols, pen=pg.mkPen(color='k',width=3)))  ## setting pen=None disables line drawing
        #self.graphicsView.disableAutoRange()
        

        self.show() # Show the GUI

    def updatePlot(self, data):
        self.dataX.append(data[0])
        #for i in range(len(data)-1):
        self.voltageLabel.setText(str(data[3]/1000) + " V")
        for i in range(2):
            self.dataY[i].append(data[i+1])
            #self.curvas[i].setData(self.dataX, self.dataY[i])
            #if i == 0:
            self.curvas[i].setData(self.dataX, self.dataY[i])
            
        self.p2.setGeometry(self.p1.vb.sceneBoundingRect())
        return

        """if len(self.dataY) > 200:
            self.dataY = self.dataY[1:]
            self.dataX = self.dataX[1:]"""

        print(data)
        self.curvas[-1].setData(self.dataX, self.dataY)

    def setPwm(self, data):
        self.sr.sendCmd(1, int(float(self.pwmvalue.toPlainText())*100.0))

    def updateSerial(self, data):
        self.serialMonitor.insertPlainText(data)

    def startRun(self, data):
        #self.dataX = []
        #self.dataY = []
        #self.curvas[-1].setData(self.dataX, self.dataY)
        if self.configTab.isConfigReady == False:
            self.serialMonitor.insertPlainText("Configuration NOT ready!\n")
        self.dataX = []
        self.dataY = [[], []]
        for i in range(2):
            self.curvas[i].setData(self.dataX, self.dataY[i])
            
        self.sr.sendCmd(2,35)

    def startReadCurrent(self):
        self.sr.sendCmd(3)

    def closeEvent(self, event):
        self.sr.close()
        self.threadpool.waitForDone()
        event.accept()

    def eventFilter(self, obj, event):
        if event.type() == QEvent.KeyPress and obj is self.pwmvalue:
            if event.key() in [Qt.Key_Return, Qt.Key_Enter] and self.pwmvalue.hasFocus():
                print('Enter pressed')
                self.setPwm(None)
                self.pwmvalue.clear()
        return super().eventFilter(obj, event)

app = QApplication(sys.argv) # Create an instance of QtWidgets.QApplication
window = Ui() # Create an instance of our class
app.exec_() # Start the application