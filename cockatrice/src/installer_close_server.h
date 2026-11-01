#ifndef COCKATRICE_INSTALLER_CLOSE_SERVER_H
#define COCKATRICE_INSTALLER_CLOSE_SERVER_H

#include <QObject>
#include <QString>

class QLocalServer;

/**
 * @brief Listens for the installer's request to close this instance gracefully.
 *
 * The Windows installer cannot reach an instance whose window is hidden in the notification
 * area with WM_CLOSE, and it must not kill a process that may hold an unsaved deck. It
 * connects to this local socket instead and writes one line; the instance then closes the
 * same way its own close button does - saving settings and answering its own confirmation,
 * never bypassing it.
 *
 * The socket name carries the process id, so every instance has its own and an installer
 * that found several can ask each one. It is built by installerCloseSocketName() in
 * tools/installer-holder-probe/installer_close_name.h; the two must stay identical.
 */
class InstallerCloseServer : public QObject
{
    Q_OBJECT
public:
    explicit InstallerCloseServer(QObject *parent = nullptr);

    /**
     * @brief Starts listening. A failure is not fatal: the installer falls back to
     *        WM_CLOSE, which is what older clients answer.
     */
    bool listen();

    /// The socket name this process listens on. The installer's probe builds the same
    /// string from the process id and the account name.
    static QString socketName(qint64 processId);

    /// The name for an explicit account, exposed so the probe's name helper and this one
    /// can be tested against each other.
    static QString socketNameForUser(qint64 processId, const QString &userName);

signals:
    void closeRequested();

private:
    QLocalServer *server = nullptr;
    QString name;
};

#endif // COCKATRICE_INSTALLER_CLOSE_SERVER_H
