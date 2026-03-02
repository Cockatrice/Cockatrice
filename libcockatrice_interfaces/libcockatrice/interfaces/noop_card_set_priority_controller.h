#ifndef COCKATRICE_NOOP_CARD_SET_PRIORITY_CONTROLLER_H
#define COCKATRICE_NOOP_CARD_SET_PRIORITY_CONTROLLER_H

#include "interface_card_set_priority_controller.h"

class NoopCardSetPriorityController : public ICardSetPriorityController
{
public:
    void setSortKey(QString /* shortName */, unsigned int /* sortKey */) override
    {
    }
    void setEnabled(QString /* shortName */, bool /* enabled */) override
    {
    }
    void setIsKnown(QString /* shortName */, bool /* isknown */) override
    {
    }

    unsigned int getSortKey(QString /* shortName */) override
    {
        return 0;
    }
    bool isEnabled(QString /* shortName */) override
    {
        return true;
    }
    bool isKnown(QString /* shortName */) override
    {
        return true;
    }
};

#endif // COCKATRICE_NOOP_CARD_SET_PRIORITY_CONTROLLER_H
