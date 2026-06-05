#include "game/board/abstract_card_item.h"
#include "game/board/arrow_item.h"
#include "game/board/card_item.h"
#include "game/board/card_list.h"
#include "game/player/player_logic.h"
#include "game/zones/card_zone_logic.h"
#include "game/zones/hand_zone_logic.h"
#include "game/zones/stack_zone_logic.h"
#include "game/zones/table_zone_logic.h"

#include <QApplication>
#include <QColor>
#include <QMetaObject>
#include <QString>

// Stubs for AbstractCardItem
void AbstractCardItem::setHovered(bool)
{
}

// Stubs for ArrowItem
ArrowItem::ArrowItem(PlayerLogic *, int, ArrowTarget *, ArrowTarget *, QColor const &)
    : QObject(nullptr), QGraphicsItem(nullptr)
{
}
void ArrowItem::sendCreateArrowCommand(PlayerLogic *, CardItem *, ArrowTarget *, QColor const &, int)
{
}
void ArrowItem::paint(QPainter *, QStyleOptionGraphicsItem const *, QWidget *)
{
}
void ArrowItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
}
const QMetaObject *ArrowItem::metaObject() const
{
    return nullptr;
}
void *ArrowItem::qt_metacast(const char *)
{
    return nullptr;
}
int ArrowItem::qt_metacall(QMetaObject::Call, int, void **)
{
    return 0;
}

// Stubs for QMetaObject
const QMetaObject TableZoneLogic::staticMetaObject = {
    {&QObject::staticMetaObject, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
const QMetaObject StackZoneLogic::staticMetaObject = {
    {&QObject::staticMetaObject, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
const QMetaObject HandZoneLogic::staticMetaObject = {
    {&QObject::staticMetaObject, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
const QMetaObject PlayerLogic::staticMetaObject = {
    {&QObject::staticMetaObject, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
const QMetaObject CardZoneLogic::staticMetaObject = {
    {&QObject::staticMetaObject, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};

// Stubs for CardList
CardList::CardList(bool) : QList<CardItem *>()
{
}

// Stubs for CardZoneLogic
CardZoneLogic::CardZoneLogic(PlayerLogic *, const QString &, bool, bool, bool _contentsKnown, QObject *)
    : QObject(nullptr), cards(_contentsKnown)
{
}
CardItem *CardZoneLogic::getCard(int)
{
    return nullptr;
}
CardItem *CardZoneLogic::takeCard(int, int, bool)
{
    return nullptr;
}
void CardZoneLogic::addCardImpl(CardItem *, int, int)
{
}
QString CardZoneLogic::getTranslatedName(bool, GrammaticalCase) const
{
    return QString();
}
const QMetaObject *CardZoneLogic::metaObject() const
{
    return nullptr;
}
void *CardZoneLogic::qt_metacast(const char *)
{
    return nullptr;
}
int CardZoneLogic::qt_metacall(QMetaObject::Call, int, void **)
{
    return 0;
}

// QApplication Mock implementation
namespace QApplicationMock
{
bool hasPopup = false;
}
QWidget *QApplication::activePopupWidget()
{
    return QApplicationMock::hasPopup ? (QWidget *)1 : nullptr;
}
