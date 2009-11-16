#include <QtGui>

#include "zoneviewwidget.h"
#include "carditem.h"
#include "zoneviewzone.h"
#include "player.h"
#include "client.h"
#include "gamescene.h"

ZoneViewWidget::ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards, QGraphicsItem *parent)
	: QGraphicsWidget(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint/* | Qt::WindowCloseButtonHint*/), player(_player)
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	QFont font;
	font.setPixelSize(8);
	setFont(font);

	QGraphicsLinearLayout *vbox = new QGraphicsLinearLayout(Qt::Vertical);
	setLayout(vbox);
	
	if (numberCards == -1) {
		sortCheckBox = new QCheckBox;
		QGraphicsProxyWidget *sortProxy = new QGraphicsProxyWidget;
		sortProxy->setWidget(sortCheckBox);
		vbox->addItem(sortProxy);
	} else
		sortCheckBox = 0;
	
	if (_origZone->getIsShufflable() && (numberCards == -1)) {
		shuffleCheckBox = new QCheckBox;
		shuffleCheckBox->setChecked(true);
		QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget;
		shuffleProxy->setWidget(shuffleCheckBox);
		vbox->addItem(shuffleProxy);
	} else
		shuffleCheckBox = 0;
	
	qreal left, top, right, bottom;
	getWindowFrameMargins(&left, &top, &right, &bottom);
	qreal h = scene()->sceneRect().height() - (top + bottom);

/*	scrollBar = new QScrollBar(Qt::Vertical);
	QGraphicsProxyWidget *scrollProxy = new QGraphicsProxyWidget(this);
	scrollProxy->setWidget(scrollBar);
	scrollProxy->setPos(138, y);
	scrollProxy->resize(scrollProxy->size().width(), h - y);

	qreal w = 138 + scrollProxy->size().width();
*/qreal w = 138;
	resize(w, h);

	zone = new ZoneViewZone(player, _origZone, numberCards, this);
	connect(zone, SIGNAL(contentsChanged()), this, SLOT(resizeToZoneContents()));
	zone->dumpObjectInfo();
	vbox->addItem(zone);
	zone->initializeCards();
	
	if (sortCheckBox) {
		connect(sortCheckBox, SIGNAL(stateChanged(int)), zone, SLOT(setSortingEnabled(int)));
		QSettings settings;
		sortCheckBox->setChecked(settings.value("zoneview/sorting").toInt());
	}

	retranslateUi();
}

void ZoneViewWidget::retranslateUi()
{
	setWindowTitle(zone->getTranslatedName(false, CaseNominative));
	if (sortCheckBox)
		sortCheckBox->setText(tr("sort alphabetically"));
	if (shuffleCheckBox)
		shuffleCheckBox->setText(tr("shuffle when closing"));
}

void ZoneViewWidget::resizeToZoneContents()
{
	qDebug("+++++++ bla");
	int cardCount = zone->getCards().size();
	const QRectF &playersRect = static_cast<GameScene *>(scene())->getPlayersRect();
	int h = 0;
	if (cardCount * CARD_HEIGHT / 5 < playersRect.height() * 1.5)
		h = cardCount * CARD_HEIGHT / 5;
	else
		h = playersRect.height() * 1.5;
	qDebug(QString("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx resizing to %1").arg(h).toLatin1());
	resize(size().width(), h);
	emit sizeChanged();
}

void ZoneViewWidget::closeEvent(QCloseEvent *event)
{
	player->client->stopDumpZone(player->getId(), zone->getName());
	if (shuffleCheckBox)
		if (shuffleCheckBox->isChecked())
			player->client->shuffle();
	emit closePressed(this);
	deleteLater();
	event->accept();
}
