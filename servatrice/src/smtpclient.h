#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>

class QxtSmtp;
class QxtMailMessage;

class SmtpClient : public QObject {
	Q_OBJECT
public:
	SmtpClient(QObject *parent = 0);
	~SmtpClient();
protected:
	QxtSmtp *smtp;
public slots:
	bool enqueueActivationTokenMail(const QString &nickname, const QString &recipient, const QString &token, const QString &type = "REG");
	void sendAllEmails();
protected slots:
	void authenticated();
	void authenticationFailed(const QByteArray & msg);
	void connected();
	void connectionFailed(const QByteArray & msg);
	void disconnected();
	void encrypted();
	void encryptionFailed(const QByteArray & msg);
	void finished();
	void mailFailed(int mailID, int errorCode, const QByteArray & msg);
	void mailSent(int mailID);
	void recipientRejected(int mailID, const QString & address, const QByteArray & msg);
	void senderRejected(int mailID, const QString & address, const QByteArray & msg);
};

#endif