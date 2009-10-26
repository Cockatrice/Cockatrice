#include <QtGui>
#include <QDebug>
#include "widget.h"
#include "protocol.h"

Widget::Widget()
	: QMainWindow(), currentCommand(0)
{
	edit1 = new QTextEdit;
	start = new QPushButton;
	connect(start, SIGNAL(clicked()), this, SLOT(startClicked()));
	
	buffer = new QBuffer;
	buffer->open(QIODevice::ReadWrite);
	connect(buffer, SIGNAL(readyRead()), this, SLOT(updateEdit()));
	
	xmlWriter.setDevice(buffer);
	xmlWriter.setAutoFormatting(true);
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(edit1);
	vbox->addWidget(start);
	
	QWidget *central = new QWidget;
	central->setLayout(vbox);
	setCentralWidget(central);
}

void Widget::startClicked()
{
	currentCommand = 0;
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("cockatrice_communication");
	
	Command *test = new Command_Ping;
	test->write(xmlWriter);
	
	Command *test2 = new Command_ChatLeaveChannel("foobar");
	test2->write(xmlWriter);
	
	Command *test3 = new Command_ChatSay("foobar", "Hallo, dies ist ein Test");
	test3->write(xmlWriter);
}

bool Widget::readCurrentCommand()
{
	if (!currentCommand)
		return false;
	if (currentCommand->read(xmlReader)) {
		qDebug() << "setting to 0";
		currentCommand = 0;
	}
	return true;
}

void Widget::parseXml()
{
	if (readCurrentCommand())
		return;
	
	while (!xmlReader.atEnd()) {
		xmlReader.readNext();
		if (xmlReader.isStartElement()) {
			QString cmdStr = xmlReader.name().toString();
			qDebug() << "parseXml: startElement: " << cmdStr;
			if (cmdStr == "ping")
				currentCommand = new Command_Ping;
			else if (cmdStr == "chat_leave_channel")
				currentCommand = new Command_ChatLeaveChannel;
			else if (cmdStr == "chat_say")
				currentCommand = new Command_ChatSay;
			else
				qDebug() << "unrecognized command";
			readCurrentCommand();
		}
	}
}

void Widget::parseBuffer()
{
	xmlReader.clear();
	buffer->seek(0);
	while (!buffer->atEnd()) {
		QByteArray oneByte = buffer->read(1);
		xmlReader.addData(oneByte);
		parseXml();
	}
}

void Widget::updateEdit()
{
	buffer->seek(0);
	edit1->setText(buffer->readAll());
	
	parseBuffer();
}
