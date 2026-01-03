#include "deck_analytics_widget.h"

#include "abstract_analytics_panel_widget.h"
#include "add_analytics_panel_dialog.h"
#include "analytics_panel_widget_factory.h"
#include "analyzer_modules/mana_base/mana_base_config.h"
#include "analyzer_modules/mana_curve/mana_curve_config.h"
#include "analyzer_modules/mana_devotion/mana_devotion_config.h"
#include "deck_list_statistics_analyzer.h"
#include "resizable_panel.h"

#include <QEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QVBoxLayout>

DeckAnalyticsWidget::DeckAnalyticsWidget(QWidget *parent, DeckListStatisticsAnalyzer *_statsAnalyzer)
    : QWidget(parent), statsAnalyzer(_statsAnalyzer)
{
    layout = new QVBoxLayout(this);

    // Controls
    controlContainer = new QWidget(this);
    controlLayout = new QHBoxLayout(controlContainer);
    addButton = new QPushButton(this);
    removeButton = new QPushButton(this);
    saveButton = new QPushButton(this);
    loadButton = new QPushButton(this);
    controlLayout->addWidget(addButton);
    controlLayout->addWidget(removeButton);
    controlLayout->addWidget(saveButton);
    controlLayout->addWidget(loadButton);

    layout->addWidget(controlContainer);

    connect(addButton, &QPushButton::clicked, this, &DeckAnalyticsWidget::onAddPanel);
    connect(removeButton, &QPushButton::clicked, this, &DeckAnalyticsWidget::onRemoveSelected);
    connect(saveButton, &QPushButton::clicked, this, &DeckAnalyticsWidget::saveLayout);
    connect(loadButton, &QPushButton::clicked, this, &DeckAnalyticsWidget::loadLayout);

    // Scroll area and container
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    panelContainer = new QWidget(scrollArea);
    panelLayout = new QVBoxLayout(panelContainer);
    panelLayout->setSpacing(8);
    panelLayout->setContentsMargins(4, 4, 4, 4);
    panelLayout->addStretch(1); // push panels up

    scrollArea->setWidget(panelContainer);
    layout->addWidget(scrollArea);

    loadLayout();

    retranslateUi();
}

void DeckAnalyticsWidget::retranslateUi()
{
    addButton->setText(tr("Add Panel"));
    removeButton->setText(tr("Remove Panel"));
    saveButton->setText(tr("Save Layout"));
    loadButton->setText(tr("Load Layout"));
}

void DeckAnalyticsWidget::updateDisplays()
{
    statsAnalyzer->analyze();
}

void DeckAnalyticsWidget::onAddPanel()
{
    AddAnalyticsPanelDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString selection = dlg.selectedType();
    if (selection.isEmpty()) {
        return;
    }

    AbstractAnalyticsPanelWidget *analyticsWidget =
        AnalyticsPanelWidgetFactory::instance().create(selection, this, statsAnalyzer);
    if (!analyticsWidget) {
        return;
    }

    if (!analyticsWidget->applyConfigFromDialog()) {
        analyticsWidget->deleteLater();
        return;
    }

    addPanelInstance(selection, analyticsWidget, analyticsWidget->saveConfig());
}

void DeckAnalyticsWidget::addPanelInstance(const QString &typeId,
                                           AbstractAnalyticsPanelWidget *panel,
                                           const QJsonObject &cfg)
{
    panel->loadConfig(cfg);
    panel->updateDisplay();

    auto *resPanel = new ResizablePanel(typeId, panel, panelContainer);
    panelWrappers.push_back(resPanel);

    panelLayout->insertWidget(panelLayout->count() - 1, resPanel);

    // Event filter for selection
    resPanel->installEventFilter(this);
    panel->installEventFilter(this);

    // Connect drag-drop signals
    connect(resPanel, &ResizablePanel::dropRequested, this, &DeckAnalyticsWidget::onPanelDropped);
}

void DeckAnalyticsWidget::onRemoveSelected()
{
    int idx = indexOfSelectedWrapper();
    if (idx < 0) {
        return;
    }

    ResizablePanel *panel = panelWrappers.takeAt(idx);
    selectWrapper(nullptr);

    panel->deleteLater();
}

