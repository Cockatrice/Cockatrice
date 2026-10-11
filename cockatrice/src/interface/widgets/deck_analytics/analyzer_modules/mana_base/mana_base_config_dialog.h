
#ifndef COCKATRICE_MANA_BASE_ADD_DIALOG_H
#define COCKATRICE_MANA_BASE_ADD_DIALOG_H

#include "mana_base_config.h"

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
