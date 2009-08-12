#include <QTimer>
#include "client.h"

PendingCommand::PendingCommand(const QString &_cmd, int _msgid, QObject *parent)
	: QObject(parent), cmd(_cmd), msgid(_msgid), time(0)
{
}

void PendingCommand::responseReceived(int _msgid, ServerResponse _resp)
{
	if (_msgid == msgid) {
		emit finished(_resp);
		deleteLater();
	}
}

void PendingCommand::checkTimeout()
{
	if (++time > 5)
		emit timeout();
}

Client::Client(QObject *parent)
	: QObject(parent), status(StatusDisconnected), MsgId(0)
{
	timer = new QTimer(this);
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(ping()));

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

void Client::timeout()
{
	emit serverTimeout();
	disconnectFromServer();
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

void Client::removePendingCommand()
{
	PendingCommands.removeAt(PendingCommands.indexOf(static_cast<PendingCommand *>(sender())));
}

void Client::loginResponse(ServerResponse response)
{
	if (response == RespOk)
		setStatus(StatusIdle);
	else {
		emit serverError(response);
		disconnectFromServer();
	}
}

void Client::enterGameResponse(ServerResponse response)
{
	if (response == RespOk)
		setStatus(StatusPlaying);
}

void Client::leaveGameResponse(ServerResponse response)
{
	if (response == RespOk)
		setStatus(StatusIdle);
}

void Client::readLine()
{
	while (socket->canReadLine()) {
		QString line = QString(socket->readLine()).trimmed();

		if (line.isNull())
			break;
		qDebug(QString("<< %1").arg(line).toLatin1());
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
			if (values.size() != 2) {
				// XXX
			}
			bool ok;
			int msgid = values.takeFirst().toInt(&ok);
			if (!ok) {
				// XXX
			}

			ServerResponse resp;
			if (values[0] == "ok")
				resp = RespOk;
			else if (values[0] == "password")
				resp = RespPassword;
			else
				resp = RespErr;
			emit responseReceived(msgid, resp);
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
					if (prefix == "list_players")
						playerlist << new ServerPlayer(val[0].toInt(), val[1]);
					else if (prefix == "list_counters")
					{ }
					else if (prefix == "list_zones")
						zonelist << new ServerZone(val[0], val[1] == "1", val[2] == "1", val[3].toInt());
					else if (prefix == "dump_zone")
						zonedump << new ServerZoneCard(val[0].toInt(), val[1], val[2].toInt(), val[3].toInt(), val[4].toInt(), val[5] == "1", val[6]);
					else if (prefix == "welcome")
						welcomemsg << val[0];
				}
				if (prefix == "list_players")
					emit playerListReceived(playerlist);
				else if (prefix == "list_counters")
				{ }
				else if (prefix == "list_zones")
					emit zoneListReceived(cmdid, zonelist);
				else if (prefix == "dump_zone")
					emit zoneDumpReceived(cmdid, zonedump);
				else if (prefix == "welcome") {
					emit welcomeMsgReceived(welcomemsg);
					setStatus(StatusLoggingIn);
					login(playerName, password);
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
	qDebug(QString(">> %1").arg(s).toLatin1());
	QTextStream stream(socket);
	stream.setCodec("UTF-8");
	stream << s << endl;
	stream.flush();
}

PendingCommand *Client::cmd(const QString &s)
{
	msg(QString("%1|%2").arg(++MsgId).arg(s));
	PendingCommand *pc = new PendingCommand(s, MsgId, this);
	PendingCommands << pc;
	connect(this, SIGNAL(responseReceived(int, ServerResponse)), pc, SLOT(responseReceived(int, ServerResponse)));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(removePendingCommand()));
	connect(pc, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(timer, SIGNAL(timeout()), pc, SLOT(checkTimeout()));
	return pc;
}

void Client::connectToServer(const QString &hostname, unsigned int port, const QString &_playerName, const QString &_password)
{
	disconnectFromServer();
	
	playerName = _playerName;
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

void Client::ping()
{
	cmd("ping");
}

PendingCommand *Client::listGames()
{
	return cmd("list_games");
}

PendingCommand *Client::listPlayers()
{
	return cmd("list_players");
}

PendingCommand *Client::createGame(const QString &description, const QString &password, unsigned int maxPlayers)
{
	PendingCommand *pc = cmd(QString("create_game|%1|%2|%3").arg(description).arg(password).arg(maxPlayers));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(enterGameResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::joinGame(int gameId, const QString &password)
{
	PendingCommand *pc = cmd(QString("join_game|%1|%2").arg(gameId).arg(password));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(enterGameResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::leaveGame()
{
	PendingCommand *pc = cmd("leave_game");
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(leaveGameResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::login(const QString &name, const QString &pass)
{
	PendingCommand *pc = cmd(QString("login|%1|%2").arg(name).arg(pass));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(loginResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::say(const QString &s)
{
	return cmd(QString("say|%1").arg(s));
}

PendingCommand *Client::shuffle()
{
	return cmd("shuffle");
}

PendingCommand *Client::rollDice(unsigned int sides)
{
	return cmd(QString("roll_dice|%1").arg(sides));
}

PendingCommand *Client::drawCards(unsigned int number)
{
	return cmd(QString("draw_cards|%1").arg(number));
}

PendingCommand *Client::moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y, bool faceDown)
{
	// if startzone is public: cardid is the card's id
	// else: cardid is the position of the card in the zone (e.g. deck)
	return cmd(QString("move_card|%1|%2|%3|%4|%5|%6").arg(cardid).arg(startzone).arg(targetzone).arg(x).arg(y).arg(faceDown ? 1 : 0));
}

PendingCommand *Client::createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y)
{
	return cmd(QString("create_token|%1|%2|%3|%4|%5").arg(zone).arg(name).arg(powtough).arg(x).arg(y));
}

PendingCommand *Client::setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue)
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

PendingCommand *Client::readyStart()
{
	return cmd("ready_start");
}

PendingCommand *Client::incCounter(const QString &counter, int delta)
{
	return cmd(QString("inc_counter|%1|%2").arg(counter).arg(delta));
}

PendingCommand *Client::addCounter(const QString &counter, QColor color, int value)
{
	return cmd(QString("add_counter|%1|%2|%3").arg(counter).arg(color.red() * 65536 + color.green() * 256 + color.blue()).arg(value));
}

PendingCommand *Client::setCounter(const QString &counter, int value)
{
	return cmd(QString("set_counter|%1|%2").arg(counter).arg(value));
}

PendingCommand *Client::delCounter(const QString &counter)
{
	return cmd(QString("del_counter|%1").arg(counter));
}

PendingCommand *Client::setActivePlayer(int player)
{
	return cmd(QString("set_active_player|%1").arg(player));
}

PendingCommand *Client::setActivePhase(int phase)
{
	return cmd(QString("set_active_phase|%1").arg(phase));
}

PendingCommand *Client::dumpZone(int player, const QString &zone, int numberCards)
{
	return cmd(QString("dump_zone|%1|%2|%3").arg(player).arg(zone).arg(numberCards));
}

PendingCommand *Client::stopDumpZone(int player, const QString &zone)
{
	return cmd(QString("stop_dump_zone|%1|%2").arg(player).arg(zone));
}
