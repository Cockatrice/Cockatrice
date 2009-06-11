#include <QTimer>
#include "client.h"

Client::Client(QObject *parent)
	: QObject(parent), status(StatusDisconnected), MsgId(0)
{
	timer = new QTimer(this);
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkTimeout()));

	socket = new QTcpSocket(this);
	socket->setTextModeEnabled(true);
	connect(socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(readLine()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
}

Client::~Client()
{
	disconnectFromServer();
}

void Client::checkTimeout()
{
	bool timeout = false;
	QListIterator<PendingCommand *> i(PendingCommands);
	while (i.hasNext()) {
		PendingCommand *c = i.next();
		if (c->timeout()) {
			timeout = true;
			break;
		}
	}
	if (timeout) {
		disconnectFromServer();
		emit serverTimeout();
	} else
		ping();
}

void Client::slotSocketError(QAbstractSocket::SocketError /*error*/)
{
	emit logSocketError(socket->errorString());
	disconnectFromServer();
}

void Client::slotConnected()
{
	timer->start();
	setStatus(StatusAwaitingWelcome);
}

void Client::readLine()
{
	while (socket->canReadLine()) {
		QString line = QString(socket->readLine()).trimmed();

		if (line.isNull())
			break;
		qDebug(QString("readLine: %1").arg(line).toLatin1());
		QStringList values = line.split("|");
		QString prefix = values.takeFirst();
		// prefix is one of {welcome, private, public, resp, list_games, list_players, list_counters, list_zones, dump_zone}
		if ((prefix == "private") || (prefix == "public")) {
			ServerEventData event(line);
			if (event.getEventType() == eventPlayerId) {
				QStringList data = event.getEventData();
				if (data.size() != 2) {
					// XXX
				}
				bool ok;
				int id = data[0].toInt(&ok);
				if (!ok) {
					// XXX
				}
				emit playerIdReceived(id, data[1]);
			} else
				emit gameEvent(event);
		} else if (prefix == "resp") {
			bool ok;
			int msgid = values.takeFirst().toInt(&ok);
			if (!ok) {
				// XXX
			}

			if (values.empty()) {
				// XXX
			}

			ok = !values.takeFirst().compare("ok");

			// XXX
			ServerErrorMessage message = msgNone;

			// Update list of pending commands
			QListIterator<PendingCommand *> i(PendingCommands);
			bool found = false;
			PendingCommand *c;
			while (i.hasNext()) {
				c = i.next();
				if (c->getMsgId() == msgid) {
					found = true;
					break;
				}
			}
			if (found) {
				PendingCommands.removeAt(PendingCommands.indexOf(c));
				delete c;
			} else
				qDebug(QString("msgid unknown: %1").arg(msgid).toLatin1());

			emit responseReceived(new ServerResponse(msgid, ok, message));
		} else if (prefix == "list_games") {
			emit gameListEvent(new ServerGame(values[0].toInt(), values[5], values[1], values[2].toInt(), values[3].toInt(), values[4].toInt()));
		} else if ((prefix == "list_players") || (prefix == "list_counters") || (prefix == "list_zones") || (prefix == "dump_zone") || (prefix == "welcome")) {
			int cmdid = values.takeFirst().toInt();
			if (values[0] == ".") {
				QListIterator<QStringList> i(msgbuf);
				QList<ServerPlayer *> playerlist;
				QList<ServerZone *> zonelist;
				QList<ServerZoneCard *> zonedump;
				QStringList welcomemsg;
				while (i.hasNext()) {
					QStringList val = i.next();

					// XXX Parametergültigkeit überprüfen
					if (!prefix.compare("list_players"))
						playerlist << new ServerPlayer(val[0].toInt(), val[1]);
					else if (!prefix.compare("list_counters"))
					{ }
					else if (!prefix.compare("list_zones"))
						zonelist << new ServerZone(val[0], val[1] == "1", val[2] == "1", val[3].toInt());
					else if (!prefix.compare("dump_zone"))
						zonedump << new ServerZoneCard(val[0].toInt(), val[1], val[2].toInt(), val[3].toInt(), val[4].toInt(), val[5] == "1", val[6]);
					else if (!prefix.compare("welcome"))
						welcomemsg << val[0];
				}
				if (!prefix.compare("list_players"))
					emit playerListReceived(playerlist);
				else if (!prefix.compare("list_counters"))
				{ }
				else if (!prefix.compare("list_zones"))
					emit zoneListReceived(cmdid, zonelist);
				else if (!prefix.compare("dump_zone"))
					emit zoneDumpReceived(cmdid, zonedump);
				else if (!prefix.compare("welcome")) {
					emit welcomeMsgReceived(welcomemsg);
					setStatus(StatusConnected);
					login(PlayerName, password);
				}
				msgbuf.clear();
			} else
				msgbuf << values;
		} else {
			// XXX
		}
	}
}

void Client::setStatus(const ProtocolStatus _status)
{
	status = _status;
	emit statusChanged(_status);
}

void Client::msg(const QString &s)
{
	qDebug(QString("msg gibt aus: %1").arg(s).toLatin1());
	QTextStream stream(socket);
	stream.setCodec("UTF-8");
	stream << s << endl;
	stream.flush();
}

int Client::cmd(const QString &s)
{
	msg(QString("%1|%2").arg(++MsgId).arg(s));
	PendingCommands << new PendingCommand(s, MsgId);
	return MsgId;
}

void Client::connectToServer(const QString &hostname, unsigned int port, const QString &playername, const QString &_password)
{
	PlayerName = playername;
	password = _password;
	socket->connectToHost(hostname, port);
	setStatus(StatusConnecting);
}

void Client::disconnectFromServer()
{
	timer->stop();

	for (int i = 0; i < PendingCommands.size(); i++)
		delete PendingCommands[i];
	PendingCommands.clear();

	setStatus(StatusDisconnected);
	socket->close();
}

int Client::ping()
{
	return cmd("ping");
}

int Client::listGames()
{
	return cmd("list_games");
}

int Client::listPlayers()
{
	return cmd("list_players");
}

int Client::createGame(const QString &description, const QString &password, unsigned int maxPlayers)
{
	return cmd(QString("create_game|%1|%2|%3").arg(description).arg(password).arg(maxPlayers));
}

int Client::joinGame(int gameId, const QString &password)
{
	return cmd(QString("join_game|%1|%2").arg(gameId).arg(password));
}

int Client::leaveGame()
{
	return cmd("leave_game");
}

int Client::login(const QString &name, const QString &pass)
{
	return cmd(QString("login|%1|%2").arg(name).arg(pass));
}

int Client::say(const QString &s)
{
	return cmd(QString("say|%1").arg(s));
}

int Client::shuffle()
{
	return cmd("shuffle");
}

int Client::rollDice(unsigned int sides)
{
	return cmd(QString("roll_dice|%1").arg(sides));
}

int Client::drawCards(unsigned int number)
{
	return cmd(QString("draw_cards|%1").arg(number));
}

int Client::moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y, bool faceDown)
{
	// if startzone is public: cardid is the card's id
	// else: cardid is the position of the card in the zone (e.g. deck)
	return cmd(QString("move_card|%1|%2|%3|%4|%5|%6").arg(cardid).arg(startzone).arg(targetzone).arg(x).arg(y).arg(faceDown ? 1 : 0));
}

