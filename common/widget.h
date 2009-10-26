#ifndef WIDGET_H
#define WIDGET_H

#include <QMainWindow>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class QTextEdit;
class QPushButton;
class QBuffer;
class Command;

class Widget : public QMainWindow {
	Q_OBJECT
private:
	QTextEdit *edit1;
	QPushButton *start;
	QBuffer *buffer;
	QXmlStreamReader xmlReader;
	QXmlStreamWriter xmlWriter;
	
	Command *currentCommand;
	bool readCurrentCommand();
	void parseBuffer();
	void parseXml();
private slots:
	void startClicked();
	void updateEdit();
public:
	Widget();
};

#endif
