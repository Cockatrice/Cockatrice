#ifndef COCKATRICE_SINGLE_INSTANCE_MANAGER_H
#define COCKATRICE_SINGLE_INSTANCE_MANAGER_H

#include <QDataStream>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>

class SingleInstanceManager : public QObject
{
    Q_OBJECT
public:
    explicit SingleInstanceManager(QObject *parent = nullptr);

    // Returns true if this process became the primary instance, false if
    // another instance is already running (and received our files).
    bool tryRun(const QStringList &initialFiles);

signals:
    void filesReceived(const QStringList &files);

private slots:
    void handleNewConnection();

private:
    bool forwardToPrimary(const QStringList &filesToSend);

    QString serverName;
    QLocalServer *server = nullptr;
};

#endif // COCKATRICE_SINGLE_INSTANCE_MANAGER_H
