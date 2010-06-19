TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src ../cockatrice/src
MOC_DIR = build
OBJECTS_DIR = build
QT += network svg xml

HEADERS += src/oracleimporter.h src/window_main.h ../cockatrice/src/carddatabase.h ../cockatrice/src/settingscache.h
SOURCES += src/main.cpp src/oracleimporter.cpp src/window_main.cpp ../cockatrice/src/carddatabase.cpp ../cockatrice/src/settingscache.cpp

macx {
	CONFIG += x86 ppc
	LIBS += -bind_at_load
}
