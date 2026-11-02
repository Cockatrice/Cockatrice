#include "draw_probability_widget.h"

#include "../../../general/display/banner_widget.h"
#include "../../../general/layout_containers/flow_widget.h"
#include "../../abstract_analytics_panel_widget.h"
#include "../../deck_list_statistics_analyzer.h"
#include "deck_list_model.h"
#include "draw_probability_calculator.h"
#include "draw_probability_config_dialog.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialog>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVariant>
#include <libcockatrice/card/database/card_database_manager.h>

class QWidget;

DrawProbabilityWidget::DrawProbabilityWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer)
    : AbstractAnalyticsPanelWidget(parent, analyzer)
{
    controls = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    controls->setSpacing(4, 4);

    labelPrefix = new QLabel(this);
    controls->addWidget(labelPrefix);

    criteriaCombo = new QComboBox(this);
    // Give these things item-data so we can translate the actual user-facing strings
    criteriaCombo->addItem(QString(), "name");
    criteriaCombo->addItem(QString(), "type");
    criteriaCombo->addItem(QString(), "subtype");
    criteriaCombo->addItem(QString(), "cmc");
    controls->addWidget(criteriaCombo);

    exactnessCombo = new QComboBox(this);
    exactnessCombo->addItem(QString(), true);  // At least
    exactnessCombo->addItem(QString(), false); // Exactly
    controls->addWidget(exactnessCombo);

    quantitySpin = new QSpinBox(this);
    quantitySpin->setRange(0, 60);
    controls->addWidget(quantitySpin);

    labelMiddle = new QLabel(this);
    controls->addWidget(labelMiddle);

    drawnSpin = new QSpinBox(this);
    drawnSpin->setRange(0, 60);
    drawnSpin->setValue(7);
    controls->addWidget(drawnSpin);

    labelSuffix = new QLabel(this);
    controls->addWidget(labelSuffix);

    labelPrefix->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    labelMiddle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    labelSuffix->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    layout->addWidget(controls);

    populationLabel = new QLabel(this);
    populationLabel->setWordWrap(true);
    layout->addWidget(populationLabel);

    // Table
    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(3);
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(resultTable);

    // Connections
    connect(criteriaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        config.criteria = criteriaCombo->currentData().toString();
        updateDisplay();
    });

    connect(exactnessCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        config.atLeast = exactnessCombo->currentData().toBool();
        updateDisplay();
    });

    connect(quantitySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
        config.quantity = v;
        updateDisplay();
    });

    connect(drawnSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
        config.drawn = v;
        updateDisplay();
    });

    retranslateUi();
    applyConfigToToolbar();
    updateFilterOptions();
}

void DrawProbabilityWidget::retranslateUi()
{
    bannerWidget->setText(tr("Draw Probability"));

    labelPrefix->setText(tr("Probability of drawing"));

    criteriaCombo->setItemText(0, tr("Card Name"));
    criteriaCombo->setItemText(1, tr("Type"));
    criteriaCombo->setItemText(2, tr("Subtype"));
    criteriaCombo->setItemText(3, tr("Mana Value"));

    exactnessCombo->setItemText(0, tr("At least"));
    exactnessCombo->setItemText(1, tr("Exactly"));

    labelMiddle->setText(tr("card(s) in a draw of"));
    labelSuffix->setText(tr("cards"));

    resultTable->setHorizontalHeaderLabels({tr("Category"), tr("Qty"), tr("Odds (%)")});
    updateFilterOptions();
}

QDialog *DrawProbabilityWidget::createConfigDialog(QWidget *parent)
{
    auto *dlg = new DrawProbabilityConfigDialog(parent);
    dlg->setFromConfig(config);
    return dlg;
}

QJsonObject DrawProbabilityWidget::extractConfigFromDialog(QDialog *dlg) const
{
    auto *dp = qobject_cast<DrawProbabilityConfigDialog *>(dlg);
    return dp ? dp->result().toJson() : QJsonObject{};
}

void DrawProbabilityWidget::applyConfigToToolbar()
{
    // Apply the complete configuration before calculating; each control normally triggers a refresh.
    const QSignalBlocker criteriaBlocker(criteriaCombo);
    const QSignalBlocker exactnessBlocker(exactnessCombo);
    const QSignalBlocker quantityBlocker(quantitySpin);
    const QSignalBlocker drawnBlocker(drawnSpin);
    auto setComboByData = [](QComboBox *combo, const QVariant &value) {
        int idx = combo->findData(value);
        if (idx >= 0) {
            combo->setCurrentIndex(idx);
        }
    };

    setComboByData(criteriaCombo, config.criteria);
    setComboByData(exactnessCombo, config.atLeast);

    quantitySpin->setValue(config.quantity);
    drawnSpin->setValue(config.drawn);
}

void DrawProbabilityWidget::updateDisplay()
{
    updateFilterOptions();
}

void DrawProbabilityWidget::loadConfig(const QJsonObject &cfg)
{
    config = DrawProbabilityConfig::fromJson(cfg);
    applyConfigToToolbar();
    updateFilterOptions();
}

void DrawProbabilityWidget::updateFilterOptions()
{
    resultTable->setRowCount(0);
    populationLabel->clear();
    if (!analyzer || !analyzer->getModel() || !analyzer->getModel()->getDeckList()) {
        return;
    }

    const auto population = collectDrawProbabilityPopulation(*analyzer->getModel()->getDeckList(),
                                                             *CardDatabaseManager::query(), config.criteria);
    QString unavailableReason;
    if (population.unclassifiedCards > 0) {
        unavailableReason = tr("Card data is missing for %1 main-deck cards. Odds for this category are unavailable.")
                                .arg(population.unclassifiedCards);
    } else if (config.drawn > population.totalCards) {
        unavailableReason = tr("Choose a draw size no larger than the main deck to calculate odds.");
    }
    populationLabel->setText(tr("Main deck: %1 cards.").arg(population.totalCards) +
                             (unavailableReason.isEmpty() ? QString() : " " + unavailableReason));

    resultTable->setRowCount(population.categoryCounts.size());
    int row = 0;
    for (auto it = population.categoryCounts.cbegin(); it != population.categoryCounts.cend(); ++it, ++row) {
        const auto probability = unavailableReason.isEmpty()
                                     ? calculateDrawProbability(population.totalCards, it.value(), config.drawn,
                                                                config.quantity, config.atLeast)
                                     : std::nullopt;
        resultTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        resultTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        auto *odds = new QTableWidgetItem(probability ? QString::number(*probability * 100.0, 'f', 2) : tr("N/A"));
        odds->setToolTip(unavailableReason);
        resultTable->setItem(row, 2, odds);
    }
}
