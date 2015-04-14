#-------------------------------------------------
#
# Project created by QtCreator 2015-04-11T10:34:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = e-nose

target.files = e-nose
target.path = /root/qt_program
INSTALLS = target

TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    common.c \
    ds18b20.c \
    GPIO.c \
    HW_interface.c \
    i2c.c \
    pid.c \
    PWM.c \
    sht21.c \
    spidev.c \
    tlc1543.c \
    uart.c \
    mythread.cpp

HEADERS  += mainwindow.h \
    common.h \
    ds18b20.h \
    GPIO.h \
    HW_interface.h \
    i2c.h \
    pid.h \
    PWM.h \
    sht21.h \
    spidev.h \
    tlc1543.h \
    uart.h \
    mythread.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    恒温调节.ods \
    气泵电机转速相关测试数据.ods \
    README.md \
    LCD_Images/480X272像素3.jpg

RESOURCES += \
    LCD_Images/LCD_backround_images.qrc
