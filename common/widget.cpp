#include <QtGui>
#include <QDebug>
#include "widget.h"
#include "protocol.h"
#include "protocol_items.h"

Widget::Widget()
	: QMainWindow()
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
	
	resize(400, 500);
	
	Command::initializeHash();
}

void Widget::startClicked()
{
	currentItem = 0;
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("cockatrice_communication");
	xmlWriter.writeAttribute("version", "4");
	
	Command *test = new Command_Ping;
	test->write(xmlWriter);
	
	Command *test2 = new Command_ChatLeaveChannel("foobar");
	test2->write(xmlWriter);
	
	Command *test3 = new Command_ChatSay("foobar", "Hallo, dies ist ein Test");
	test3->write(xmlWriter);
	
	ProtocolResponse *test4 = new ProtocolResponse(123, ProtocolResponse::RespContextError);
	test4->write(xmlWriter);
	
	GameEvent *test5 = new Event_RollDie(1234, true, 1, 20, 13);
	test5->write(xmlWriter);
}

bool Widget::readCurrentCommand()
{
	if (!currentItem)
		return false;
	if (currentItem->read(xmlReader)) {
		qDebug() << "setting to 0";
		currentItem = 0;
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
			QString itemType = xmlReader.name().toString();
			QString itemName = xmlReader.attributes().value("name").toString();
			qDebug() << "parseXml: startElement: " << "type =" << itemType << ", name =" << itemName;
			currentItem = ProtocolItem::getNewItem(itemType + itemName);
			if (!currentItem)
				qDebug() << "unrecognized item";
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
