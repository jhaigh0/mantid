# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/reflectometer/parameters_refm.ui'
#
# Created: Thu Aug 18 07:40:13 2011
#      by: PyQt4 UI code generator 4.7.2
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_DataRefl(object):
    def setupUi(self, DataRefl):
        DataRefl.setObjectName("DataRefl")
        DataRefl.setEnabled(True)
        DataRefl.resize(913, 476)
        self.centralwidget = QtGui.QWidget(DataRefl)
        self.centralwidget.setObjectName("centralwidget")
        self.groupBox = QtGui.QGroupBox(self.centralwidget)
        self.groupBox.setGeometry(QtCore.QRect(20, 10, 331, 71))
        self.groupBox.setObjectName("groupBox")
        self.horizontalLayoutWidget = QtGui.QWidget(self.groupBox)
        self.horizontalLayoutWidget.setGeometry(QtCore.QRect(10, 30, 321, 31))
        self.horizontalLayoutWidget.setObjectName("horizontalLayoutWidget")
        self.horizontalLayout = QtGui.QHBoxLayout(self.horizontalLayoutWidget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.radioButton_2 = QtGui.QRadioButton(self.horizontalLayoutWidget)
        self.radioButton_2.setChecked(False)
        self.radioButton_2.setObjectName("radioButton_2")
        self.horizontalLayout.addWidget(self.radioButton_2)
        self.radioButton_3 = QtGui.QRadioButton(self.horizontalLayoutWidget)
        self.radioButton_3.setObjectName("radioButton_3")
        self.horizontalLayout.addWidget(self.radioButton_3)
        self.radioButton = QtGui.QRadioButton(self.horizontalLayoutWidget)
        self.radioButton.setChecked(True)
        self.radioButton.setObjectName("radioButton")
        self.horizontalLayout.addWidget(self.radioButton)
        self.radioButton_4 = QtGui.QRadioButton(self.horizontalLayoutWidget)
        self.radioButton_4.setObjectName("radioButton_4")
        self.horizontalLayout.addWidget(self.radioButton_4)
        self.groupBox_2 = QtGui.QGroupBox(self.centralwidget)
        self.groupBox_2.setGeometry(QtCore.QRect(370, 10, 521, 71))
        self.groupBox_2.setObjectName("groupBox_2")
        self.horizontalLayoutWidget_3 = QtGui.QWidget(self.groupBox_2)
        self.horizontalLayoutWidget_3.setGeometry(QtCore.QRect(10, 30, 501, 31))
        self.horizontalLayoutWidget_3.setObjectName("horizontalLayoutWidget_3")
        self.horizontalLayout_3 = QtGui.QHBoxLayout(self.horizontalLayoutWidget_3)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.radioButton_9 = QtGui.QRadioButton(self.horizontalLayoutWidget_3)
        self.radioButton_9.setChecked(True)
        self.radioButton_9.setObjectName("radioButton_9")
        self.horizontalLayout_3.addWidget(self.radioButton_9)
        self.label = QtGui.QLabel(self.horizontalLayoutWidget_3)
        self.label.setObjectName("label")
        self.horizontalLayout_3.addWidget(self.label)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.radioButton_5 = QtGui.QRadioButton(self.horizontalLayoutWidget_3)
        self.radioButton_5.setObjectName("radioButton_5")
        self.horizontalLayout_2.addWidget(self.radioButton_5)
        self.radioButton_6 = QtGui.QRadioButton(self.horizontalLayoutWidget_3)
        self.radioButton_6.setObjectName("radioButton_6")
        self.horizontalLayout_2.addWidget(self.radioButton_6)
        self.radioButton_7 = QtGui.QRadioButton(self.horizontalLayoutWidget_3)
        self.radioButton_7.setObjectName("radioButton_7")
        self.horizontalLayout_2.addWidget(self.radioButton_7)
        self.radioButton_8 = QtGui.QRadioButton(self.horizontalLayoutWidget_3)
        self.radioButton_8.setObjectName("radioButton_8")
        self.horizontalLayout_2.addWidget(self.radioButton_8)
        self.horizontalLayout_3.addLayout(self.horizontalLayout_2)
        self.pushButton_2 = QtGui.QPushButton(self.centralwidget)
        self.pushButton_2.setGeometry(QtCore.QRect(770, 400, 115, 32))
        self.pushButton_2.setObjectName("pushButton_2")
        self.groupBox_4 = QtGui.QGroupBox(self.centralwidget)
        self.groupBox_4.setGeometry(QtCore.QRect(20, 110, 871, 141))
        self.groupBox_4.setObjectName("groupBox_4")
        self.horizontalLayoutWidget_4 = QtGui.QWidget(self.groupBox_4)
        self.horizontalLayoutWidget_4.setGeometry(QtCore.QRect(10, 40, 211, 81))
        self.horizontalLayoutWidget_4.setObjectName("horizontalLayoutWidget_4")
        self.horizontalLayout_4 = QtGui.QHBoxLayout(self.horizontalLayoutWidget_4)
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.radioButton_10 = QtGui.QRadioButton(self.horizontalLayoutWidget_4)
        self.radioButton_10.setChecked(True)
        self.radioButton_10.setObjectName("radioButton_10")
        self.horizontalLayout_4.addWidget(self.radioButton_10)
        self.radioButton_11 = QtGui.QRadioButton(self.horizontalLayoutWidget_4)
        self.radioButton_11.setObjectName("radioButton_11")
        self.horizontalLayout_4.addWidget(self.radioButton_11)
        self.widget = QtGui.QWidget(self.groupBox_4)
        self.widget.setEnabled(True)
        self.widget.setGeometry(QtCore.QRect(220, 30, 861, 101))
        self.widget.setObjectName("widget")
        self.layoutWidget = QtGui.QWidget(self.widget)
        self.layoutWidget.setGeometry(QtCore.QRect(10, 10, 207, 84))
        self.layoutWidget.setObjectName("layoutWidget")
        self.verticalLayout_5 = QtGui.QVBoxLayout(self.layoutWidget)
        self.verticalLayout_5.setObjectName("verticalLayout_5")
        self.horizontalLayout_12 = QtGui.QHBoxLayout()
        self.horizontalLayout_12.setObjectName("horizontalLayout_12")
        self.label_4 = QtGui.QLabel(self.layoutWidget)
        self.label_4.setEnabled(True)
        self.label_4.setObjectName("label_4")
        self.horizontalLayout_12.addWidget(self.label_4)
        self.lineEdit_2 = QtGui.QLineEdit(self.layoutWidget)
        self.lineEdit_2.setEnabled(True)
        self.lineEdit_2.setObjectName("lineEdit_2")
        self.horizontalLayout_12.addWidget(self.lineEdit_2)
        self.label_8 = QtGui.QLabel(self.layoutWidget)
        self.label_8.setEnabled(True)
        self.label_8.setObjectName("label_8")
        self.horizontalLayout_12.addWidget(self.label_8)
        self.verticalLayout_5.addLayout(self.horizontalLayout_12)
        self.label_13 = QtGui.QLabel(self.layoutWidget)
        self.label_13.setText("")
        self.label_13.setAlignment(QtCore.Qt.AlignCenter)
        self.label_13.setObjectName("label_13")
        self.verticalLayout_5.addWidget(self.label_13)
        self.horizontalLayout_13 = QtGui.QHBoxLayout()
        self.horizontalLayout_13.setObjectName("horizontalLayout_13")
        self.label_14 = QtGui.QLabel(self.layoutWidget)
        self.label_14.setObjectName("label_14")
        self.horizontalLayout_13.addWidget(self.label_14)
        self.lineEdit_7 = QtGui.QLineEdit(self.layoutWidget)
        self.lineEdit_7.setObjectName("lineEdit_7")
        self.horizontalLayout_13.addWidget(self.lineEdit_7)
        self.label_15 = QtGui.QLabel(self.layoutWidget)
        self.label_15.setObjectName("label_15")
        self.horizontalLayout_13.addWidget(self.label_15)
        self.verticalLayout_5.addLayout(self.horizontalLayout_13)
        self.layoutWidget1 = QtGui.QWidget(self.widget)
        self.layoutWidget1.setGeometry(QtCore.QRect(230, 10, 331, 84))
        self.layoutWidget1.setObjectName("layoutWidget1")
        self.verticalLayout_6 = QtGui.QVBoxLayout(self.layoutWidget1)
        self.verticalLayout_6.setObjectName("verticalLayout_6")
        self.horizontalLayout_14 = QtGui.QHBoxLayout()
        self.horizontalLayout_14.setObjectName("horizontalLayout_14")
        self.label_16 = QtGui.QLabel(self.layoutWidget1)
        self.label_16.setEnabled(True)
        self.label_16.setObjectName("label_16")
        self.horizontalLayout_14.addWidget(self.label_16)
        self.lineEdit_8 = QtGui.QLineEdit(self.layoutWidget1)
        self.lineEdit_8.setEnabled(True)
        self.lineEdit_8.setObjectName("lineEdit_8")
        self.horizontalLayout_14.addWidget(self.lineEdit_8)
        spacerItem = QtGui.QSpacerItem(140, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_14.addItem(spacerItem)
        self.verticalLayout_6.addLayout(self.horizontalLayout_14)
        self.label_17 = QtGui.QLabel(self.layoutWidget1)
        self.label_17.setEnabled(True)
        self.label_17.setAlignment(QtCore.Qt.AlignCenter)
        self.label_17.setObjectName("label_17")
        self.verticalLayout_6.addWidget(self.label_17)
        self.horizontalLayout_15 = QtGui.QHBoxLayout()
        self.horizontalLayout_15.setObjectName("horizontalLayout_15")
        self.label_18 = QtGui.QLabel(self.layoutWidget1)
        self.label_18.setEnabled(True)
        self.label_18.setObjectName("label_18")
        self.horizontalLayout_15.addWidget(self.label_18)
        self.lineEdit_4 = QtGui.QLineEdit(self.layoutWidget1)
        self.lineEdit_4.setEnabled(True)
        self.lineEdit_4.setObjectName("lineEdit_4")
        self.horizontalLayout_15.addWidget(self.lineEdit_4)
        self.label_19 = QtGui.QLabel(self.layoutWidget1)
        self.label_19.setEnabled(True)
        self.label_19.setObjectName("label_19")
        self.horizontalLayout_15.addWidget(self.label_19)
        self.verticalLayout_6.addLayout(self.horizontalLayout_15)
        self.verticalLayoutWidget_4 = QtGui.QWidget(self.widget)
        self.verticalLayoutWidget_4.setGeometry(QtCore.QRect(570, 10, 66, 86))
        self.verticalLayoutWidget_4.setObjectName("verticalLayoutWidget_4")
        self.verticalLayout_7 = QtGui.QVBoxLayout(self.verticalLayoutWidget_4)
        self.verticalLayout_7.setObjectName("verticalLayout_7")
        self.radioButton_14 = QtGui.QRadioButton(self.verticalLayoutWidget_4)
        self.radioButton_14.setObjectName("radioButton_14")
        self.verticalLayout_7.addWidget(self.radioButton_14)
        self.radioButton_15 = QtGui.QRadioButton(self.verticalLayoutWidget_4)
        self.radioButton_15.setObjectName("radioButton_15")
        self.verticalLayout_7.addWidget(self.radioButton_15)
        self.groupBox_5 = QtGui.QGroupBox(self.centralwidget)
        self.groupBox_5.setGeometry(QtCore.QRect(20, 280, 871, 101))
        self.groupBox_5.setObjectName("groupBox_5")
        self.label_2 = QtGui.QLabel(self.groupBox_5)
        self.label_2.setGeometry(QtCore.QRect(20, 30, 101, 31))
        self.label_2.setMaximumSize(QtCore.QSize(270, 16777215))
        self.label_2.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.label_2.setObjectName("label_2")
        self.pushButton = QtGui.QPushButton(self.groupBox_5)
        self.pushButton.setEnabled(True)
        self.pushButton.setGeometry(QtCore.QRect(110, 30, 731, 32))
        self.pushButton.setObjectName("pushButton")
        self.label_3 = QtGui.QLabel(self.groupBox_5)
        self.label_3.setGeometry(QtCore.QRect(10, 70, 91, 20))
        self.label_3.setMaximumSize(QtCore.QSize(270, 16777215))
        self.label_3.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.label_3.setObjectName("label_3")
        self.lineEdit = QtGui.QLineEdit(self.groupBox_5)
        self.lineEdit.setGeometry(QtCore.QRect(120, 70, 711, 22))
        self.lineEdit.setObjectName("lineEdit")
        DataRefl.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(DataRefl)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 913, 22))
        self.menubar.setObjectName("menubar")
        DataRefl.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(DataRefl)
        self.statusbar.setObjectName("statusbar")
        DataRefl.setStatusBar(self.statusbar)

        self.retranslateUi(DataRefl)
        QtCore.QMetaObject.connectSlotsByName(DataRefl)

    def retranslateUi(self, DataRefl):
        DataRefl.setWindowTitle(QtGui.QApplication.translate("DataRefl", "MainWindow", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("DataRefl", "Run reduction for following data spin states", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_2.setText(QtGui.QApplication.translate("DataRefl", "Off_On", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_3.setText(QtGui.QApplication.translate("DataRefl", "On_Off", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton.setText(QtGui.QApplication.translate("DataRefl", "Off_Off", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_4.setText(QtGui.QApplication.translate("DataRefl", "On_On", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_2.setTitle(QtGui.QApplication.translate("DataRefl", "Normalization spin states", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_9.setText(QtGui.QApplication.translate("DataRefl", "Same as data file", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("DataRefl", " or ", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_5.setText(QtGui.QApplication.translate("DataRefl", "Off_On", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_6.setText(QtGui.QApplication.translate("DataRefl", "On_Off", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_7.setText(QtGui.QApplication.translate("DataRefl", "Off_Off", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_8.setText(QtGui.QApplication.translate("DataRefl", "On_On", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButton_2.setText(QtGui.QApplication.translate("DataRefl", "Advanced...", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_4.setTitle(QtGui.QApplication.translate("DataRefl", "Q range", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_10.setText(QtGui.QApplication.translate("DataRefl", "Automatic         ", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_11.setText(QtGui.QApplication.translate("DataRefl", "Manual", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("DataRefl", "    from", None, QtGui.QApplication.UnicodeUTF8))
        self.label_8.setText(QtGui.QApplication.translate("DataRefl", "Angstroms", None, QtGui.QApplication.UnicodeUTF8))
        self.label_14.setText(QtGui.QApplication.translate("DataRefl", "        to", None, QtGui.QApplication.UnicodeUTF8))
        self.label_15.setText(QtGui.QApplication.translate("DataRefl", "Angstroms", None, QtGui.QApplication.UnicodeUTF8))
        self.label_16.setText(QtGui.QApplication.translate("DataRefl", "             Nbr bins", None, QtGui.QApplication.UnicodeUTF8))
        self.label_17.setText(QtGui.QApplication.translate("DataRefl", "or", None, QtGui.QApplication.UnicodeUTF8))
        self.label_18.setText(QtGui.QApplication.translate("DataRefl", "              bin size", None, QtGui.QApplication.UnicodeUTF8))
        self.label_19.setText(QtGui.QApplication.translate("DataRefl", "Angstroms        ", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_14.setText(QtGui.QApplication.translate("DataRefl", "Linear", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_15.setText(QtGui.QApplication.translate("DataRefl", "Log", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_5.setTitle(QtGui.QApplication.translate("DataRefl", "Output file name", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("DataRefl", "Output folder", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButton.setText(QtGui.QApplication.translate("DataRefl", "~/results/", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("DataRefl", "File name", None, QtGui.QApplication.UnicodeUTF8))

