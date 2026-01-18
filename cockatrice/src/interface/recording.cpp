#include "recording.h"

#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>

XMLEventObserver::XMLEventObserver(QObject *p) : pqEventObserver(p)
{
    xmlStream = nullptr;
}

XMLEventObserver::~XMLEventObserver()
{
    delete xmlStream;
}

void XMLEventObserver::setStream(QTextStream *stream)
{
    if (xmlStream) {
        xmlStream->writeEndElement();
        xmlStream->writeEndDocument();
        delete xmlStream;
        xmlStream = nullptr;
    }
    if (Stream) {
        *Stream << xmlString;
    }
    xmlString = QString();
    pqEventObserver::setStream(stream);
    if (Stream) {
        xmlStream = new QXmlStreamWriter(&xmlString);
        xmlStream->setAutoFormatting(true);
        xmlStream->writeStartDocument();
        xmlStream->writeStartElement("events");
    }
}

void XMLEventObserver::onRecordEvent(const QString &widget,
                                     const QString &command,
                                     const QString &arguments,
                                     const int &eventType)
{
    if (xmlStream) {
        xmlStream->writeStartElement("event");
        xmlStream->writeAttribute("widget", widget);
        if (eventType == pqEventTypes::ACTION_EVENT) {
            xmlStream->writeAttribute("command", command);
        } else // if(eventType == pqEventTypes::CHECK_EVENT)
        {
            xmlStream->writeAttribute("property", command);
        }
        xmlStream->writeAttribute("arguments", arguments);
        xmlStream->writeEndElement();
    }
}

XMLEventSource::XMLEventSource(QObject *p) : pqEventSource(p)
{
    xmlStream = nullptr;
}

XMLEventSource::~XMLEventSource()
{
    delete xmlStream;
}

void XMLEventSource::setContent(const QString &xmlFileName)
{
    delete xmlStream;
    xmlStream = nullptr;

    QFile xml(xmlFileName);
    if (!xml.open(QIODevice::ReadOnly)) {
        qCWarning(RecordingLog) << "Failed to load " << xmlFileName;
        return;
    }
    QByteArray data = xml.readAll();
    xmlStream = new QXmlStreamReader(data);
    /* This checked for valid event objects, but also caused the first event
     * to get dropped. Commenting this out in the example. If you wish to report
     * empty XML test files a flag indicating whether valid events were found is
     * probably the best way to go.
    while (!XMLStream->atEnd())
      {
      QXmlStreamReader::TokenType token = XMLStream->readNext();
      if (token == QXmlStreamReader::StartElement)
        {
        if (XMLStream->name() == "event")
          {
          break;
          }
        }
      } */
    if (xmlStream->atEnd()) {
        qCWarning(RecordingLog) << "Invalid xml from" << xmlFileName;
    }
}

int XMLEventSource::getNextEvent(QString &widget, QString &command, QString &arguments, int &eventType)
{
    if (xmlStream->atEnd()) {
        return DONE;
    }
    while (!xmlStream->atEnd()) {
        QXmlStreamReader::TokenType token = xmlStream->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xmlStream->name() == "event") {
                break;
            }
        }
    }
    if (xmlStream->atEnd()) {
        return DONE;
    }
    eventType = pqEventTypes::ACTION_EVENT;
    widget = xmlStream->attributes().value("widget").toString();
    command = xmlStream->attributes().value("command").toString();
    arguments = xmlStream->attributes().value("arguments").toString();
    return SUCCESS;
}
