#ifndef LIBCOCKATRICE_URL_SCHEME_EVENT_FILTER_H
#define LIBCOCKATRICE_URL_SCHEME_EVENT_FILTER_H

// Lives in libcockatrice_utility_gui (not libcockatrice_utility) because
// QFileOpenEvent is in Qt::Gui, and libcockatrice_utility is intentionally
// Core+Network-only so servatrice (headless server, transitively consumes
// libcockatrice_utility) does not pull in Qt::Gui.

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
 *
 * Scheme matching is case-insensitive (per RFC 3986).  Matching events are
 * consumed (eventFilter returns true) so they do not propagate to other
 * QFileOpenEvent handlers.  If the app ever handles non-URL file-open events
 * (e.g. .cor file association), make sure those handlers see the events first
 * by installing this filter LAST, or by ensuring the prefix uniquely
 * partitions URL events from file events.
 */
class UrlSchemeEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit UrlSchemeEventFilter(const QString &schemePrefix, QObject *parent = nullptr)
        : QObject(parent), m_prefix(schemePrefix)
    {
    }

signals:
    void urlReceived(const QString &url);

public:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            const QString url = static_cast<QFileOpenEvent *>(event)->url().toString();
            if (url.startsWith(m_prefix, Qt::CaseInsensitive)) {
                emit urlReceived(url);
                return true;
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QString m_prefix;
};

#endif // LIBCOCKATRICE_URL_SCHEME_EVENT_FILTER_H
