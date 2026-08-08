#ifndef COCKATRICE_URL_SCHEME_EVENT_FILTER_H
#define COCKATRICE_URL_SCHEME_EVENT_FILTER_H

#include <QEvent>
#include <QFileOpenEvent>
#include <QObject>
#include <QString>
#include <QUrl>

/**
 * @brief Event filter that catches QFileOpenEvent URLs matching a scheme and
 *        re-emits them as urlReceived().
 *
 * On macOS, when the application is registered as a URL scheme handler, the
 * OS delivers incoming URLs via QFileOpenEvent on the QApplication object.
 * Install this filter on QApplication to intercept them:
 *
 * @code
 *   UrlSchemeEventFilter filter(QStringList{QStringLiteral("cockatrice")});
 *   QObject::connect(&filter, &UrlSchemeEventFilter::urlReceived,
 *                    &mainWindow, &MainWindow::handleUrl);
 *   app.installEventFilter(&filter);
 * @endcode
 *
 * Note: the strings are compared against QUrl::scheme(), so they must be
 * written without the "://" suffix (e.g. "cockatrice", not "cockatrice://").
 */
class UrlSchemeEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit UrlSchemeEventFilter(const QStringList &schemes, QObject *parent = nullptr)
        : QObject(parent), prefixes(schemes)
    {
    }

signals:
    void urlReceived(const QString &url);

public:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            auto *fileEvent = static_cast<QFileOpenEvent *>(event);

            const QUrl url = fileEvent->url();

            for (const auto &prefix : prefixes) {
                if (url.scheme() == prefix) {
                    emit urlReceived(url.toString());
                    return true;
                }
            }

            if (url.isLocalFile()) {
                emit urlReceived(url.toLocalFile());
                return true;
            }
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QStringList prefixes;
};

#endif // COCKATRICE_URL_SCHEME_EVENT_FILTER_H
