#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include "protocol.h"
#include "protocol_commands.h"

QHash<QString, Command::NewCommandFunction> Command::commandHash;

Command::Command(const QString &_cmdName)
	: cmdName(_cmdName)
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
				extractParameters();
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

Command *Command::getNewCommand(const QString &name)
{
	if (!commandHash.contains(name))
		return 0;
	return commandHash.value(name)();
}
