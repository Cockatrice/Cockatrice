#ifndef COCKATRICE_INTERFACE_CARD_SET_PRIORITY_CONTROLLER_H
#define COCKATRICE_INTERFACE_CARD_SET_PRIORITY_CONTROLLER_H
#include <QString>

class ICardSetPriorityController
{
public:
    virtual ~ICardSetPriorityController() = default;

    virtual void setSortKey(QString shortName, unsigned int sortKey) = 0;
    virtual void setEnabled(QString shortName, bool enabled) = 0;
    virtual void setIsKnown(QString shortName, bool isknown) = 0;

    virtual unsigned int getSortKey(QString shortName) = 0;
    virtual bool isEnabled(QString shortName) = 0;
    virtual bool isKnown(QString shortName) = 0;
};

#endif // COCKATRICE_INTERFACE_CARD_SET_PRIORITY_CONTROLLER_H
