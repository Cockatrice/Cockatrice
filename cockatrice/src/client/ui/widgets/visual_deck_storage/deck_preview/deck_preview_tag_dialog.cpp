#include "deck_preview_tag_dialog.h"

#include "../../../../../dialogs/dlg_default_tags_editor.h"
#include "../../../../../settings/cache_settings.h"
#include "deck_preview_tag_item_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

DeckPreviewTagDialog::DeckPreviewTagDialog(const QStringList &knownTags, const QStringList &activeTags, QWidget *parent)
    : QDialog(parent), activeTags_(activeTags), knownTags_(knownTags)
{
    resize(400, 500);

    QStringList defaultTags = SettingsCache::instance().getVisualDeckStorageDefaultTagsList();

    // Merge knownTags with defaultTags, ensuring no duplicates
    QStringList combinedTags = defaultTags + knownTags + activeTags;
    combinedTags.removeDuplicates();

    // Main layout
    auto *mainLayout = new QVBoxLayout(this);

    // Filter bar
    filterInput_ = new QLineEdit(this);
    mainLayout->addWidget(filterInput_);

    connect(filterInput_, &QLineEdit::textChanged, this, &DeckPreviewTagDialog::filterTags);

    // Instruction label
    instructionLabel = new QLabel(this);
    instructionLabel->setWordWrap(true);
    mainLayout->addWidget(instructionLabel);

    // Tag list view
    tagListView_ = new QListWidget(this);
    mainLayout->addWidget(tagListView_);

    // Populate combined tags
    for (const auto &tag : combinedTags) {
        auto *item = new QListWidgetItem(tagListView_);
        auto *tagWidget = new DeckPreviewTagItemWidget(tag, activeTags.contains(tag), this);
        tagListView_->addItem(item);
        tagListView_->setItemWidget(item, tagWidget);

        connect(tagWidget->checkBox(), &QCheckBox::toggled, this, &DeckPreviewTagDialog::onCheckboxStateChanged);
    }

    // Add tag input layout
    auto *addTagLayout = new QHBoxLayout();
    newTagInput_ = new QLineEdit(this);
    addTagButton_ = new QPushButton(this);
    editButton = new QPushButton(this);
    connect(editButton, &QPushButton::clicked, this, [=]() {
        auto defaultTagsEditor = new DlgDefaultTagsEditor(nullptr);
        defaultTagsEditor->exec();
    });
    addTagLayout->addWidget(newTagInput_);
    addTagLayout->addWidget(addTagButton_);
    addTagLayout->addWidget(editButton);
    mainLayout->addLayout(addTagLayout);

    connect(addTagButton_, &QPushButton::clicked, this, &DeckPreviewTagDialog::addTag);

    // OK and Cancel buttons
    auto *buttonLayout = new QHBoxLayout();
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
    newTagInput_->setPlaceholderText(tr("Add a new tag (e.g., Aggro️)"));
    addTagButton_->setText(tr("Add Tag"));
    filterInput_->setPlaceholderText(tr("Filter tags..."));
    okButton->setText(tr("OK"));
    editButton->setText(tr("Edit default tags"));
    cancelButton->setText(tr("Cancel"));
}

void DeckPreviewTagDialog::refreshTagList()
{
    // First, clear the current tags in the list view
    tagListView_->clear();

    // Get the updated list of tags from SettingsCache
    QStringList defaultTags = SettingsCache::instance().getVisualDeckStorageDefaultTagsList();
    QStringList combinedTags = defaultTags + knownTags_ + activeTags_;
    combinedTags.removeDuplicates();

    // Re-populate the tag list view
    for (const auto &tag : combinedTags) {
        auto *item = new QListWidgetItem(tagListView_);
        auto *tagWidget = new DeckPreviewTagItemWidget(tag, activeTags_.contains(tag), this);
        tagListView_->addItem(item);
        tagListView_->setItemWidget(item, tagWidget);

        connect(tagWidget->checkBox(), &QCheckBox::toggled, this, &DeckPreviewTagDialog::onCheckboxStateChanged);
    }
}

QStringList DeckPreviewTagDialog::getActiveTags() const
{
    return activeTags_;
}

void DeckPreviewTagDialog::addTag()
{
    QString newTag = newTagInput_->text().trimmed();
    if (newTag.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"), tr("Tag name cannot be empty!"));
        return;
    }

    // Prevent duplicate tags
    for (int i = 0; i < tagListView_->count(); ++i) {
        auto *item = tagListView_->item(i);
        auto *tagWidget = qobject_cast<DeckPreviewTagItemWidget *>(tagListView_->itemWidget(item));
        if (tagWidget && tagWidget->checkBox()->text() == newTag) {
            QMessageBox::warning(this, tr("Duplicate Tag"), tr("This tag already exists."));
            return;
        }
    }

    // Add the new tag
    auto *item = new QListWidgetItem(tagListView_);
    auto *tagWidget = new DeckPreviewTagItemWidget(newTag, true, this);
    tagListView_->addItem(item);
    tagListView_->setItemWidget(item, tagWidget);
    activeTags_.append(newTag);

    connect(tagWidget->checkBox(), &QCheckBox::toggled, this, &DeckPreviewTagDialog::onCheckboxStateChanged);

    // Clear the input field
    newTagInput_->clear();
}

void DeckPreviewTagDialog::filterTags(const QString &text)
{
    for (int i = 0; i < tagListView_->count(); ++i) {
        auto *item = tagListView_->item(i);
        auto *tagWidget = qobject_cast<DeckPreviewTagItemWidget *>(tagListView_->itemWidget(item));
        if (tagWidget) {
            bool matches = tagWidget->checkBox()->text().contains(text, Qt::CaseInsensitive);
            item->setHidden(!matches);
        }
    }
}

void DeckPreviewTagDialog::onCheckboxStateChanged()
{
    activeTags_.clear();
    for (int i = 0; i < tagListView_->count(); ++i) {
        auto *item = tagListView_->item(i);
        auto *tagWidget = qobject_cast<DeckPreviewTagItemWidget *>(tagListView_->itemWidget(item));
        if (tagWidget && tagWidget->checkBox()->isChecked()) {
            activeTags_.append(tagWidget->checkBox()->text());
        }
    }
}
