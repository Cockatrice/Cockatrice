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
 * @param numberCards num of cards to reveal from the zone. Ex: scry the top 3 cards.
 * Pass in a negative number to reveal the entire zone.
 * -1 specifically will give the option to shuffle the zone upon closing the window.
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

    // If the number is < 0, then it means that we can give the option to make the area sorted
    if (numberCards < 0) {
        QGraphicsLinearLayout *hFilterbox = new QGraphicsLinearLayout(Qt::Horizontal);

        // groupBy label
        groupByLabel.setFixedHeight(23);
        groupByLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        groupByLabel.setAlignment(Qt::AlignLeft);

        QGraphicsProxyWidget *groupByLabelProxy = new QGraphicsProxyWidget;
        groupByLabelProxy->setWidget(&groupByLabel);
        groupByLabelProxy->setZValue(2000000008);
        hFilterbox->addItem(groupByLabelProxy);
        hFilterbox->setItemSpacing(0, 0);

        // groupBy options
        groupBySelector.setFixedHeight(23);

        QGraphicsProxyWidget *groupBySelectorProxy = new QGraphicsProxyWidget;
        groupBySelectorProxy->setWidget(&groupBySelector);
        groupBySelectorProxy->setZValue(2000000008);
        groupBySelectorProxy->setMinimumWidth(120);
        hFilterbox->addItem(groupBySelectorProxy);
        hFilterbox->setItemSpacing(1, 1);

        // sortBy label
        sortByLabel.setFixedHeight(23);
        sortByLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sortByLabel.setAlignment(Qt::AlignLeft);

        QGraphicsProxyWidget *sortByLabelProxy = new QGraphicsProxyWidget;
        sortByLabelProxy->setWidget(&sortByLabel);
        sortByLabelProxy->setZValue(2000000007);
        hFilterbox->addItem(sortByLabelProxy);
        hFilterbox->setItemSpacing(2, 0);

        // sortBy options
        sortBySelector.setFixedHeight(23);

        QGraphicsProxyWidget *sortBySelectorProxy = new QGraphicsProxyWidget;
        sortBySelectorProxy->setWidget(&sortBySelector);
        sortBySelectorProxy->setZValue(2000000007);
        sortBySelectorProxy->setMinimumWidth(120);
        hFilterbox->addItem(sortBySelectorProxy);
        hFilterbox->setItemSpacing(3, 1);

        vbox->addItem(hFilterbox);

        // line
        QGraphicsProxyWidget *lineProxy = new QGraphicsProxyWidget;
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lineProxy->setWidget(line);
        vbox->addItem(lineProxy);

        QGraphicsLinearLayout *hPilebox = new QGraphicsLinearLayout(Qt::Horizontal);

        // pile view options
        QGraphicsProxyWidget *pileViewProxy = new QGraphicsProxyWidget;
        pileViewProxy->setWidget(&pileViewCheckBox);
        hPilebox->addItem(pileViewProxy);

        // shuffle options
        if (_origZone->getIsShufflable() && numberCards == -1) {
            shuffleCheckBox.setChecked(true);
            QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget;
            shuffleProxy->setWidget(&shuffleCheckBox);
            hPilebox->addItem(shuffleProxy);
        }

        vbox->addItem(hPilebox);
    }

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

    retranslateUi();

    // only wire up sort options after creating ZoneViewZone, since it segfaults otherwise.
    if (numberCards < 0) {
        connect(&groupBySelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &ZoneViewWidget::processGroupBy);
        connect(&sortBySelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &ZoneViewWidget::processSortBy);
        connect(&pileViewCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &ZoneViewWidget::processSetPileView);
        groupBySelector.setCurrentIndex(SettingsCache::instance().getZoneViewGroupByIndex());
        sortBySelector.setCurrentIndex(SettingsCache::instance().getZoneViewSortByIndex());
        pileViewCheckBox.setChecked(SettingsCache::instance().getZoneViewPileView());

        if (CardList::NoSort == static_cast<CardList::SortOption>(groupBySelector.currentData().toInt())) {
            pileViewCheckBox.setEnabled(false);
        }
    }

    setLayout(vbox);

    connect(zone, SIGNAL(optimumRectChanged()), this, SLOT(resizeToZoneContents()));
    connect(zone, SIGNAL(beingDeleted()), this, SLOT(zoneDeleted()));
    zone->initializeCards(cardList);

    auto *lastResizeBeforeVisibleTimer = new QTimer(this);
    connect(lastResizeBeforeVisibleTimer, &QTimer::timeout, this, [=] {
        resizeToZoneContents();
        disconnect(lastResizeBeforeVisibleTimer);
        lastResizeBeforeVisibleTimer->deleteLater();
    });
    lastResizeBeforeVisibleTimer->setSingleShot(true);
    lastResizeBeforeVisibleTimer->start(1);
}

void ZoneViewWidget::processGroupBy(int index)
{
    auto option = static_cast<CardList::SortOption>(groupBySelector.itemData(index).toInt());
    SettingsCache::instance().setZoneViewGroupByIndex(index);
    zone->setGroupBy(option);

    // disable pile view checkbox if we're not grouping by anything
    pileViewCheckBox.setEnabled(option != CardList::NoSort);

    // reset sortBy if it has the same value as groupBy
    if (option != CardList::NoSort &&
        option == static_cast<CardList::SortOption>(sortBySelector.currentData().toInt())) {
        sortBySelector.setCurrentIndex(1); // set to SortByName
    }
}

void ZoneViewWidget::processSortBy(int index)
{
    auto option = static_cast<CardList::SortOption>(sortBySelector.itemData(index).toInt());

    // set to SortByName instead if it has the same value as groupBy
    if (option != CardList::NoSort &&
        option == static_cast<CardList::SortOption>(groupBySelector.currentData().toInt())) {
        sortBySelector.setCurrentIndex(1); // set to SortByName
        return;
    }

    SettingsCache::instance().setZoneViewSortByIndex(index);
    zone->setSortBy(option);
}

void ZoneViewWidget::processSetPileView(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setZoneViewPileView(value);
    zone->setPileView(value);
}

void ZoneViewWidget::retranslateUi()
{
    setWindowTitle(zone->getTranslatedName(false, CaseNominative));

    groupByLabel.setText(tr("Group by"));
    sortByLabel.setText(tr("Sort by"));

    { // We can't change the strings after they're put into the QComboBox, so this is our workaround
        int oldIndex = groupBySelector.currentIndex();
        groupBySelector.clear();
        groupBySelector.addItem(tr("---"), CardList::NoSort);
        groupBySelector.addItem(tr("Type"), CardList::SortByType);
        groupBySelector.addItem(tr("Mana Value"), CardList::SortByManaValue);
        groupBySelector.setCurrentIndex(oldIndex);
    }

    {
        int oldIndex = sortBySelector.currentIndex();
        sortBySelector.clear();
        sortBySelector.addItem(tr("---"), CardList::NoSort);
        sortBySelector.addItem(tr("Name"), CardList::SortByName);
        sortBySelector.addItem(tr("Type"), CardList::SortByType);
        sortBySelector.addItem(tr("Mana Value"), CardList::SortByManaValue);
        sortBySelector.setCurrentIndex(oldIndex);
    }

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
