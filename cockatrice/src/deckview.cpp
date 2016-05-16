#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "deckview.h"
#include "decklist.h"
#include "carddatabase.h"
#include "settingscache.h"
#include "thememanager.h"
#include "main.h"

DeckViewCardDragItem::DeckViewCardDragItem(DeckViewCard *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag)
    : AbstractCardDragItem(_item, _hotSpot, parentDrag), currentZone(0)
{
}

void DeckViewCardDragItem::updatePosition(const QPointF &cursorScenePos)
{
    QList<QGraphicsItem *> colliding = scene()->items(cursorScenePos);

    DeckViewCardContainer *cursorZone = 0;
    for (int i = colliding.size() - 1; i >= 0; i--)
        if ((cursorZone = qgraphicsitem_cast<DeckViewCardContainer *>(colliding.at(i))))
            break;
    if (!cursorZone)
        return;
    currentZone = cursorZone;
    
    QPointF newPos = cursorScenePos;
    if (newPos != pos()) {
        for (int i = 0; i < childDrags.size(); i++)
            childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
        setPos(newPos);
    }
}

void DeckViewCardDragItem::handleDrop(DeckViewCardContainer *target)
{
    DeckViewCard *card = static_cast<DeckViewCard *>(item);
    DeckViewCardContainer *start = static_cast<DeckViewCardContainer *>(item->parentItem());
    start->removeCard(card);
    target->addCard(card);
    card->setParentItem(target);
}

void DeckViewCardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    DeckViewScene *sc = static_cast<DeckViewScene *>(scene());
    sc->removeItem(this);

    if (currentZone) {
        handleDrop(currentZone);
        for (int i = 0; i < childDrags.size(); i++) {
            DeckViewCardDragItem *c = static_cast<DeckViewCardDragItem *>(childDrags[i]);
            c->handleDrop(currentZone);
            sc->removeItem(c);
        }
        
        sc->updateContents();
    }
    
    event->accept();
}

DeckViewCard::DeckViewCard(const QString &_name, const QString &_originZone, QGraphicsItem *parent)
    : AbstractCardItem(_name, 0, -1, parent), originZone(_originZone), dragItem(0)
{
    setAcceptHoverEvents(true);
}

DeckViewCard::~DeckViewCard()
{
    delete dragItem;
}

void DeckViewCard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractCardItem::paint(painter, option, widget);
    
    painter->save();
    QPen pen;
    pen.setWidth(3);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setColor(originZone == "main" ? Qt::green : Qt::red);
    painter->setPen(pen);
    painter->drawRect(QRectF(1, 1, CARD_WIDTH - 2, CARD_HEIGHT - 2.5));
    painter->restore();
}

void DeckViewCard::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < 2 * QApplication::startDragDistance())
        return;
    
    if (static_cast<DeckViewScene *>(scene())->getLocked())
        return;
    
    delete dragItem;
    dragItem = new DeckViewCardDragItem(this, event->pos());
    scene()->addItem(dragItem);
    dragItem->updatePosition(event->scenePos());
    dragItem->grabMouse();
    
    QList<QGraphicsItem *> sel = scene()->selectedItems();
    int j = 0;
    for (int i = 0; i < sel.size(); i++) {
        DeckViewCard *c = static_cast<DeckViewCard *>(sel.at(i));
        if (c == this)
            continue;
        ++j;
        QPointF childPos = QPointF(j * CARD_WIDTH / 2, 0);
        DeckViewCardDragItem *drag = new DeckViewCardDragItem(c, childPos, dragItem);
        drag->setPos(dragItem->pos() + childPos);
        scene()->addItem(drag);
    }
    setCursor(Qt::OpenHandCursor);
}

void DeckViewCard::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    processHoverEvent();
}

DeckViewCardContainer::DeckViewCardContainer(const QString &_name)
    : QGraphicsItem(), name(_name), width(0), height(0)
{
    setCacheMode(DeviceCoordinateCache);
}

