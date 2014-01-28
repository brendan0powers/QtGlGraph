#-------------------------------------------------
#
# Project created by QtCreator 2014-01-26T13:06:29
#
#-------------------------------------------------

QT       += core gui opengl widgets

TARGET = GlGraph
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        glgraphwidget.cpp

HEADERS  += mainwindow.h \
         glgraphwidget.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    graphshader.vert \
    graphshader.frag \
    gridshader.vert \
    gridshader.frag

RESOURCES += \
    Shaders.qrc
