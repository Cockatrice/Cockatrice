#ifndef KEYBOARD_NAVIGATOR_TEST_FAKES_H
#define KEYBOARD_NAVIGATOR_TEST_FAKES_H

#include "game/board/card_item.h"
#include "game/zones/card_zone_logic.h"
#include "game/zones/hand_zone_logic.h"
#include "game/zones/stack_zone_logic.h"
#include "game/zones/table_zone_logic.h"

// Define safe macro replacements for the visual tests
#define setFocus() zValue()
#define getVisuallyOrderedHandCards() getCards()

namespace QApplicationMock
{
extern bool hasPopup;
}

class FakeCardZoneLogic : public CardZoneLogic
{
public:
    QString name;
    FakeCardZoneLogic(const QString &n) : CardZoneLogic(nullptr, n, false, false, true, nullptr), name(n)
    {
    }
    void addCardImpl(CardItem *, int, int) override
    {
    }
    void setDummyCardCount(int count)
    {
        cards.clear();
        for (int i = 0; i < count; i++) {
            cards.insert(i, (CardItem *)nullptr);
        }
    }
    const QString getName() const
    {
        return name;
    }
};

#define DECLARE_FAKE_ZONE(FakeName, TargetName)                                                                        \
    class FakeName : public FakeCardZoneLogic                                                                          \
    {                                                                                                                  \
    public:                                                                                                            \
        FakeName(const QString &n) : FakeCardZoneLogic(n)                                                              \
        {                                                                                                              \
        }                                                                                                              \
        const QMetaObject *metaObject() const override                                                                 \
        {                                                                                                              \
            return &TargetName::staticMetaObject;                                                                      \
        }                                                                                                              \
        void *qt_metacast(const char *clname) override                                                                 \
        {                                                                                                              \
            if (!clname)                                                                                               \
                return nullptr;                                                                                        \
            if (!strcmp(clname, #TargetName))                                                                          \
                return static_cast<void *>(this);                                                                      \
            return FakeCardZoneLogic::qt_metacast(clname);                                                             \
        }                                                                                                              \
    };

DECLARE_FAKE_ZONE(FakeTableZoneLogic, TableZoneLogic)
DECLARE_FAKE_ZONE(FakeStackZoneLogic, StackZoneLogic)
DECLARE_FAKE_ZONE(FakeHandZoneLogic, HandZoneLogic)

#endif // KEYBOARD_NAVIGATOR_TEST_FAKES_H
