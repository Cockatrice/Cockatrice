
#ifndef COCKATRICE_MANA_DEVOTION_ADD_DIALOG_H
#define COCKATRICE_MANA_DEVOTION_ADD_DIALOG_H

#include "../../deck_list_statistics_analyzer.h"
#include "mana_devotion_config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

class ManaDevotionConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ManaDevotionConfigDialog(DeckListStatisticsAnalyzer *analyzer,
                             ManaDevotionConfig initial = {},
                             QWidget *parent = nullptr);
    void retranslateUi();

    void accept() override;

    ManaDevotionConfig result() const
    {
        return config;
    }

private:
    ManaDevotionConfig config;
    QVBoxLayout *layout;
    QLabel *labelDisplayType;
    QComboBox *displayType;
    QLabel *labelFilters;
    QListWidget *filterList;
    QDialogButtonBox *buttons;
};

#endif // COCKATRICE_MANA_DEVOTION_ADD_DIALOG_H
