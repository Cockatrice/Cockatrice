#include "recording.h"

#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>

XMLEventObserver::XMLEventObserver(QObject *p) : pqEventObserver(p)
{
    XMLStream = nullptr;
}

XMLEventObserver::~XMLEventObserver()
{
    delete XMLStream;
}

void XMLEventObserver::setStream(QTextStream *stream)
{
    if (XMLStream) {
        XMLStream->writeEndElement();
        XMLStream->writeEndDocument();
        delete XMLStream;
        XMLStream = nullptr;
    }
    if (Stream) {
        *Stream << XMLString;
    }
    XMLString = QString();
    pqEventObserver::setStream(stream);
    if (Stream) {
        XMLStream = new QXmlStreamWriter(&XMLString);
        XMLStream->setAutoFormatting(true);
        XMLStream->writeStartDocument();
        XMLStream->writeStartElement("events");
    }
}

void XMLEventObserver::onRecordEvent(const QString &widget,
                                     const QString &command,
                                     const QString &arguments,
                                     const int &eventType)
{
    if (XMLStream) {
        XMLStream->writeStartElement("event");
        XMLStream->writeAttribute("widget", widget);
        if (eventType == pqEventTypes::ACTION_EVENT) {
            XMLStream->writeAttribute("command", command);
        } else // if(eventType == pqEventTypes::CHECK_EVENT)
        {
            XMLStream->writeAttribute("property", command);
        }
        XMLStream->writeAttribute("arguments", arguments);
        XMLStream->writeEndElement();
    }
}

XMLEventSource::XMLEventSource(QObject *p) : pqEventSource(p)
{
    XMLStream = nullptr;
}

XMLEventSource::~XMLEventSource()
{
    delete XMLStream;
}

void XMLEventSource::setContent(const QString &xmlFileName)
{
    delete XMLStream;
    XMLStream = nullptr;

    QFile xml(xmlFileName);
    if (!xml.open(QIODevice::ReadOnly)) {
        qCWarning(RecordingLog) << "Failed to load " << xmlFileName;
        return;
    }
    QByteArray data = xml.readAll();
    XMLStream = new QXmlStreamReader(data);
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
    if (XMLStream->atEnd()) {
        qCWarning(RecordingLog) << "Invalid xml from" << xmlFileName;
    }
}

int XMLEventSource::getNextEvent(QString &widget, QString &command, QString &arguments, int &eventType)
{
    if (XMLStream->atEnd()) {
        return DONE;
    }
    while (!XMLStream->atEnd()) {
        QXmlStreamReader::TokenType token = XMLStream->readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (XMLStream->name() == "event") {
                break;
            }
        }
    }
    if (XMLStream->atEnd()) {
        return DONE;
    }
    eventType = pqEventTypes::ACTION_EVENT;
    widget = XMLStream->attributes().value("widget").toString();
    command = XMLStream->attributes().value("command").toString();
    arguments = XMLStream->attributes().value("arguments").toString();
    return SUCCESS;
}
