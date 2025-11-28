#include "dlg_default_tags_editor.h"

#include "../../../client/settings/cache_settings.h"

#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

DlgDefaultTagsEditor::DlgDefaultTagsEditor(QWidget *parent) : QDialog(parent)
{
    mainLayout = new QVBoxLayout(this);

    // Input field + Add button (horizontal layout)
    inputLayout = new QHBoxLayout();
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
    buttonLayout = new QHBoxLayout();
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
        auto *item = new QListWidgetItem(); // Create item but don't insert yet

        QWidget *widget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(5);

        QLineEdit *lineEdit = new QLineEdit(tag, this);
        lineEdit->setFrame(false);
        lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(lineEdit);

        QPushButton *deleteButton = new QPushButton(tr("✖"), this);
        deleteButton->setFixedSize(25, 25);
        layout->addWidget(deleteButton);

        widget->setLayout(layout);

        // Set item height explicitly to match the widget (widgets get squished without it)
        item->setSizeHint(widget->sizeHint());

        listWidget->addItem(item);
        listWidget->setItemWidget(item, widget);

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
