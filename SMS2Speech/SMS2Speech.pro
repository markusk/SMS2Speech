#-------------------------------------------------
#
# Project created by QtCreator 2016-03-10T10:51:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SMS2Speech
ICON = SMS2Speech.icns
TEMPLATE = app

SOURCES += main.cpp\
			mainwindow.cpp \
			circuit.cpp \
			direcsSerial.cpp \
			interfaceAvr.cpp \
			QtSpeech_mac.cpp \
			speakThread.cpp



HEADERS  += mainwindow.h \
			circuit.h \
			direcsSerial.h \
			interfaceAvr.h \
			QtSpeech.h \
			speakThread.h


FORMS    += mainwindow.ui

# For QtSpeech!
LIBS *= -framework AppKit
