#ifndef COCKATRICE_RECORDING_H
#define COCKATRICE_RECORDING_H

#include "pqEventObserver.h"
#include "pqEventSource.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(RecordingLog, "recording");

class QXmlStreamReader;
class QXmlStreamWriter;

class XMLEventObserver : public pqEventObserver
{
    QXmlStreamWriter *XMLStream;
    QString XMLString;

public:
    explicit XMLEventObserver(QObject *p);
    ~XMLEventObserver() override;

protected:
    void setStream(QTextStream *stream) override;
    void onRecordEvent(const QString &widget,
                       const QString &command,
                       const QString &arguments,
                       const int &eventType) override;
};

class XMLEventSource : public pqEventSource
{
    QXmlStreamReader *XMLStream;

public:
    explicit XMLEventSource(QObject *p);
    ~XMLEventSource() override;

protected:
    void setContent(const QString &xmlFileName) override;
    int getNextEvent(QString &widget, QString &command, QString &arguments, int &eventType) override;
};

#endif // COCKATRICE_RECORDING_H
