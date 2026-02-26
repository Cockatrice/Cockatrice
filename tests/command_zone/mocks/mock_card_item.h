/**
 * @file mock_card_item.h
 * @brief Lightweight CardItem stub for command zone unit tests.
 *
 * This mock provides minimal CardItem functionality needed to test
 * CommandZoneLogic::addCardImpl without pulling in graphics dependencies.
 */

#ifndef MOCK_CARD_ITEM_H
#define MOCK_CARD_ITEM_H

#include <QObject>
#include <QString>

/**
 * @brief Stub CardRef for tests (avoids network/protobuf dependencies)
 */
struct CardRef
{
    QString name;
    QString setCode;
};

/**
 * @brief Lightweight CardItem mock for testing zone logic.
 *
 * Tracks method calls and state changes for verification in tests.
 */
class MockCardItem : public QObject
{
    Q_OBJECT
public:
    explicit MockCardItem(int _id = 1, QObject *parent = nullptr) : QObject(parent), id(_id)
    {
    }

    // State tracking
    int id;
    bool visible = false;
    bool resetStateCalled = false;
    bool resetStateShowFaceDown = false;
    CardRef cardRef;

    // CardItem interface methods used by CommandZoneLogic::addCardImpl
    [[nodiscard]] int getId() const
    {
        return id;
    }
    void setId(int _id)
    {
        id = _id;
    }
    void setVisible(bool v)
    {
        visible = v;
    }
    [[nodiscard]] bool isVisible() const
    {
        return visible;
    }
    void resetState(bool showFaceDown)
    {
        resetStateCalled = true;
        resetStateShowFaceDown = showFaceDown;
    }
    void setCardRef(const CardRef &ref)
    {
        cardRef = ref;
    }
};

#endif // MOCK_CARD_ITEM_H
