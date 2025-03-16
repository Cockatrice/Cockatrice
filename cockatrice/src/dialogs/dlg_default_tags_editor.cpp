#include "dlg_default_tags_editor.h"

#include "../settings/cache_settings.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

DlgDefaultTagsEditor::DlgDefaultTagsEditor(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::WindowModal); // Steals focus from parent dialog

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Input field + Add button (horizontal layout)
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputField = new QLineEdit(this);
    addButton = new QPushButton(this);
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(addButton);
    mainLayout->addLayout(inputLayout);

    // List widget for tags
    listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    listWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    mainLayout->addWidget(listWidget);

    // Button layout (Confirm and Cancel)
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    confirmButton = new QPushButton(this);
    cancelButton = new QPushButton(this);
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    loadStringList();
    retranslateUi();

    // Connect signals
    connect(addButton, &QPushButton::clicked, this, &DlgDefaultTagsEditor::addItem);
    connect(cancelButton, &QPushButton::clicked, this, &DlgDefaultTagsEditor::reject); // Cancel closes dialog
    connect(confirmButton, &QPushButton::clicked, this, &DlgDefaultTagsEditor::confirmChanges);
    connect(inputField, &QLineEdit::returnPressed, this, &DlgDefaultTagsEditor::addItem);
}

void DlgDefaultTagsEditor::retranslateUi()
{
    setWindowTitle(tr("Edit Tags"));
    addButton->setText(tr("Add"));
    confirmButton->setText(tr("Confirm"));
    cancelButton->setText(tr("Cancel"));
    inputField->setPlaceholderText(tr("Enter a tag and press Enter"));
}

void DlgDefaultTagsEditor::loadStringList()
{
    listWidget->clear();
    QStringList tags = SettingsCache::instance().getVisualDeckStorageDefaultTagsList();
    for (const QString &tag : tags) {
        auto *item = new QListWidgetItem(listWidget);
        QWidget *widget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        // Editable tag label
        QLineEdit *lineEdit = new QLineEdit(tag, this);
        lineEdit->setFrame(false);
        lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        layout->addWidget(lineEdit);

        // Delete button
        QPushButton *deleteButton = new QPushButton(tr("✖"), this);
        deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        deleteButton->setFixedWidth(25);
        layout->addSpacing(5); // Adds spacing before the delete button
        layout->addWidget(deleteButton);

        widget->setLayout(layout);
        listWidget->setItemWidget(item, widget);

        // Delete button signal
        connect(deleteButton, &QPushButton::clicked, this, [this, item]() { deleteItem(item); });
    }
}

void DlgDefaultTagsEditor::addItem()
{
    QString newTag = inputField->text().trimmed();
    if (newTag.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"), tr("Tag name cannot be empty!"));
        return;
    }

    // Prevent duplicate tags
    for (int i = 0; i < listWidget->count(); ++i) {
        QWidget *widget = listWidget->itemWidget(listWidget->item(i));
        if (!widget)
            continue;
        QLineEdit *lineEdit = widget->findChild<QLineEdit *>();
        if (lineEdit && lineEdit->text() == newTag) {
            QMessageBox::warning(this, tr("Duplicate Tag"), tr("This tag already exists."));
            return;
        }
    }

    // Add new tag
    auto *item = new QListWidgetItem(listWidget);
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    // Editable tag label
    QLineEdit *lineEdit = new QLineEdit(newTag, this);
    lineEdit->setFrame(false);
    layout->addWidget(lineEdit);

    // Delete button
    QPushButton *deleteButton = new QPushButton(tr("✖"), this);
    deleteButton->setFixedSize(24, 24);
    layout->addWidget(deleteButton);

    widget->setLayout(layout);
    listWidget->setItemWidget(item, widget);

    connect(deleteButton, &QPushButton::clicked, this, [this, item]() { deleteItem(item); });

    inputField->clear();
}

void DlgDefaultTagsEditor::deleteItem(QListWidgetItem *item)
{
    delete listWidget->takeItem(listWidget->row(item));
}

void DlgDefaultTagsEditor::confirmChanges()
{
    QStringList updatedList;
    for (int i = 0; i < listWidget->count(); ++i) {
        QWidget *widget = listWidget->itemWidget(listWidget->item(i));
        if (!widget)
            continue;
        QLineEdit *lineEdit = widget->findChild<QLineEdit *>();
        if (lineEdit) {
            updatedList.append(lineEdit->text());
        }
    }
    SettingsCache::instance().setVisualDeckStorageDefaultTagsList(updatedList);
    accept(); // Close dialog and confirm changes
}