QRectF DeckViewCardContainer::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void DeckViewCardContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    qreal totalTextWidth = getCardTypeTextWidth();
    
    painter->fillRect(boundingRect(), themeManager->getTableBgBrush());
    painter->setPen(QColor(255, 255, 255, 100));
    painter->drawLine(QPointF(0, separatorY), QPointF(width, separatorY));
    
    painter->setPen(QColor(Qt::white));
    QFont f("Serif");
    f.setStyleHint(QFont::Serif);
    f.setPixelSize(24);
    f.setWeight(QFont::Bold);
    painter->setFont(f);
    painter->drawText(10, 0, width - 20, separatorY, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, InnerDecklistNode::visibleNameFromName(name) + QString(": %1").arg(cards.size()));
    
    f.setPixelSize(16);
    painter->setFont(f);
    QList<QString> cardTypeList = cardsByType.uniqueKeys();
    qreal yUntilNow = separatorY + paddingY;
    for (int i = 0; i < cardTypeList.size(); ++i) {
        if (i != 0) {
            painter->setPen(QColor(255, 255, 255, 100));
            painter->drawLine(QPointF(0, yUntilNow - paddingY / 2), QPointF(width, yUntilNow - paddingY / 2));
        }
        qreal thisRowHeight = CARD_HEIGHT * currentRowsAndCols[i].first;
        QRectF textRect(0, yUntilNow, totalTextWidth, thisRowHeight);
        yUntilNow += thisRowHeight + paddingY;

        QString displayString = QString("%1\n(%2)").arg(cardTypeList[i]).arg(cardsByType.count(cardTypeList[i]));

        painter->setPen(Qt::white);
        painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, displayString);
    }
}

void DeckViewCardContainer::addCard(DeckViewCard *card)
{
    cards.append(card);
    cardsByType.insert(card->getInfo() ? card->getInfo()->getMainCardType() : "", card);
}

void DeckViewCardContainer::removeCard(DeckViewCard *card)
{
    cards.removeAt(cards.indexOf(card));
    cardsByType.remove(card->getInfo() ? card->getInfo()->getMainCardType(): "", card);
}

QList<QPair<int, int> > DeckViewCardContainer::getRowsAndCols() const
{
    QList<QPair<int, int> > result;
    QList<QString> cardTypeList = cardsByType.uniqueKeys();
    for (int i = 0; i < cardTypeList.size(); ++i)
        result.append(QPair<int, int>(1, cardsByType.count(cardTypeList[i])));
    return result;
}

int DeckViewCardContainer::getCardTypeTextWidth() const
{
    QFont f("Serif");
    f.setStyleHint(QFont::Serif);
    f.setPixelSize(16);
    f.setWeight(QFont::Bold);
    QFontMetrics fm(f);
    
    int maxCardTypeWidth = 0;
    QMapIterator<QString, DeckViewCard *> i(cardsByType);
    while (i.hasNext()) {
        int cardTypeWidth = fm.size(Qt::TextSingleLine, i.next().key()).width();
        if (cardTypeWidth > maxCardTypeWidth)
            maxCardTypeWidth = cardTypeWidth;
    }
    
    return maxCardTypeWidth + 15;
}

QSizeF DeckViewCardContainer::calculateBoundingRect(const QList<QPair<int, int> > &rowsAndCols) const
{
    qreal totalHeight = 0;
    qreal totalWidth = 0;
    
    // Calculate space needed for cards
    for (int i = 0; i < rowsAndCols.size(); ++i) {
        totalHeight += CARD_HEIGHT * rowsAndCols[i].first + paddingY;
        if (CARD_WIDTH * rowsAndCols[i].second > totalWidth)
            totalWidth = CARD_WIDTH * rowsAndCols[i].second;
    }
    
    return QSizeF(getCardTypeTextWidth() + totalWidth, totalHeight + separatorY + paddingY);
}

bool DeckViewCardContainer::sortCardsByName(DeckViewCard * c1, DeckViewCard * c2)
{
    if (c1 && c2)
       return c1->getName() < c2->getName();
    return false;
}

