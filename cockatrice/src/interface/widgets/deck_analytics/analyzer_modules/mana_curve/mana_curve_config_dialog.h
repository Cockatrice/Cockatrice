#ifndef COCKATRICE_MANA_CURVE_ADD_DIALOG_H
#define COCKATRICE_MANA_CURVE_ADD_DIALOG_H

#include "mana_curve_config.h"

#include <QDialog>
#include <qtmetamacros.h>

class QListWidget;
class QCheckBox;
class QComboBox;
class DeckListStatisticsAnalyzer;
class QDialogButtonBox;
class QLabel;
class QWidget;

class ManaCurveConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ManaCurveConfigDialog(DeckListStatisticsAnalyzer *analyzer, QWidget *parent = nullptr);
    void retranslateUi();
    void setFromConfig(const ManaCurveConfig &cfg);

    ManaCurveConfig result() const
    {
        return cfg;
    }

private:
    ManaCurveConfig cfg;
    DeckListStatisticsAnalyzer *analyzer;

    QLabel *labelGroupBy;
    QComboBox *groupBy;
    QLabel *labelFilters;
    QListWidget *filterList;
    QDialogButtonBox *buttons;
    QCheckBox *showMain;
    QCheckBox *showCatRows;

private slots:
    void accept() override;
};

#endif // COCKATRICE_MANA_CURVE_ADD_DIALOG_H
