#include "playertarget.h"
#include "player.h"
#include "protocol_datastructures.h"
#include <QPainter>

PlayerTarget::PlayerTarget(Player *_owner)
	: ArrowTarget(_owner, _owner)
{
	font = QFont("Times");
	font.setStyleHint(QFont::Serif);
	font.setPixelSize(20);
}

QRectF PlayerTarget::boundingRect() const
{
	return QRectF(0, 0, 64, 64);
}

void PlayerTarget::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	ServerInfo_User *info = owner->getUserInfo();
	painter->fillRect(boundingRect(), QColor(255, 255, 255, 100));
	painter->setFont(font);
	painter->setPen(Qt::black);
	painter->drawText(boundingRect(), Qt::AlignCenter, info->getName());
}
