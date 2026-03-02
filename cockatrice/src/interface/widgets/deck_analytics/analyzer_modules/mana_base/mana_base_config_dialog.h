
#ifndef COCKATRICE_MANA_BASE_ADD_DIALOG_H
#define COCKATRICE_MANA_BASE_ADD_DIALOG_H

#include "../../deck_list_statistics_analyzer.h"
#include "mana_base_config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

class ManaBaseConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ManaBaseConfigDialog(DeckListStatisticsAnalyzer *analyzer, ManaBaseConfig initial = {}, QWidget *parent = nullptr);
    void retranslateUi();

    void accept() override;

    ManaBaseConfig result() const
    {
        return config;
    }

private:
    ManaBaseConfig config;
    QVBoxLayout *layout;
    QLabel *displayTypeLabel;
    QComboBox *displayType;
    QLabel *filterLabel;
    QListWidget *filterList;
    QDialogButtonBox *buttons;
};

#endif // COCKATRICE_MANA_BASE_ADD_DIALOG_H
