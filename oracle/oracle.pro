TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src ../cockatrice/src
MOC_DIR = build
OBJECTS_DIR = build
QT += network svg xml

HEADERS += src/oracleimporter.h src/window_main.h ../cockatrice/src/carddatabase.h
SOURCES += src/main.cpp src/oracleimporter.cpp src/window_main.cpp ../cockatrice/src/carddatabase.cpp
