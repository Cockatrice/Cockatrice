#include "abstract_analytics_panel_widget.h"

#include "deck_list_statistics_analyzer.h"

#include <QPushButton>

AbstractAnalyticsPanelWidget::AbstractAnalyticsPanelWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer)
    : QWidget(parent), analyzer(analyzer)
{
    layout = new QVBoxLayout(this);

    bannerAndSettingsContainer = new QWidget(this);

    bannerAndSettingsLayout = new QHBoxLayout(bannerAndSettingsContainer);
    bannerAndSettingsContainer->setLayout(bannerAndSettingsLayout);

    // close button
    closeButton = new QPushButton(this);
    closeButton->hide();
    closeButton->setIcon(QPixmap("theme:icons/close"));
    closeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(closeButton, &QPushButton::clicked, this, &AbstractAnalyticsPanelWidget::closed);
    bannerAndSettingsLayout->addWidget(closeButton, 0);

    // banner
    bannerWidget = new BannerWidget(this, "Analytics Widget", Qt::Vertical, 100);
    bannerWidget->setMaximumHeight(100);

    bannerAndSettingsLayout->addWidget(bannerWidget, 1);

    // config button
    configureButton = new QPushButton(this);
    configureButton->setIcon(QPixmap("theme:icons/cogwheel"));
    configureButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(configureButton, &QPushButton::clicked, this, &AbstractAnalyticsPanelWidget::applyConfigFromDialog);
    bannerAndSettingsLayout->addWidget(configureButton, 0);

    layout->addWidget(bannerAndSettingsContainer);

    connect(analyzer, &DeckListStatisticsAnalyzer::statsUpdated, this, &AbstractAnalyticsPanelWidget::updateDisplay);
}

void AbstractAnalyticsPanelWidget::setClosable(bool closable)
{
    closeButton->setHidden(!closable);
}

bool AbstractAnalyticsPanelWidget::applyConfigFromDialog()
{
    QDialog *dlg = createConfigDialog(this);
    if (!dlg) {
        return false;
    }

    bool ok = dlg->exec() == QDialog::Accepted;
    if (ok) {
        // dialog must expose its final config as JSON
        auto newCfg = extractConfigFromDialog(dlg);
        loadConfig(newCfg);
        updateDisplay();
    }
    dlg->deleteLater();
    return ok;
}