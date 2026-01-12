#include "draw_probability_widget.h"

#include "draw_probability_config_dialog.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QSpinBox>
#include <QTableWidgetItem>
#include <QWidget>
#include <QtMath>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>

DrawProbabilityWidget::DrawProbabilityWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer)
    : AbstractAnalyticsPanelWidget(parent, analyzer)
{
    controls = new QWidget(this);
    controlLayout = new QHBoxLayout(controls);

    labelPrefix = new QLabel(this);
    controlLayout->addWidget(labelPrefix);

    criteriaCombo = new QComboBox(this);
    // Give these things item-data so we can translate the actual user-facing strings
    criteriaCombo->addItem(QString(), "name");
    criteriaCombo->addItem(QString(), "type");
    criteriaCombo->addItem(QString(), "subtype");
    criteriaCombo->addItem(QString(), "cmc");
    controlLayout->addWidget(criteriaCombo);

    exactnessCombo = new QComboBox(this);
    exactnessCombo->addItem(QString(), true);  // At least
    exactnessCombo->addItem(QString(), false); // Exactly
    controlLayout->addWidget(exactnessCombo);

    quantitySpin = new QSpinBox(this);
    quantitySpin->setRange(1, 60);
    controlLayout->addWidget(quantitySpin);

    labelMiddle = new QLabel(this);
    controlLayout->addWidget(labelMiddle);

    drawnSpin = new QSpinBox(this);
    drawnSpin->setRange(1, 60);
    drawnSpin->setValue(7);
    controlLayout->addWidget(drawnSpin);

    labelSuffix = new QLabel(this);
    controlLayout->addWidget(labelSuffix);

    labelPrefix->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    labelMiddle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    labelSuffix->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    controlLayout->addStretch(1);
    layout->addWidget(controls);

    // Table
    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(3);
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    labelMiddle->setText(tr("card(s) having drawn at least"));
    labelSuffix->setText(tr("cards"));

    resultTable->setHorizontalHeaderLabels({tr("Category"), tr("Qty"), tr("Odds (%)")});
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
    if (!analyzer->getModel()->getDeckList()) {
        return;
    }

    const QString criteria = config.criteria;
    const bool atLeast = config.atLeast;
    const int quantity = config.quantity;
    const int drawn = config.drawn;

    QMap<QString, int> categoryCounts;
    int totalDeckCards = 0;

    const auto nodes = analyzer->getModel()->getCardNodes();
    for (auto *node : nodes) {
        CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(node->getName());
        if (!info) {
            continue;
        }

        totalDeckCards += node->getNumber();

        QStringList categories;
        if (criteria == "name") {
            categories << info->getName();
        } else if (criteria == "type") {
            categories = info->getMainCardType().split(' ', Qt::SkipEmptyParts);
        } else if (criteria == "subtype") {
            categories = info->getCardType().split(' ', Qt::SkipEmptyParts);
        } else if (criteria == "cmc") {
            categories << QString::number(info->getCmc().toInt());
        }

        for (const QString &cat : categories) {
            categoryCounts[cat] += node->getNumber();
        }
    }

    resultTable->setRowCount(categoryCounts.size());

    int row = 0;
    for (auto it = categoryCounts.cbegin(); it != categoryCounts.cend(); ++it, ++row) {
        const QString &cat = it.key();
        const int copies = it.value();

        double probability = 0.0;
        if (atLeast) {
            for (int k = quantity; k <= drawn && k <= copies; ++k) {
                probability += hypergeometricProbability(totalDeckCards, copies, drawn, k);
            }
        } else {
            probability = hypergeometricProbability(totalDeckCards, copies, drawn, quantity);
        }

        resultTable->setItem(row, 0, new QTableWidgetItem(cat));
        resultTable->setItem(row, 1, new QTableWidgetItem(QString::number(copies)));
        resultTable->setItem(row, 2, new QTableWidgetItem(QString::number(probability * 100.0, 'f', 2)));
    }
}

double DrawProbabilityWidget::hypergeometricProbability(int N, int K, int n, int k)
{
    if (k < 0 || k > n || K > N || n > N) {
        return 0.0;
    }

    double logP = 0.0;
    for (int i = 1; i <= k; ++i) {
        logP += qLn(double(K - k + i) / i);
    }
    for (int i = 1; i <= n - k; ++i) {
        logP += qLn(double(N - K - (n - k) + i) / i);
    }
    for (int i = 1; i <= n; ++i) {
        logP -= qLn(double(N - n + i) / i);
    }

    return qExp(logP);
}
