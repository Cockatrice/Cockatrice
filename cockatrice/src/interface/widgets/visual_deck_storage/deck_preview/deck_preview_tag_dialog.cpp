#include "deck_preview_tag_dialog.h"

#include "../../../../interface/widgets/dialogs/dlg_default_tags_editor.h"
#include "deck_preview_tag_item_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <libcockatrice/settings/cache_settings.h>

DeckPreviewTagDialog::DeckPreviewTagDialog(const QStringList &knownTags,
                                           const QStringList &_activeTags,
                                           QWidget *parent)
    : QDialog(parent), activeTags(_activeTags), knownTags_(knownTags)
{
    resize(400, 500);

    QStringList defaultTags = SettingsCache::instance().getVisualDeckStorageDefaultTagsList();

    // Merge knownTags with defaultTags, ensuring no duplicates
    QStringList combinedTags = defaultTags + knownTags + activeTags;
    combinedTags.removeDuplicates();

    // Main layout
    mainLayout = new QVBoxLayout(this);

    // Filter bar
    filterInput = new QLineEdit(this);
    mainLayout->addWidget(filterInput);

    connect(filterInput, &QLineEdit::textChanged, this, &DeckPreviewTagDialog::filterTags);

    // Instruction label
    instructionLabel = new QLabel(this);
    instructionLabel->setWordWrap(true);
    mainLayout->addWidget(instructionLabel);

    // Tag list view
    tagListView = new QListWidget(this);
    mainLayout->addWidget(tagListView);

    // Populate combined tags
    for (const auto &tag : combinedTags) {
        auto *item = new QListWidgetItem(tagListView);
        auto *tagWidget = new DeckPreviewTagItemWidget(tag, activeTags.contains(tag), this);
        tagListView->addItem(item);
        tagListView->setItemWidget(item, tagWidget);

        connect(tagWidget->checkBox(), &QCheckBox::toggled, this, &DeckPreviewTagDialog::onCheckboxStateChanged);
    }

    // Add tag input layout
    auto *addTagLayout = new QHBoxLayout();
    newTagInput = new QLineEdit(this);
    addTagButton = new QPushButton(this);
    editButton = new QPushButton(this);
    connect(editButton, &QPushButton::clicked, this, [this]() {
        auto *editor = new DlgDefaultTagsEditor(this);
        editor->setAttribute(Qt::WA_DeleteOnClose);
        editor->setModal(true);
        editor->show();
        QTimer::singleShot(0, editor, [editor]() {
            editor->raise();
            editor->activateWindow();
        });
    });
    addTagLayout->addWidget(newTagInput);
    addTagLayout->addWidget(addTagButton);
    addTagLayout->addWidget(editButton);
    mainLayout->addLayout(addTagLayout);

    connect(addTagButton, &QPushButton::clicked, this, &DeckPreviewTagDialog::addTag);
    connect(newTagInput, &QLineEdit::textChanged, this,
            [this](const QString &text) { addTagButton->setEnabled(!text.trimmed().isEmpty()); });

    // OK and Cancel buttons
    buttonLayout = new QHBoxLayout(this);
    okButton = new QPushButton(this);
    cancelButton = new QPushButton(this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &DeckPreviewTagDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &DeckPreviewTagDialog::reject);

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageDefaultTagsListChanged, this,
            &DeckPreviewTagDialog::refreshTagList);

    retranslateUi();
}

void DeckPreviewTagDialog::retranslateUi()
{
    setWindowTitle(tr("Deck Tags Manager"));
    instructionLabel->setText(tr("Manage your deck tags. Check or uncheck tags as needed, or add new ones:"));
    newTagInput->setPlaceholderText(tr("Add a new tag (e.g., Aggro️)"));
    addTagButton->setText(tr("Add Tag"));
    filterInput->setPlaceholderText(tr("Filter tags..."));
    okButton->setText(tr("OK"));
    editButton->setText(tr("Edit default tags"));
    cancelButton->setText(tr("Cancel"));
}

void DeckPreviewTagDialog::refreshTagList()
{
    // First, clear the current tags in the list view
    tagListView->clear();

    // Get the updated list of tags from SettingsCache
    QStringList defaultTags = SettingsCache::instance().getVisualDeckStorageDefaultTagsList();
    QStringList combinedTags = defaultTags + knownTags_ + activeTags;
    combinedTags.removeDuplicates();

    // Re-populate the tag list view
    for (const auto &tag : combinedTags) {
        auto *item = new QListWidgetItem(tagListView);
        auto *tagWidget = new DeckPreviewTagItemWidget(tag, activeTags.contains(tag), this);
        tagListView->addItem(item);
        tagListView->setItemWidget(item, tagWidget);

        connect(tagWidget->checkBox(), &QCheckBox::toggled, this, &DeckPreviewTagDialog::onCheckboxStateChanged);
    }
}

QStringList DeckPreviewTagDialog::getActiveTags() const
{
    return activeTags;
}

void DeckPreviewTagDialog::addTag()
{
    QString newTag = newTagInput->text().trimmed();
    if (newTag.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"), tr("Tag name cannot be empty!"));
        return;
    }

    // Prevent duplicate tags
    for (int i = 0; i < tagListView->count(); ++i) {
        auto *item = tagListView->item(i);
        auto *tagWidget = qobject_cast<DeckPreviewTagItemWidget *>(tagListView->itemWidget(item));
        if (tagWidget && tagWidget->checkBox()->text() == newTag) {
            QMessageBox::warning(this, tr("Duplicate Tag"), tr("This tag already exists."));
            return;
        }
    }

    // Add the new tag
    auto *item = new QListWidgetItem(tagListView);
    auto *tagWidget = new DeckPreviewTagItemWidget(newTag, true, this);
    tagListView->addItem(item);
    tagListView->setItemWidget(item, tagWidget);
    activeTags.append(newTag);

    connect(tagWidget->checkBox(), &QCheckBox::toggled, this, &DeckPreviewTagDialog::onCheckboxStateChanged);

    // Clear the input field
    newTagInput->clear();
}

void DeckPreviewTagDialog::filterTags(const QString &text)
{
    for (int i = 0; i < tagListView->count(); ++i) {
        auto *item = tagListView->item(i);
        auto *tagWidget = qobject_cast<DeckPreviewTagItemWidget *>(tagListView->itemWidget(item));
        if (tagWidget) {
            bool matches = tagWidget->checkBox()->text().contains(text, Qt::CaseInsensitive);
            item->setHidden(!matches);
        }
    }
}

void DeckPreviewTagDialog::onCheckboxStateChanged()
{
    activeTags.clear();
    for (int i = 0; i < tagListView->count(); ++i) {
        auto *item = tagListView->item(i);
        auto *tagWidget = qobject_cast<DeckPreviewTagItemWidget *>(tagListView->itemWidget(item));
        if (tagWidget && tagWidget->checkBox()->isChecked()) {
            activeTags.append(tagWidget->checkBox()->text());
        }
    }
}