void DeckViewCardContainer::rearrangeItems(const QList<QPair<int, int> > &rowsAndCols)
{
    currentRowsAndCols = rowsAndCols;
    
    int totalCols = 0, totalRows = 0;
    qreal yUntilNow = separatorY + paddingY;
    qreal x = (qreal) getCardTypeTextWidth();
    for (int i = 0; i < rowsAndCols.size(); ++i) {
        const int tempRows = rowsAndCols[i].first;
        const int tempCols = rowsAndCols[i].second;
        totalRows += tempRows;
        if (tempCols > totalCols)
            totalCols = tempCols;
        
        QList<QString> cardTypeList = cardsByType.uniqueKeys();
        QList<DeckViewCard *> row = cardsByType.values(cardTypeList[i]);
        qSort( row.begin(), row.end(), DeckViewCardContainer::sortCardsByName);
        for (int j = 0; j < row.size(); ++j) {
            DeckViewCard *card = row[j];
            card->setPos(x + (j % tempCols) * CARD_WIDTH, yUntilNow + (j / tempCols) * CARD_HEIGHT);
        }
        yUntilNow += tempRows * CARD_HEIGHT + paddingY;
    }
    
    prepareGeometryChange();
    QSizeF bRect = calculateBoundingRect(rowsAndCols);
    width = bRect.width();
    height = bRect.height();
}

void DeckViewCardContainer::setWidth(qreal _width)
{
    prepareGeometryChange();
    width = _width;
    update();
}

DeckViewScene::DeckViewScene(QObject *parent)
    : QGraphicsScene(parent), locked(true), deck(0), optimalAspectRatio(1.0)
{
}

DeckViewScene::~DeckViewScene()
{
    clearContents();
    delete deck;
}

void DeckViewScene::clearContents()
{
    QMapIterator<QString, DeckViewCardContainer *> i(cardContainers);
    while (i.hasNext())
        delete i.next().value();
    cardContainers.clear();
}

void DeckViewScene::setDeck(const DeckList &_deck)
{
    if (deck)
        delete deck;
    
    deck = new DeckList(_deck);
    rebuildTree();
    applySideboardPlan(deck->getCurrentSideboardPlan());
    rearrangeItems();
}

void DeckViewScene::rebuildTree()
{
    clearContents();
    
    if (!deck)
        return;
    
    InnerDecklistNode *listRoot = deck->getRoot();
    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        
        DeckViewCardContainer *container = cardContainers.value(currentZone->getName(), 0);
        if (!container) {
            container = new DeckViewCardContainer(currentZone->getName());
            cardContainers.insert(currentZone->getName(), container);
            addItem(container);
        }
        
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                DeckViewCard *newCard = new DeckViewCard(currentCard->getName(), currentZone->getName(), container);
                container->addCard(newCard);
                emit newCardAdded(newCard);
            }
        }
    }
}

void DeckViewScene::applySideboardPlan(const QList<MoveCard_ToZone> &plan)
{
    for (int i = 0; i < plan.size(); ++i) {
        const MoveCard_ToZone &m = plan[i];
        DeckViewCardContainer *start = cardContainers.value(QString::fromStdString(m.start_zone()));
        DeckViewCardContainer *target = cardContainers.value(QString::fromStdString(m.target_zone()));
        if (!start || !target)
            continue;
        
        DeckViewCard *card = 0;
        const QList<DeckViewCard *> &cardList = start->getCards();
        for (int j = 0; j < cardList.size(); ++j)
            if (cardList[j]->getName() == QString::fromStdString(m.card_name())) {
                card = cardList[j];
                break;
            }
        if (!card)
            continue;
        
        start->removeCard(card);
        target->addCard(card);
        card->setParentItem(target);
    }
}

