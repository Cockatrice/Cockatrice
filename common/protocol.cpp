#include "protocol.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

Command::Command(const QString &_cmdName)
	: cmdName(_cmdName)
{
	
}

void Command::validateParameters()
{
}

bool Command::read(QXmlStreamReader &xml)
{
	while (!xml.atEnd()) {
		xml.readNext();
		if (xml.isStartElement()) {
			qDebug() << "startElement: " << xml.name().toString();
		} else if (xml.isEndElement()) {
			qDebug() << "endElement: " << xml.name().toString();
			if (xml.name() == cmdName) {
				validateParameters();
				qDebug() << "FERTIG";
				deleteLater();
				return true;
			} else {
				QString tagName = xml.name().toString();
				if (!parameters.contains(tagName))
					qDebug() << "unrecognized attribute";
				else
					parameters[tagName] = currentElementText;
			}
		} else if (xml.isCharacters() && !xml.isWhitespace()) {
			currentElementText = xml.text().toString();
			qDebug() << "text: " << currentElementText;
		}
	}
	return false;
}

void Command::write(QXmlStreamWriter &xml)
{
	xml.writeStartElement(cmdName);
	
	QMapIterator<QString, QString> i(parameters);
	while (i.hasNext()) {
		i.next();
		xml.writeTextElement(i.key(), i.value());
	}
	
	xml.writeEndElement();
}
