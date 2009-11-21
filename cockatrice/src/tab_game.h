#ifndef TAB_GAME_H
#define TAB_GAME_H

#include <QWidget>

class Client;
class CardDatabase;
class GameEvent;
class GameView;
class DeckView;
class GameScene;
class Game;
class CardInfoWidget;
class MessageLogWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class ZoneViewLayout;
class ZoneViewWidget;
class PhasesToolbar;

class TabGame : public QWidget {
	Q_OBJECT
private:
	Client *client;
	int gameId;

	CardInfoWidget *cardInfo;
	MessageLogWidget *messageLog;
	QLabel *sayLabel;
	QLineEdit *sayEdit;
	PhasesToolbar *phasesToolbar;
	GameScene *scene;
	GameView *gameView;
	DeckView *deckView;
	Game *game;
	ZoneViewLayout *zoneLayout;
	QAction *aCloseMostRecentZoneView;
private slots:
public:
	TabGame(Client *_client, int _gameId);
	void retranslateUi();
	void processGameEvent(GameEvent *event);
};

#endif
