#include "add_analytics_panel_dialog.h"

#include "analytics_panel_widget_factory.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

AddAnalyticsPanelDialog::AddAnalyticsPanelDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Add Analytics Panel"));

    layout = new QVBoxLayout(this);

    typeCombo = new QComboBox(this);

    // Populate using descriptors
    const auto widgets = AnalyticsPanelWidgetFactory::instance().availableWidgets();

    for (const auto &desc : widgets) {
        // Show translated title to user
        typeCombo->addItem(desc.title, desc.type);
    }

    layout->addWidget(typeCombo);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
