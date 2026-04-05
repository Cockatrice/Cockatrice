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

    bool tryRun(const QStringList &initialFiles);

signals:
    void filesReceived(const QStringList &files);

private slots:
    void handleNewConnection();

private:
    QString serverName;
    QLocalServer *server = nullptr;
};

#endif // COCKATRICE_SINGLE_INSTANCE_MANAGER_H