void DeckViewScene::rearrangeItems()
{
    const int spacing = CARD_HEIGHT / 3;
    QList<DeckViewCardContainer *> contList = cardContainers.values();
    
    // Initialize space requirements
    QList<QList<QPair<int, int> > > rowsAndColsList;
    QList<QList<int> > cardCountList;
    for (int i = 0; i < contList.size(); ++i) {
        QList<QPair<int, int> > rowsAndCols = contList[i]->getRowsAndCols();
        rowsAndColsList.append(rowsAndCols);
        
        cardCountList.append(QList<int>());
        for (int j = 0; j < rowsAndCols.size(); ++j)
            cardCountList[i].append(rowsAndCols[j].second);
    }
    
    qreal totalHeight, totalWidth;
    for (;;) {
        // Calculate total size before this iteration
        totalHeight = -spacing;
        totalWidth = 0;
        for (int i = 0; i < contList.size(); ++i) {
            QSizeF contSize = contList[i]->calculateBoundingRect(rowsAndColsList[i]);
            totalHeight += contSize.height() + spacing;
            if (contSize.width() > totalWidth)
                totalWidth = contSize.width();
        }
        
        // We're done when the aspect ratio shifts from too high to too low.
        if (totalWidth / totalHeight <= optimalAspectRatio)
            break;
        
        // Find category with highest column count
        int maxIndex1 = -1, maxIndex2 = -1, maxCols = 0;
        for (int i = 0; i < rowsAndColsList.size(); ++i)
            for (int j = 0; j < rowsAndColsList[i].size(); ++j)
                if (rowsAndColsList[i][j].second > maxCols) {
                    maxIndex1 = i;
                    maxIndex2 = j;
                    maxCols = rowsAndColsList[i][j].second;
                }
        
        // Add row to category
        const int maxRows = rowsAndColsList[maxIndex1][maxIndex2].first;
        const int maxCardCount = cardCountList[maxIndex1][maxIndex2];
        rowsAndColsList[maxIndex1][maxIndex2] = QPair<int, int>(maxRows + 1, (int) ceil((qreal) maxCardCount / (qreal) (maxRows + 1)));
    }
    
    totalHeight = -spacing;
    for (int i = 0; i < contList.size(); ++i) {
        DeckViewCardContainer *c = contList[i];
        c->rearrangeItems(rowsAndColsList[i]);
        c->setPos(0, totalHeight + spacing);
        totalHeight += c->boundingRect().height() + spacing;
    }
    
    totalWidth = totalHeight * optimalAspectRatio;
    for (int i = 0; i < contList.size(); ++i)
        contList[i]->setWidth(totalWidth);
    
    setSceneRect(QRectF(0, 0, totalWidth, totalHeight));
}

void DeckViewScene::updateContents()
{
    rearrangeItems();
    emit sideboardPlanChanged();
}

QList<MoveCard_ToZone> DeckViewScene::getSideboardPlan() const
{
    QList<MoveCard_ToZone> result;
    QMapIterator<QString, DeckViewCardContainer *> containerIterator(cardContainers);
    while (containerIterator.hasNext()) {
        DeckViewCardContainer *cont = containerIterator.next().value();
        const QList<DeckViewCard *> cardList = cont->getCards();
        for (int i = 0; i < cardList.size(); ++i)
            if (cardList[i]->getOriginZone() != cont->getName()) {
                MoveCard_ToZone m;
                m.set_card_name(cardList[i]->getName().toStdString());
                m.set_start_zone(cardList[i]->getOriginZone().toStdString());
                m.set_target_zone(cont->getName().toStdString());
                result.append(m);
            }
    }
    return result;
}

void DeckViewScene::resetSideboardPlan()
{
    rebuildTree();
    rearrangeItems();
}

DeckView::DeckView(QWidget *parent)
    : QGraphicsView(parent)
{
    deckViewScene = new DeckViewScene(this);
    
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));
    setDragMode(RubberBandDrag);
    setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing/* | QPainter::SmoothPixmapTransform*/);
    setScene(deckViewScene);

    connect(deckViewScene, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(updateSceneRect(const QRectF &)));
    connect(deckViewScene, SIGNAL(newCardAdded(AbstractCardItem *)), this, SIGNAL(newCardAdded(AbstractCardItem *)));
    connect(deckViewScene, SIGNAL(sideboardPlanChanged()), this, SIGNAL(sideboardPlanChanged()));
}

void DeckView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    deckViewScene->setOptimalAspectRatio((qreal) width() / (qreal) height());
    deckViewScene->rearrangeItems();
}

void DeckView::updateSceneRect(const QRectF &rect)
{
    fitInView(rect, Qt::KeepAspectRatio);
}

void DeckView::setDeck(const DeckList &_deck)
{
    deckViewScene->setDeck(_deck);
}

void DeckView::resetSideboardPlan()
{
    deckViewScene->resetSideboardPlan();
}
