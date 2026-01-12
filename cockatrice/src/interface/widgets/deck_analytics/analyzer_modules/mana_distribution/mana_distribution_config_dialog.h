#ifndef COCKATRICE_MANA_DISTRIBUTION_ADD_DIALOG_H
#define COCKATRICE_MANA_DISTRIBUTION_ADD_DIALOG_H

#include "mana_distribution_config.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QStringList>

class QComboBox;
class QListWidget;
class QCheckBox;
class DeckListStatisticsAnalyzer;

class ManaDistributionConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ManaDistributionConfigDialog(DeckListStatisticsAnalyzer *analyzer, QWidget *parent = nullptr);
    void retranslateUi();

    void setFromConfig(const ManaDistributionConfig &cfg);
    const ManaDistributionConfig &config() const
    {
        return cfg;
    }

public slots:
    void accept() override;

private:
    DeckListStatisticsAnalyzer *analyzer;

    QLabel *labelDisplayType;
    QComboBox *displayType;
    QLabel *labelFilters;
    QListWidget *filterList;
    QCheckBox *showColorRows;
    QDialogButtonBox *buttons;

    ManaDistributionConfig cfg;
};

#endif // COCKATRICE_MANA_DISTRIBUTION_ADD_DIALOG_H
