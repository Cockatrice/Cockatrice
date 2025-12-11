#ifndef COCKATRICE_DECK_ANALYTICS_WIDGET_REGISTRAR_H
#define COCKATRICE_DECK_ANALYTICS_WIDGET_REGISTRAR_H

#include "deck_analytics_widget_factory.h"

class DeckAnalyticsWidgetRegistrar
{
public:
    DeckAnalyticsWidgetRegistrar(const QString &type, const QString &title, DeckAnalyticsWidgetFactory::Creator creator)
    {
        DeckAnalyticsWidgetFactory::instance().registerWidget({type, title, creator});
    }
};

#endif // COCKATRICE_DECK_ANALYTICS_WIDGET_REGISTRAR_H
