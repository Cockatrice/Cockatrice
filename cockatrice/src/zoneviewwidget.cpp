#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include "zoneviewwidget.h"
#include "carditem.h"
#include "zoneviewzone.h"
#include "player.h"
#include "gamescene.h"
#include "settingscache.h"
#include "gamescene.h"

#include "pb/command_stop_dump_zone.pb.h"
#include "pb/command_shuffle.pb.h"

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
        return fm.size(Qt::TextSingleLine, text) + QSizeF(10, 10);
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

ZoneViewWidget::ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards, bool _revealZone, bool _writeableRevealZone, const QList<const ServerInfo_Card *> &cardList)
    : QGraphicsWidget(0, Qt::Tool | Qt::FramelessWindowHint), player(_player), canBeShuffled(_origZone->getIsShufflable())
{
    setAcceptHoverEvents(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setZValue(2000000006);
    setFlag(ItemIgnoresTransformations);

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
        QGraphicsProxyWidget *sortByNameProxy = new QGraphicsProxyWidget;
        sortByNameProxy->setWidget(&sortByNameCheckBox);
        vbox->addItem(sortByNameProxy);

        QGraphicsProxyWidget *sortByTypeProxy = new QGraphicsProxyWidget;
        sortByTypeProxy->setWidget(&sortByTypeCheckBox);
        vbox->addItem(sortByTypeProxy);

        QGraphicsProxyWidget *lineProxy = new QGraphicsProxyWidget;
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lineProxy->setWidget(line);
        vbox->addItem(lineProxy);

        QGraphicsProxyWidget *pileViewProxy = new QGraphicsProxyWidget;
        pileViewProxy->setWidget(&pileViewCheckBox);
        vbox->addItem(pileViewProxy);
    }

    if (_origZone->getIsShufflable() && (numberCards == -1)) {
        shuffleCheckBox.setChecked(true);
        QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget;
        shuffleProxy->setWidget(&shuffleCheckBox);
        vbox->addItem(shuffleProxy);
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
    scrollBar->setSingleStep(50);
    connect(scrollBar, SIGNAL(valueChanged(int)), this, SLOT(handleScrollBarChange(int)));
    QGraphicsProxyWidget *scrollBarProxy = new QGraphicsProxyWidget;
    scrollBarProxy->setWidget(scrollBar);
    zoneHBox->addItem(scrollBarProxy);

    vbox->addItem(zoneHBox);

    zone = new ZoneViewZone(player, _origZone, numberCards, _revealZone, _writeableRevealZone, zoneContainer);
    connect(zone, SIGNAL(wheelEventReceived(QGraphicsSceneWheelEvent *)), this, SLOT(handleWheelEvent(QGraphicsSceneWheelEvent *)));

    // numberCard is the num of cards we want to reveal from an area. Ex: scry the top 3 cards.
    // If the number is < 0 then it means that we can make the area sorted and we dont care about the order.
    if (numberCards < 0) {
        connect(&sortByNameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(processSortByName(int)));
        connect(&sortByTypeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(processSortByType(int)));
        connect(&pileViewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(processSetPileView(int)));
        sortByNameCheckBox.setChecked(settingsCache->getZoneViewSortByName());
        sortByTypeCheckBox.setChecked(settingsCache->getZoneViewSortByType());
        pileViewCheckBox.setChecked(settingsCache->getZoneViewPileView());
        if (!settingsCache->getZoneViewSortByType())
            pileViewCheckBox.setEnabled(false);
    }

    retranslateUi();
    setLayout(vbox);

    connect(zone, SIGNAL(optimumRectChanged()), this, SLOT(resizeToZoneContents()));
    connect(zone, SIGNAL(beingDeleted()), this, SLOT(zoneDeleted()));
    zone->initializeCards(cardList);
}

void ZoneViewWidget::processSortByType(int value) {
    pileViewCheckBox.setEnabled(value);
    settingsCache->setZoneViewSortByType(value);
    zone->setPileView(pileViewCheckBox.isChecked());
    zone->setSortByType(value);
}

void ZoneViewWidget::processSortByName(int value) {
    settingsCache->setZoneViewSortByName(value);
    zone->setSortByName(value);
}

void ZoneViewWidget::processSetPileView(int value) {
    settingsCache->setZoneViewPileView(value);
    zone->setPileView(value);
}

void ZoneViewWidget::retranslateUi()
{
    titleLabel->setText(zone->getTranslatedName(false, CaseNominative));
    sortByNameCheckBox.setText(tr("sort by name"));
    sortByTypeCheckBox.setText(tr("sort by type"));
    shuffleCheckBox.setText(tr("shuffle when closing"));
    pileViewCheckBox.setText(tr("pile view"));
}

void ZoneViewWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->fillRect(boundingRect(), palette().color(QPalette::Window));
    QGraphicsWidget::paint(painter, option, widget);
}

void ZoneViewWidget::moveWidget(QPointF scenePos)
{
    if(scenePos.x() < 0)
    {
        scenePos.setX(0);
    } else {
        qreal maxw = scene()->sceneRect().width() - 100;
        if(scenePos.x() > maxw)
            scenePos.setX(maxw);
    }

    if(scenePos.y() < 0)
    {
        scenePos.setY(0);
    } else {
        qreal maxh = scene()->sceneRect().height() - 100;
        if(scenePos.y() > maxh)
            scenePos.setY(maxh);
    }

    setPos(scenePos);
}

void ZoneViewWidget::resizeToZoneContents()
{
    QRectF zoneRect = zone->getOptimumRect();
    qreal totalZoneHeight = zoneRect.height();
    if (zoneRect.height() > 500)
        zoneRect.setHeight(500);
    QSizeF newSize(qMax(QGraphicsWidget::layout()->effectiveSizeHint(Qt::MinimumSize, QSizeF()).width(), zoneRect.width() + scrollBar->width() + 10), zoneRect.height() + extraHeight + 10);
    setMaximumSize(newSize);
    resize(newSize);

    zone->setGeometry(QRectF(0, -scrollBar->value(), zoneContainer->size().width(), totalZoneHeight));
    scrollBar->setMaximum(totalZoneHeight - zoneRect.height());

    if (layout())
        layout()->invalidate();
}

void ZoneViewWidget::handleWheelEvent(QGraphicsSceneWheelEvent *event)
{
    QWheelEvent wheelEvent(QPoint(), event->delta(), event->buttons(), event->modifiers(), event->orientation());
    scrollBar->event(&wheelEvent);
}

void ZoneViewWidget::handleScrollBarChange(int value)
{
    zone->setY(-value);
}

void ZoneViewWidget::closeEvent(QCloseEvent *event)
{
    disconnect(zone, SIGNAL(beingDeleted()), this, 0);
    if (zone->getNumberCards() != -2) {
        Command_StopDumpZone cmd;
        cmd.set_player_id(player->getId());
        cmd.set_zone_name(zone->getName().toStdString());
        player->sendGameCommand(cmd);
    }
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
