#ifndef COCKATRICE_URL_SCHEME_EVENT_FILTER_H
#define COCKATRICE_URL_SCHEME_EVENT_FILTER_H

#include <QEvent>
#include <QFileOpenEvent>
#include <QObject>
#include <QString>

/**
 * @brief Event filter that catches QFileOpenEvent URLs matching a scheme
 *        prefix and re-emits them as urlReceived().
 *
 * On macOS, when the application is registered as a URL scheme handler, the
 * OS delivers incoming URLs via QFileOpenEvent on the QApplication object.
 * Install this filter on QApplication to intercept them:
 *
 * @code
 *   UrlSchemeEventFilter filter(QStringLiteral("cockatrice://"));
 *   QObject::connect(&filter, &UrlSchemeEventFilter::urlReceived,
 *                    &mainWindow, &MainWindow::handleUrl);
 *   app.installEventFilter(&filter);
 * @endcode
 */
class UrlSchemeEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit UrlSchemeEventFilter(const QStringList &schemePrefix, QObject *parent = nullptr)
        : QObject(parent), m_prefixes(schemePrefix)
    {
    }

signals:
    void urlReceived(const QString &url);

public:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            auto *fileEvent = static_cast<QFileOpenEvent *>(event);

            qWarning() << "[MAC][FileOpenEvent] raw url:" << fileEvent->url()
                       << "toString:" << fileEvent->url().toString() << "schemePrefix:" << m_prefixes;

            const QString url = fileEvent->url().toString();

            qWarning() << "[MAC][FileOpenEvent] extracted url:" << url;

            for (auto m_prefix : m_prefixes) {
                if (url.startsWith(m_prefix)) {
                    qWarning() << "[MAC][FileOpenEvent] MATCH prefix → emitting urlReceived";
                    emit urlReceived(url);
                    return true;
                }
            }

            qWarning() << "[MAC][FileOpenEvent] ignored (wrong scheme)";
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QStringList m_prefixes;
};

#endif // COCKATRICE_URL_SCHEME_EVENT_FILTER_H
