#include "view_zone_widget.h"

#include "../../settings/cache_settings.h"
#include "../cards/card_item.h"
#include "../game_scene.h"
#include "../player/player.h"
#include "pb/command_shuffle.pb.h"
#include "view_zone.h"

#include <QCheckBox>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <QStyleOptionTitleBar>

/**
 * @param _player the player the cards were revealed to.
 * @param _origZone the zone the cards were revealed from.
 * @param numberCards num of cards to reveal from the zone. Ex: scry the top 3 cards. Use -1 to reveal the entire zone.
 * @param _revealZone if false, the cards will be face down.
 * @param _writeableRevealZone whether the player can interact with the revealed cards.
 */
ZoneViewWidget::ZoneViewWidget(Player *_player,
                               CardZone *_origZone,
                               int numberCards,
                               bool _revealZone,
                               bool _writeableRevealZone,
                               const QList<const ServerInfo_Card *> &cardList)
    : QGraphicsWidget(0, Qt::Window), canBeShuffled(_origZone->getIsShufflable()), player(_player)
{
    setAcceptHoverEvents(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setZValue(2000000006);
    setFlag(ItemIgnoresTransformations);

    QGraphicsLinearLayout *vbox = new QGraphicsLinearLayout(Qt::Vertical);
    QGraphicsLinearLayout *hPilebox = 0;

    if (numberCards < 0) {
        hPilebox = new QGraphicsLinearLayout(Qt::Horizontal);
        QGraphicsLinearLayout *hFilterbox = new QGraphicsLinearLayout(Qt::Horizontal);

        QGraphicsProxyWidget *sortByNameProxy = new QGraphicsProxyWidget;
        sortByNameProxy->setWidget(&sortByNameCheckBox);
        hFilterbox->addItem(sortByNameProxy);

        QGraphicsProxyWidget *sortByTypeProxy = new QGraphicsProxyWidget;
        sortByTypeProxy->setWidget(&sortByTypeCheckBox);
        hFilterbox->addItem(sortByTypeProxy);

        vbox->addItem(hFilterbox);

        QGraphicsProxyWidget *lineProxy = new QGraphicsProxyWidget;
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lineProxy->setWidget(line);
        vbox->addItem(lineProxy);

        QGraphicsProxyWidget *pileViewProxy = new QGraphicsProxyWidget;
        pileViewProxy->setWidget(&pileViewCheckBox);
        hPilebox->addItem(pileViewProxy);
    }

    if (_origZone->getIsShufflable() && (numberCards == -1)) {
        shuffleCheckBox.setChecked(true);
        QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget;
        shuffleProxy->setWidget(&shuffleCheckBox);
        hPilebox->addItem(shuffleProxy);
    }

    vbox->addItem(hPilebox);

    extraHeight = vbox->sizeHint(Qt::PreferredSize).height();
    resize(150, 150);

    QGraphicsLinearLayout *zoneHBox = new QGraphicsLinearLayout(Qt::Horizontal);

    zoneContainer = new QGraphicsWidget(this);
    zoneContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    zoneContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    zoneHBox->addItem(zoneContainer);

    scrollBar = new QScrollBar(Qt::Vertical);
    scrollBar->setMinimum(0);
    scrollBar->setSingleStep(20);
    scrollBar->setPageStep(200);
    connect(scrollBar, SIGNAL(valueChanged(int)), this, SLOT(handleScrollBarChange(int)));
    scrollBarProxy = new ScrollableGraphicsProxyWidget;
    scrollBarProxy->setWidget(scrollBar);
    zoneHBox->addItem(scrollBarProxy);

    vbox->addItem(zoneHBox);

    zone = new ZoneViewZone(player, _origZone, numberCards, _revealZone, _writeableRevealZone, zoneContainer);
    connect(zone, SIGNAL(wheelEventReceived(QGraphicsSceneWheelEvent *)), scrollBarProxy,
            SLOT(recieveWheelEvent(QGraphicsSceneWheelEvent *)));

    // numberCard is the num of cards we want to reveal from an area. Ex: scry the top 3 cards.
    // If the number is < 0 then it means that we can make the area sorted and we dont care about the order.
    if (numberCards < 0) {
        connect(&sortByNameCheckBox, SIGNAL(QT_STATE_CHANGED(QT_STATE_CHANGED_T)), this,
                SLOT(processSortByName(QT_STATE_CHANGED_T)));
        connect(&sortByTypeCheckBox, SIGNAL(QT_STATE_CHANGED(QT_STATE_CHANGED_T)), this,
                SLOT(processSortByType(QT_STATE_CHANGED_T)));
        connect(&pileViewCheckBox, SIGNAL(QT_STATE_CHANGED(QT_STATE_CHANGED_T)), this,
                SLOT(processSetPileView(QT_STATE_CHANGED_T)));
        sortByNameCheckBox.setChecked(SettingsCache::instance().getZoneViewSortByName());
        sortByTypeCheckBox.setChecked(SettingsCache::instance().getZoneViewSortByType());
        pileViewCheckBox.setChecked(SettingsCache::instance().getZoneViewPileView());
        if (!SettingsCache::instance().getZoneViewSortByType())
            pileViewCheckBox.setEnabled(false);
    }

    retranslateUi();
    setLayout(vbox);

    connect(zone, SIGNAL(optimumRectChanged()), this, SLOT(resizeToZoneContents()));
    connect(zone, SIGNAL(beingDeleted()), this, SLOT(zoneDeleted()));
    zone->initializeCards(cardList);
}

void ZoneViewWidget::processSortByType(QT_STATE_CHANGED_T value)
{
    pileViewCheckBox.setEnabled(value);
    SettingsCache::instance().setZoneViewSortByType(value);
    zone->setPileView(pileViewCheckBox.isChecked());
    zone->setSortByType(value);
}

void ZoneViewWidget::processSortByName(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setZoneViewSortByName(value);
    zone->setSortByName(value);
}

void ZoneViewWidget::processSetPileView(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setZoneViewPileView(value);
    zone->setPileView(value);
}

void ZoneViewWidget::retranslateUi()
{
    setWindowTitle(zone->getTranslatedName(false, CaseNominative));
    sortByNameCheckBox.setText(tr("sort by name"));
    sortByTypeCheckBox.setText(tr("sort by type"));
    shuffleCheckBox.setText(tr("shuffle when closing"));
    pileViewCheckBox.setText(tr("pile view"));
}

void ZoneViewWidget::moveEvent(QGraphicsSceneMoveEvent * /* event */)
{
    if (!scene())
        return;

    int titleBarHeight = 24;

    QPointF scenePos = pos();

    if (scenePos.x() < 0) {
        scenePos.setX(0);
    } else {
        qreal maxw = scene()->sceneRect().width() - 100;
        if (scenePos.x() > maxw)
            scenePos.setX(maxw);
    }

    if (scenePos.y() < titleBarHeight) {
        scenePos.setY(titleBarHeight);
    } else {
        qreal maxh = scene()->sceneRect().height() - titleBarHeight;
        if (scenePos.y() > maxh)
            scenePos.setY(maxh);
    }

    if (scenePos != pos())
        setPos(scenePos);
}

void ZoneViewWidget::resizeToZoneContents()
{
    QRectF zoneRect = zone->getOptimumRect();
    qreal totalZoneHeight = zoneRect.height();
    if (zoneRect.height() > 500)
        zoneRect.setHeight(500);
    QSizeF newSize(qMax(QGraphicsWidget::layout()->effectiveSizeHint(Qt::MinimumSize, QSizeF()).width(),
                        zoneRect.width() + scrollBar->width() + 10),
                   zoneRect.height() + extraHeight + 10);
    setMaximumSize(newSize);
    resize(newSize);

    zone->setGeometry(QRectF(0, -scrollBar->value(), zoneContainer->size().width(), totalZoneHeight));
    scrollBar->setMaximum(totalZoneHeight - zoneRect.height());

    if (layout())
        layout()->invalidate();
}

void ZoneViewWidget::handleScrollBarChange(int value)
{
    zone->setY(-value);
}

void ZoneViewWidget::closeEvent(QCloseEvent *event)
{
    disconnect(zone, SIGNAL(beingDeleted()), this, 0);
    if (shuffleCheckBox.isChecked())
        player->sendGameCommand(Command_Shuffle());
    emit closePressed(this);
    deleteLater();
    event->accept();
}

void ZoneViewWidget::zoneDeleted()
{
    emit closePressed(this);
    deleteLater();
}

void ZoneViewWidget::initStyleOption(QStyleOption *option) const
{
    QStyleOptionTitleBar *titleBar = qstyleoption_cast<QStyleOptionTitleBar *>(option);
    if (titleBar)
        titleBar->icon = QPixmap("theme:cockatrice");
}
