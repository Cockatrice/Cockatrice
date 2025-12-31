/**
 * @file deck_analytics_widget.h
 * @ingroup DeckEditorAnalyticsWidgets
 * @brief Main analytics widget container with resizable panels for deck statistics.
 */

#ifndef DECK_ANALYTICS_WIDGET_H
#define DECK_ANALYTICS_WIDGET_H

#include "abstract_analytics_panel_widget.h"
#include "deck_list_statistics_analyzer.h"
#include "resizable_panel.h"

#include <QJsonObject>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

class LayoutInspector;

class DeckAnalyticsWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateDisplays();

public:
    explicit DeckAnalyticsWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer);
    void retranslateUi();

private slots:
    void onAddPanel();
    void onRemoveSelected();
    void onPanelDropped(ResizablePanel *dragged, ResizablePanel *target, bool insertBefore);
    void saveLayout();
    void loadLayout();
    void addDefaultPanels();
    bool loadLayoutInternal();
    void clearPanels();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void selectWrapper(ResizablePanel *panel);
    int indexOfSelectedWrapper() const;

private:
    void addPanelInstance(const QString &typeId, AbstractAnalyticsPanelWidget *panel, const QJsonObject &cfg = {});

    QVBoxLayout *layout;
    QWidget *controlContainer;
    QHBoxLayout *controlLayout;

    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *saveButton;
    QPushButton *loadButton;

    QScrollArea *scrollArea;
    QWidget *panelContainer;
    QVBoxLayout *panelLayout;

    QVector<ResizablePanel *> panelWrappers;
    ResizablePanel *selectedWrapper = nullptr;

    DeckListStatisticsAnalyzer *statsAnalyzer;
    LayoutInspector *insp = nullptr;
};

#endif // DECK_ANALYTICS_WIDGET_H
