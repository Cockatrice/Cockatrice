
#ifndef COCKATRICE_MANA_DEVOTION_ADD_DIALOG_H
#define COCKATRICE_MANA_DEVOTION_ADD_DIALOG_H

#include "mana_devotion_config.h"

#include <QDialog>
#include <QList>
#include <QString>
#include <qtmetamacros.h>

class DeckListStatisticsAnalyzer;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QListWidget;
class QVBoxLayout;
class QWidget;

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
