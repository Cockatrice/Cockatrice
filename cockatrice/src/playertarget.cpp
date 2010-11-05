#include "playertarget.h"
#include "player.h"
#include "protocol_datastructures.h"
#include "pixmapgenerator.h"
#include <QPainter>
#include <QPixmapCache>
#include <QDebug>

PlayerTarget::PlayerTarget(Player *_owner)
	: ArrowTarget(_owner, _owner)
{
	setCacheMode(DeviceCoordinateCache);

	if (!fullPixmap.loadFromData(_owner->getUserInfo()->getAvatarBmp()))
		fullPixmap = QPixmap();
}

QRectF PlayerTarget::boundingRect() const
{
	return QRectF(0, 0, 100, 64);
}

void PlayerTarget::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	ServerInfo_User *info = owner->getUserInfo();

	painter->save();
	QRectF translatedRect = painter->combinedTransform().mapRect(boundingRect());
	QSize translatedSize = translatedRect.size().toSize();
	QPixmap cachedPixmap;
	const QString cacheKey = "avatar" + QString::number(translatedSize.width()) + "_" + QString::number(info->getUserLevel()) + "_" + QString::number(fullPixmap.cacheKey());
#if QT_VERSION >= 0x040600
	if (!QPixmapCache::find(cacheKey, &cachedPixmap)) {
#else
	if (!QPixmapCache::find(cacheKey, cachedPixmap)) {
#endif
		if (fullPixmap.isNull())
			cachedPixmap = UserLevelPixmapGenerator::generatePixmap(translatedSize.height(), info->getUserLevel());
		else
			cachedPixmap = fullPixmap.scaled(translatedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QPixmapCache::insert(cacheKey, cachedPixmap);
	}
	painter->resetTransform();
	
	painter->translate((translatedSize.width() - cachedPixmap.width()) / 2.0, 0);
	painter->drawPixmap(cachedPixmap.rect(), cachedPixmap, cachedPixmap.rect());
	painter->resetTransform();
	
	QString name = info->getName();
	if (name.size() > 13)
		name = name.mid(0, 10) + "...";
	
	QFont font;
	font.setPixelSize(qMax(translatedSize.height() / 4, 9));
	painter->setFont(font);
	painter->setBackgroundMode(Qt::OpaqueMode);
	painter->setBackground(QColor(0, 0, 0, 100));
	painter->setPen(Qt::white);
	painter->drawText(translatedRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWrapAnywhere, name);
	painter->restore();

	if (getBeingPointedAt())
		painter->fillRect(boundingRect(), QBrush(QColor(255, 0, 0, 100)));
}
