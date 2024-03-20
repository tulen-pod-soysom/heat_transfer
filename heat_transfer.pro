QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    heat_renderer.cpp \
    main.cpp \
    mainwindow.cpp


HEADERS += \
    heat_renderer.h \
    mainwindow.h \
    heat_transfer_program.hpp

FORMS += \
    mainwindow.ui

INCLUDEPATH += \
    C:\libs\boost_1_82_0 \
    C:\my_lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
