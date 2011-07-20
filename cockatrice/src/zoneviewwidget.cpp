#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPalette>
#include "zoneviewwidget.h"
#include "carditem.h"
#include "zoneviewzone.h"
#include "player.h"
#include "gamescene.h"
#include "protocol_items.h"
#include "settingscache.h"
#include "gamescene.h"

TitleLabel::TitleLabel()
	: QGraphicsWidget(), text(" ")
{
	setAcceptHoverEvents(true);
}

void TitleLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	QBrush windowBrush = palette().window();
	windowBrush.setColor(windowBrush.color().darker(150));
	painter->fillRect(boundingRect(), windowBrush);
	painter->drawText(boundingRect(), Qt::AlignLeft | Qt::AlignVCenter, text);
}

QSizeF TitleLabel::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
	QFont f;
	QFontMetrics fm(f);
	if (which == Qt::MaximumSize)
		return QSizeF(constraint.width(), fm.size(Qt::TextSingleLine, text).height() + 10);
	else
		return fm.size(Qt::TextSingleLine, text);
}

void TitleLabel::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		buttonDownPos = static_cast<GameScene *>(scene())->getViewTransform().inverted().map(event->pos());
		event->accept();
	} else
		event->ignore();
}

void TitleLabel::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	emit mouseMoved(event->scenePos() - buttonDownPos);
}

ZoneViewWidget::ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards, bool _revealZone, const QList<ServerInfo_Card *> &cardList)
	: QGraphicsWidget(0, Qt::Tool | Qt::FramelessWindowHint), player(_player)
{
	setAcceptHoverEvents(true);
	setAttribute(Qt::WA_DeleteOnClose);
	setZValue(2000000006);
	setFlag(ItemIgnoresTransformations);
	setAutoFillBackground(true);
	
	QGraphicsLinearLayout *hbox = new QGraphicsLinearLayout(Qt::Horizontal);
	titleLabel = new TitleLabel;
	connect(titleLabel, SIGNAL(mouseMoved(QPointF)), this, SLOT(moveWidget(QPointF)));
	closeButton = new QPushButton("X");
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QGraphicsProxyWidget *closeButtonProxy = new QGraphicsProxyWidget;
	closeButtonProxy->setWidget(closeButton);
	
	hbox->addItem(titleLabel);
	hbox->addItem(closeButtonProxy);
	QGraphicsLinearLayout *vbox = new QGraphicsLinearLayout(Qt::Vertical);
	
	vbox->addItem(hbox);
	
	if (numberCards < 0) {
		sortByNameCheckBox = new QCheckBox;
		QGraphicsProxyWidget *sortByNameProxy = new QGraphicsProxyWidget;
		sortByNameProxy->setWidget(sortByNameCheckBox);
		vbox->addItem(sortByNameProxy);

		sortByTypeCheckBox = new QCheckBox;
		QGraphicsProxyWidget *sortByTypeProxy = new QGraphicsProxyWidget;
		sortByTypeProxy->setWidget(sortByTypeCheckBox);
		vbox->addItem(sortByTypeProxy);
	} else {
		sortByNameCheckBox = 0;
		sortByTypeCheckBox = 0;
	}
	
	if (_origZone->getIsShufflable() && (numberCards == -1)) {
		shuffleCheckBox = new QCheckBox;
		shuffleCheckBox->setChecked(true);
		QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget;
		shuffleProxy->setWidget(shuffleCheckBox);
		vbox->addItem(shuffleProxy);
	} else
		shuffleCheckBox = 0;
	
	extraHeight = vbox->sizeHint(Qt::PreferredSize).height();
	resize(150, 150);

	zone = new ZoneViewZone(player, _origZone, numberCards, _revealZone, this);
	connect(zone, SIGNAL(optimumRectChanged()), this, SLOT(resizeToZoneContents()));
	connect(zone, SIGNAL(beingDeleted()), this, SLOT(zoneDeleted()));
	vbox->addItem(zone);
	zone->initializeCards(cardList);
	
	if (sortByNameCheckBox) {
		connect(sortByNameCheckBox, SIGNAL(stateChanged(int)), zone, SLOT(setSortByName(int)));
		connect(sortByTypeCheckBox, SIGNAL(stateChanged(int)), zone, SLOT(setSortByType(int)));
		sortByNameCheckBox->setChecked(settingsCache->getZoneViewSortByName());
		sortByTypeCheckBox->setChecked(settingsCache->getZoneViewSortByType());
	}

	setLayout(vbox);
	retranslateUi();
}

void ZoneViewWidget::retranslateUi()
{
	titleLabel->setText(zone->getTranslatedName(false, CaseNominative));
	if (sortByNameCheckBox)
		sortByNameCheckBox->setText(tr("sort by name"));
	if (sortByTypeCheckBox)
		sortByTypeCheckBox->setText(tr("sort by type"));
	if (shuffleCheckBox)
		shuffleCheckBox->setText(tr("shuffle when closing"));
}

void ZoneViewWidget::moveWidget(QPointF scenePos)
{
	setPos(scenePos);
}

void ZoneViewWidget::resizeToZoneContents()
{
	QRectF zoneRect = zone->getOptimumRect();
	QSizeF newSize(zoneRect.width() + 10, zoneRect.height() + extraHeight + 10);
	setMaximumSize(newSize);
	resize(newSize);
	if (layout())
		layout()->invalidate();
}

void ZoneViewWidget::closeEvent(QCloseEvent *event)
{
	disconnect(zone, SIGNAL(beingDeleted()), this, 0);
	if (zone->getNumberCards() != -2)
		player->sendGameCommand(new Command_StopDumpZone(-1, player->getId(), zone->getName()));
	if (shuffleCheckBox)
		if (shuffleCheckBox->isChecked())
			player->sendGameCommand(new Command_Shuffle);
	emit closePressed(this);
	deleteLater();
	event->accept();
}

void ZoneViewWidget::zoneDeleted()
{
	emit closePressed(this);
	deleteLater();
}
