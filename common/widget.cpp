#include <QtGui>
#include <QDebug>
#include "widget.h"
#include "protocol.h"
#include "protocol_commands.h"

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
	
	Command::initializeHash();
}

void Widget::startClicked()
{
	currentCommand = 0;
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("cockatrice_communication");
	xmlWriter.writeAttribute("version", "4");
	
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
			currentCommand = Command::getNewCommand(cmdStr);
			if (!currentCommand)
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
