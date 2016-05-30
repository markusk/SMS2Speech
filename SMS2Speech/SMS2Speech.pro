#-------------------------------------------------
#
# Project created by QtCreator 2016-03-10T10:51:28
#
#-------------------------------------------------

QT		+= core gui
QT		+= serialport

# CONFIG += c++11
QMAKE_CXXFLAGS += -stdlib=libc++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SMS2Speech
ICON = SMS2Speech.icns
TEMPLATE = app

SOURCES += main.cpp\
			mainwindow.cpp \
			circuit.cpp \
			qtSerial.cpp \
			interfaceAvr.cpp \
			QtSpeech_mac.cpp \
			speakThread.cpp



HEADERS  += mainwindow.h \
			circuit.h \
			qtSerial.cpp \
			interfaceAvr.h \
			QtSpeech.h \
			speakThread.h


FORMS    += mainwindow.ui

# For QtSpeech!
LIBS *= -framework AppKit
