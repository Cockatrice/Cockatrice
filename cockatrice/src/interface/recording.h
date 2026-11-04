#ifndef COCKATRICE_RECORDING_H
#define COCKATRICE_RECORDING_H

#include "pqEventObserver.h"
#include "pqEventSource.h"

#include <QLoggingCategory>
#include <QScopedPointer>

inline Q_LOGGING_CATEGORY(RecordingLog, "recording");

class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief Writes a session recording as an XML test file.
 *
 * The events are buffered in memory and flushed to the stream when recording
 * ends, because the stream is handed over once the file name is known.
 */
class XMLEventObserver : public pqEventObserver
{
    QScopedPointer<QXmlStreamWriter> xmlStream;
    QString xmlString;

public:
    explicit XMLEventObserver(QObject *parent);
    ~XMLEventObserver() override;

protected:
    void setStream(QTextStream *stream) override;
    void onRecordEvent(const QString &widget,
                       const QString &command,
                       const QString &arguments,
                       const int &eventType) override;
};

/**
 * @brief Plays the events of an XML test file back, one at a time.
 */
class XMLEventSource : public pqEventSource
{
    QScopedPointer<QXmlStreamReader> xmlStream;

public:
    explicit XMLEventSource(QObject *parent);
    ~XMLEventSource() override;

protected:
    void setContent(const QString &xmlFileName) override;
    int getNextEvent(QString &widget, QString &command, QString &arguments, int &eventType) override;
};

#endif // COCKATRICE_RECORDING_H
