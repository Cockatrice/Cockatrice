#include "analytics_panel_widget_factory.h"

#include "abstract_analytics_panel_widget.h"

AnalyticsPanelWidgetFactory &AnalyticsPanelWidgetFactory::instance()
{
    static AnalyticsPanelWidgetFactory f;
    return f;
}

void AnalyticsPanelWidgetFactory::registerWidget(const Descriptor &desc)
{
    widgets.insert(desc.type, desc);
}

AbstractAnalyticsPanelWidget *
AnalyticsPanelWidgetFactory::create(const QString &type, QWidget *parent, DeckListStatisticsAnalyzer *analyzer) const
{
    auto it = widgets.find(type);
    if (it == widgets.end())
        return nullptr;

    auto w = it->creator(parent, analyzer);

    w->setDisplayTitle(it->title);

    return w;
}

QList<AnalyticsPanelWidgetFactory::Descriptor> AnalyticsPanelWidgetFactory::availableWidgets() const
{
    return widgets.values();
}