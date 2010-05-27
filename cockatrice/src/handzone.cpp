#include "handzone.h"
#include "settingscache.h"
#include "player.h"
#include "protocol_items.h"

HandZone::HandZone(Player *_p, bool _contentsKnown, QGraphicsItem *parent)
	: CardZone(_p, "hand", false, false, _contentsKnown, parent)
{
	connect(settingsCache, SIGNAL(handBgPathChanged()), this, SLOT(updateBgPixmap()));
	updateBgPixmap();
}

void HandZone::updateBgPixmap()
{
	QString bgPath = settingsCache->getHandBgPath();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	update();
}

void HandZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	if (x == -1)
		x = cards.size();
	cards.insert(x, card);

	if (!cards.getContentsKnown()) {
		card->setId(-1);
		card->setName();
	}
	card->setParentItem(this);
	card->resetState();
	card->setVisible(true);
	card->update();
}

void HandZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), cardId, getName(), cards.size(), -1, false));
}
