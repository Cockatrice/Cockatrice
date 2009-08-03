TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src ../cockatrice/src
MOC_DIR = build
OBJECTS_DIR = build
QT += network svg

HEADERS += src/oracleimporter.h ../cockatrice/src/carddatabase.h
SOURCES += src/main.cpp src/oracleimporter.cpp ../cockatrice/src/carddatabase.cpp