void DeckAnalyticsWidget::saveLayout()
{
    QJsonArray arr;

    for (auto *wrapper : panelWrappers) {
        QJsonObject entry;
        entry["type"] = wrapper->getTypeId();
        entry["config"] = wrapper->panel->saveConfig();
        entry["height"] = wrapper->getCurrentHeight();
        arr.append(entry);
    }

    QSettings s;
    s.setValue("deckAnalytics/layout", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

void DeckAnalyticsWidget::loadLayout()
{
    if (!loadLayoutInternal()) {
        addDefaultPanels();
    }
}

void DeckAnalyticsWidget::addDefaultPanels()
{
    struct DefaultPanel
    {
        QString type;
        QJsonObject cfg;
    };

    // Prepare configs
    QJsonObject manaCurveCfg = ManaCurveConfig{}.toJson();
    QJsonObject manaBaseCfg = ManaBaseConfig{"combinedBar", {}}.toJson();
    QJsonObject manaDevotionCfg = ManaDevotionConfig{"combinedBar", {}}.toJson();
    QVector<DefaultPanel> defaults = {
        {"manaCurve", manaCurveCfg}, {"manaBase", manaBaseCfg}, {"manaDevotion", manaDevotionCfg}};

    for (auto &d : defaults) {
        AbstractAnalyticsPanelWidget *w = AnalyticsPanelWidgetFactory::instance().create(d.type, this, statsAnalyzer);
        if (!w) {
            continue;
        }

        w->loadConfig(d.cfg);
        addPanelInstance(d.type, w, d.cfg);
    }
}

bool DeckAnalyticsWidget::loadLayoutInternal()
{
    QSettings s;
    QString layoutData = s.value("deckAnalytics/layout").toString();
    if (layoutData.isEmpty()) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(layoutData.toUtf8());
    if (!doc.isArray()) {
        return false;
    }

    clearPanels();

    for (auto v : doc.array()) {
        if (!v.isObject()) {
            continue;
        }
        QJsonObject o = v.toObject();
        QString type = o["type"].toString();
        QJsonObject cfg = o["config"].toObject();

        AbstractAnalyticsPanelWidget *w = AnalyticsPanelWidgetFactory::instance().create(type, this, statsAnalyzer);
        if (!w) {
            continue;
        }

        addPanelInstance(type, w, cfg);

        // Restore height AFTER adding the panel
        if (o.contains("height")) {
            panelWrappers.last()->setHeightFromSaved(o["height"].toInt());
        }
    }

    return true;
}

void DeckAnalyticsWidget::clearPanels()
{
    selectWrapper(nullptr);
    while (!panelWrappers.isEmpty()) {
        ResizablePanel *p = panelWrappers.takeLast();
        p->deleteLater();
    }
}

bool DeckAnalyticsWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        for (auto *p : panelWrappers) {
            if (obj == p || obj == p->panel) {
                selectWrapper(p);
                break;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void DeckAnalyticsWidget::selectWrapper(ResizablePanel *w)
{
    // Same wrapper
    if (selectedWrapper == w) {
        return;
    }
    // Deselect the old one
    if (selectedWrapper) {
        selectedWrapper->setSelected(false);
    }
    // Set current
    selectedWrapper = w;
    // Finally, select new
    if (selectedWrapper) {
        selectedWrapper->setSelected(true);
    }
}

int DeckAnalyticsWidget::indexOfSelectedWrapper() const
{
    if (!selectedWrapper) {
        return -1;
    }
    return panelWrappers.indexOf(selectedWrapper);
}

void DeckAnalyticsWidget::onPanelDropped(ResizablePanel *dragged, ResizablePanel *target, bool insertBefore)
{
    int draggedIdx = panelWrappers.indexOf(dragged);
    int targetIdx = panelWrappers.indexOf(target);

    if (draggedIdx == -1 || targetIdx == -1 || draggedIdx == targetIdx) {
        return;
    }

    // Remove dragged panel from list and layout
    panelWrappers.removeAt(draggedIdx);
    panelLayout->removeWidget(dragged);

    // Adjust target index if needed
    if (draggedIdx < targetIdx) {
        targetIdx--;
    }

    // Calculate insertion position
    int insertIdx = insertBefore ? targetIdx : targetIdx + 1;

    // Insert back into list and layout
    panelWrappers.insert(insertIdx, dragged);
    panelLayout->insertWidget(insertIdx, dragged);

    // Clear selection
    selectWrapper(nullptr);
}