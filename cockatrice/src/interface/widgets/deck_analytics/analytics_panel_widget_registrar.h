#ifndef COCKATRICE_DECK_ANALYTICS_WIDGET_REGISTRAR_H
#define COCKATRICE_DECK_ANALYTICS_WIDGET_REGISTRAR_H

#include "analytics_panel_widget_factory.h"

class AnalyticsPanelWidgetRegistrar
{
public:
    AnalyticsPanelWidgetRegistrar(const QString &type,
                                  const QString &title,
                                  AnalyticsPanelWidgetFactory::Creator creator)
    {
        AnalyticsPanelWidgetFactory::instance().registerWidget({type, title, creator});
    }
};

#endif // COCKATRICE_DECK_ANALYTICS_WIDGET_REGISTRAR_H
