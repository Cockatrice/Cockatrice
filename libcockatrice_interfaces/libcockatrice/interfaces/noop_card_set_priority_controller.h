#ifndef COCKATRICE_NOOP_CARD_SET_PRIORITY_CONTROLLER_H
#define COCKATRICE_NOOP_CARD_SET_PRIORITY_CONTROLLER_H

#include "interface_card_set_priority_controller.h"

class NoopCardSetPriorityController : public ICardSetPriorityController
{
public:
    void setSortKey(QString /* shortName */, unsigned int /* sortKey */)
    {
    }
    void setEnabled(QString /* shortName */, bool /* enabled */)
    {
    }
    void setIsKnown(QString /* shortName */, bool /* isknown */)
    {
    }

    unsigned int getSortKey(QString /* shortName */)
    {
        return 0;
    }
    bool isEnabled(QString /* shortName */)
    {
        return true;
    }
    bool isKnown(QString /* shortName */)
    {
        return true;
    }
};

#endif // COCKATRICE_NOOP_CARD_SET_PRIORITY_CONTROLLER_H
