#include "deck_analytics_widget_factory.h"

#include "deck_analytics_widget_base.h"

DeckAnalyticsWidgetFactory &DeckAnalyticsWidgetFactory::instance()
{
    static DeckAnalyticsWidgetFactory f;
    return f;
}

void DeckAnalyticsWidgetFactory::registerWidget(const Descriptor &desc)
{
    widgets.insert(desc.type, desc);
}

AnalyticsWidgetBase *
DeckAnalyticsWidgetFactory::create(const QString &type, QWidget *parent, DeckListStatisticsAnalyzer *analyzer) const
{
    auto it = widgets.find(type);
    if (it == widgets.end())
        return nullptr;

    auto w = it->creator(parent, analyzer);

    w->setDisplayTitle(it->title);

    return w;
}

QList<DeckAnalyticsWidgetFactory::Descriptor> DeckAnalyticsWidgetFactory::availableWidgets() const
{
    return widgets.values();
}