#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QCheckBox>
#include "zoneviewwidget.h"
#include "carditem.h"
#include "zoneviewzone.h"
#include "player.h"
#include "gamescene.h"
#include "protocol_items.h"
#include "settingscache.h"

ZoneViewWidget::ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards, bool _revealZone, const QList<ServerInfo_Card *> &cardList)
	: QGraphicsWidget(0, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint/* | Qt::WindowCloseButtonHint*/), player(_player)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setZValue(2000000006);
	
	QFont font;
	font.setPixelSize(10);
	setFont(font);

	QGraphicsLinearLayout *vbox = new QGraphicsLinearLayout(Qt::Vertical);
	
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
	setWindowTitle(zone->getTranslatedName(false, CaseNominative));
	if (sortByNameCheckBox)
		sortByNameCheckBox->setText(tr("sort by name"));
	if (sortByTypeCheckBox)
		sortByTypeCheckBox->setText(tr("sort by type"));
	if (shuffleCheckBox)
		shuffleCheckBox->setText(tr("shuffle when closing"));
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
