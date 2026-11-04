#include "recording.h"

#include "pqEventTypes.h"

#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

XMLEventObserver::XMLEventObserver(QObject *parent) : pqEventObserver(parent)
{
}

XMLEventObserver::~XMLEventObserver() = default;

void XMLEventObserver::setStream(QTextStream *stream)
{
    if (xmlStream) {
        xmlStream->writeEndElement();
        xmlStream->writeEndDocument();
        xmlStream.reset();
    }
    if (Stream) {
        *Stream << xmlString;
    }
    xmlString.clear();
    pqEventObserver::setStream(stream);
    if (Stream) {
        xmlStream.reset(new QXmlStreamWriter(&xmlString));
        xmlStream->setAutoFormatting(true);
        xmlStream->writeStartDocument();
        xmlStream->writeStartElement(QStringLiteral("events"));
    }
}

void XMLEventObserver::onRecordEvent(const QString &widget,
                                     const QString &command,
                                     const QString &arguments,
                                     const int &eventType)
{
    if (!xmlStream) {
        return;
    }

    xmlStream->writeStartElement(QStringLiteral("event"));
    xmlStream->writeAttribute(QStringLiteral("widget"), widget);
    if (eventType == pqEventTypes::ACTION_EVENT) {
        xmlStream->writeAttribute(QStringLiteral("command"), command);
    } else { // pqEventTypes::CHECK_EVENT
        xmlStream->writeAttribute(QStringLiteral("property"), command);
    }
    xmlStream->writeAttribute(QStringLiteral("arguments"), arguments);
    xmlStream->writeEndElement();
}

XMLEventSource::XMLEventSource(QObject *parent) : pqEventSource(parent)
{
}

XMLEventSource::~XMLEventSource() = default;

void XMLEventSource::setContent(const QString &xmlFileName)
{
    xmlStream.reset();

    QFile xml(xmlFileName);
    if (!xml.open(QIODevice::ReadOnly)) {
        qCWarning(RecordingLog) << "Could not read test file" << xmlFileName;
        return;
    }

    xmlStream.reset(new QXmlStreamReader(xml.readAll()));
}

int XMLEventSource::getNextEvent(QString &widget, QString &command, QString &arguments, int &eventType)
{
    if (!xmlStream) {
        return DONE;
    }

    while (!xmlStream->atEnd()) {
        if (xmlStream->readNext() != QXmlStreamReader::StartElement || xmlStream->name() != QStringLiteral("event")) {
            continue;
        }

        const QXmlStreamAttributes attributes = xmlStream->attributes();
        eventType = pqEventTypes::ACTION_EVENT;
        widget = attributes.value(QStringLiteral("widget")).toString();
        command = attributes.value(QStringLiteral("command")).toString();
        if (command.isEmpty()) {
            // A recorded check event carries a property instead of a command.
            command = attributes.value(QStringLiteral("property")).toString();
        }
        arguments = attributes.value(QStringLiteral("arguments")).toString();
        return SUCCESS;
    }

    if (xmlStream->hasError()) {
        qCWarning(RecordingLog) << "Malformed test file:" << xmlStream->errorString();
    }
    return DONE;
}
