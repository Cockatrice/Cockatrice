#ifndef COCKATRICE_DECK_ANALYTICS_WIDGET_FACTORY_H
#define COCKATRICE_DECK_ANALYTICS_WIDGET_FACTORY_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>

class AnalyticsWidgetBase;
class DeckListStatisticsAnalyzer;

class DeckAnalyticsWidgetFactory
{
public:
    using Creator = std::function<AnalyticsWidgetBase *(QWidget *, DeckListStatisticsAnalyzer *)>;

    struct Descriptor
    {
        QString type;  // stable ID ("manaProdDevotion")
        QString title; // translated, user-facing
        Creator creator;
    };

    static DeckAnalyticsWidgetFactory &instance();

    // NEW: richer registration
    void registerWidget(const Descriptor &desc);

    AnalyticsWidgetBase *create(const QString &type, QWidget *parent, DeckListStatisticsAnalyzer *analyzer) const;

    // NEW: expose widgets to UI
    QList<Descriptor> availableWidgets() const;

private:
    DeckAnalyticsWidgetFactory() = default; // Ensure private constructor
    DeckAnalyticsWidgetFactory(const DeckAnalyticsWidgetFactory &) = delete;
    DeckAnalyticsWidgetFactory &operator=(const DeckAnalyticsWidgetFactory &) = delete;

    QMap<QString, Descriptor> widgets;
};

#endif
