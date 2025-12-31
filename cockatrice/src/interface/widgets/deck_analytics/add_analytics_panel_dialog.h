
#ifndef COCKATRICE_ADD_ANALYTICS_PANEL_DIALOG_H
#define COCKATRICE_ADD_ANALYTICS_PANEL_DIALOG_H

#include "analytics_panel_widget_factory.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class AddAnalyticsPanelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddAnalyticsPanelDialog(QWidget *parent);

    QString selectedType() const
    {
        return typeCombo->currentData().toString();
    }

private:
    QVBoxLayout *layout;
    QComboBox *typeCombo;
    QDialogButtonBox *buttons;
};

#endif // COCKATRICE_ADD_ANALYTICS_PANEL_DIALOG_H