int Client::createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y)
{
	return cmd(QString("create_token|%1|%2|%3|%4|%5").arg(zone).arg(name).arg(powtough).arg(x).arg(y));
}

int Client::setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue)
{
	return cmd(QString("set_card_attr|%1|%2|%3|%4").arg(zone).arg(cardid).arg(aname).arg(avalue));
}

void Client::submitDeck(const QStringList &deck)
{
	cmd("submit_deck");
	QStringListIterator i(deck);
	while (i.hasNext())
		msg(i.next());
	msg(".");
}

int Client::readyStart()
{
	return cmd("ready_start");
}

int Client::incCounter(const QString &counter, int delta)
{
	return cmd(QString("inc_counter|%1|%2").arg(counter).arg(delta));
}

int Client::addCounter(const QString &counter, QColor color, int value)
{
	return cmd(QString("add_counter|%1|%2|%3").arg(counter).arg(color.red() * 65536 + color.green() * 256 + color.blue()).arg(value));
}

int Client::setCounter(const QString &counter, int value)
{
	return cmd(QString("set_counter|%1|%2").arg(counter).arg(value));
}

int Client::delCounter(const QString &counter)
{
	return cmd(QString("del_counter|%1").arg(counter));
}

int Client::setActivePlayer(int player)
{
	return cmd(QString("set_active_player|%1").arg(player));
}

int Client::setActivePhase(int phase)
{
	return cmd(QString("set_active_phase|%1").arg(phase));
}

int Client::dumpZone(int player, const QString &zone, int numberCards)
{
	return cmd(QString("dump_zone|%1|%2|%3").arg(player).arg(zone).arg(numberCards));
}
