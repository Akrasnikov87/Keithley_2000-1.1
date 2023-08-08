#-------------------------------------------------
#
# Project created by QtCreator 2016-08-30T11:14:36
#
#-------------------------------------------------

QT					+=	core gui widgets serialport printsupport

CONFIG				+=	c++11

TARGET				=	Keithley_2000_1.1
TEMPLATE			=	app


SOURCES				+=	main.cpp\
						mainwindow.cpp \
						qcustomplot.cpp

HEADERS				+=	mainwindow.h \
						qcustomplot.h

RESOURCES			+= \
    res.qrc
