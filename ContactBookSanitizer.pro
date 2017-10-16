#-------------------------------------------------
#
# Project created by QtCreator 2017-09-16T20:09:36
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ContactBookSanitizer
TEMPLATE = app
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
	main.cpp\
	MainWindow.cpp \
	Session.cpp \
	ContactBook.cpp \
	SessionModel.cpp \
	ContactBookModel.cpp \
	Device.cpp \
	ExampleDevice.cpp \
	DeviceVcfFile.cpp \
	VCardParser.cpp \
	Contact.cpp \
	DisplayContact.cpp

HEADERS  += \
	MainWindow.h \
	Session.h \
	ContactBook.h \
	SessionModel.h \
	ContactBookModel.h \
	Device.h \
	ExampleDevice.h \
	DeviceVcfFile.h \
	VCardParser.h \
	Exceptions.h \
	Contact.h \
	DisplayContact.h

FORMS    += MainWindow.ui
