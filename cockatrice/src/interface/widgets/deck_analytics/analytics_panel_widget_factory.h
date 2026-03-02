#ifndef COCKATRICE_DECK_ANALYTICS_WIDGET_FACTORY_H
#define COCKATRICE_DECK_ANALYTICS_WIDGET_FACTORY_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>

class AbstractAnalyticsPanelWidget;
class DeckListStatisticsAnalyzer;

class AnalyticsPanelWidgetFactory
{
public:
    using Creator = std::function<AbstractAnalyticsPanelWidget *(QWidget *, DeckListStatisticsAnalyzer *)>;

    struct Descriptor
    {
        QString type;  // stable ID ("manaProdDevotion")
        QString title; // translated, user-facing
        Creator creator;
    };

    static AnalyticsPanelWidgetFactory &instance();

    // NEW: richer registration
    void registerWidget(const Descriptor &desc);

    AbstractAnalyticsPanelWidget *
    create(const QString &type, QWidget *parent, DeckListStatisticsAnalyzer *analyzer) const;

    // NEW: expose widgets to UI
    QList<Descriptor> availableWidgets() const;

private:
    AnalyticsPanelWidgetFactory() = default; // Ensure private constructor
    AnalyticsPanelWidgetFactory(const AnalyticsPanelWidgetFactory &) = delete;
    AnalyticsPanelWidgetFactory &operator=(const AnalyticsPanelWidgetFactory &) = delete;

    QMap<QString, Descriptor> widgets;
};

#endif
