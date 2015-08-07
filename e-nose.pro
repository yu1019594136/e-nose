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
    ds18b20.c \
    i2c.c \
    pid.c \
    PWM.c \
    sht21.c \
    spidev.c \
    tlc1543.c \
    uart.c \
    thread_logic.cpp \
    thread_hardware.cpp \
    thread_data_proc.cpp \
    HW_interface.cpp \
    common.c \
    GPIO.c \
    plot_widget.cpp \
    qcommon.cpp \
    myinputpanel.cpp \
    myinputpanelcontext.cpp

HEADERS  += mainwindow.h \
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
    thread_logic.h \
    thread_hardware.h \
    thread_data_proc.h \
    common.h \
    plot_widget.h \
    qcommon.h \
    myinputpanel.h \
    myinputpanelcontext.h

FORMS    += mainwindow.ui \
    myinputpanelform.ui

OTHER_FILES += \
    恒温调节.ods \
    气泵电机转速相关测试数据.ods \
    README.md \
    LCD_Images/480X272像素3.jpg

RESOURCES += \
    lcd_backround.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/am335x_pru_package-master/pru_sw/app_loader/lib/release/ -lprussdrv
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/am335x_pru_package-master/pru_sw/app_loader/lib/debug/ -lprussdrv
else:unix: LIBS += -L$$PWD/am335x_pru_package-master/pru_sw/app_loader/lib/ -lprussdrv

INCLUDEPATH += $$PWD/am335x_pru_package-master/pru_sw/app_loader/include
DEPENDPATH += $$PWD/am335x_pru_package-master/pru_sw/app_loader/include
