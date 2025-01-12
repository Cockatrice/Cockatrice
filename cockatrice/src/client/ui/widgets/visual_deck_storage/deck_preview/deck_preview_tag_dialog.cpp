#include "deck_preview_tag_dialog.h"

#include "deck_preview_tag_item_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

DeckPreviewTagDialog::DeckPreviewTagDialog(const QStringList &knownTags, const QStringList &activeTags, QWidget *parent)
    : QDialog(parent), activeTags_(activeTags)
{
    setWindowTitle("Deck Tags Manager");
    resize(400, 500);

    QStringList defaultTags = {
        // Strategies
        "🏃️ Aggro",
        "🧙‍️ Control",
        "⚔️ Midrange",
        "🌀 Combo",
        "🪓 Mill",
        "🔒 Stax",
        "🗺️ Landfall",
        "🛡️ Pillowfort",
        "🌱 Ramp",
        "⚡ Storm",
        "💀 Aristocrats",
        "☠️ Reanimator",
        "👹 Sacrifice",
        "🔥 Burn",
        "🌟 Lifegain",
        "🔮 Spellslinger",
        "👥 Tokens",
        "🎭 Blink",
        "⏳ Time Manipulation",
        "🌍 Domain",
        "💫 Proliferate",
        "📜 Saga",
        "🎲 Chaos",
        "🪄 Auras",
        "🔫 Pingers",

        // Themes
        "👑 Monarch",
        "🚀 Vehicles",
        "💉 Infect",
        "🩸 Madness",
        "🌀 Morph",

        // Card Types
        "⚔️ Creature",
        "💎 Artifact",
        "🌔 Enchantment",
        "📖 Sorcery",
        "⚡ Instant",
        "🌌 Planeswalker",
        "🌏 Land",
        "🪄 Aura",

        // Kindred Types
        "🐉 Kindred",
        "🐠 Merfolk",
        "🐺 Werewolves",
        "🦴 Skeletons",
        "🦅 Birds",
        "👾 Eldrazi",
        "🐉 Dragons",
        "🧛 Vampires",
        "🦇 Bats",
        "🐺 Wolves",
        "🍄 Fungus",
        "🦗 Insects",
        "🐚 Sea Creatures",
        "🎩 Wizards",
        "🎻 Bards",
        "😇 Angels",
        "⚔️ Soldiers",
        "🛡️ Knights",
        "🧙 Humans",
        "🧝 Elves",
        "💀 Zombies",
        "👹 Demons",
        "🌲 Dryads",
        "🐗 Boars",
        "🦁 Cats",
        "🦊 Foxes",
        "🐀 Rats",
        "🦄 Unicorns",
        "🐘 Elephants",
        "🐻 Bears",
        "🦏 Rhinos",
        "🦂 Scorpions",
    };

    // Merge knownTags with defaultTags, ensuring no duplicates
    QStringList combinedTags = knownTags + defaultTags + activeTags;
    combinedTags.removeDuplicates();

    // Main layout
    auto *mainLayout = new QVBoxLayout(this);

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
    addTagLayout->addWidget(newTagInput_);
    addTagLayout->addWidget(addTagButton_);
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
    retranslateUi();
}

void DeckPreviewTagDialog::retranslateUi()
{
    instructionLabel->setText(tr("Manage your deck tags. Check or uncheck tags as needed, or add new ones:"));
    newTagInput_->setPlaceholderText(tr("Add a new tag (e.g., Aggro️)"));
    addTagButton_->setText(tr("Add Tag"));
    okButton->setText(tr("OK"));
    cancelButton->setText(tr("Cancel"));
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
