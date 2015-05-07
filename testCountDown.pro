#-------------------------------------------------
#
# Project created by QtCreator 2015-05-03T13:27:49
#
#-------------------------------------------------

QT       += core gui\
            xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testCountDown
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ftdichip.cpp \
    parserxml.cpp \
    dialog.cpp

HEADERS  += mainwindow.h \
    ftdichip.h \
    parserxml.h \
    dialog.h

FORMS    += mainwindow.ui \
    dialog.ui

RESOURCES += \
    QCurrentResource.qrc

RC_FILE +=\
    WinResource.rc

TRANSLATIONS += \
    TR/TestCountDown_en.ts
LIBS += "D:\Qt\Qt5.4.1\Examples\_Project\_TOO_PITER\testCountDown\FTDI\FTD2XX.lib"
