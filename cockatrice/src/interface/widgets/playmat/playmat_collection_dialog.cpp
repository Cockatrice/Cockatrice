#include "playmat_collection_dialog.h"

#include "../../../client/settings/cache_settings.h"
#include "playmat_settings_dialog.h"
#include "playmat_settings_utils.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/settings/interface_settings.h>

PlaymatCollectionDialog::PlaymatCollectionDialog(QWidget *parent) : QDialog(parent)
{
    setMinimumWidth(420);
    setupUi();
    retranslateUi();
}

void PlaymatCollectionDialog::accept()
{
    auto &interfaceSettings = SettingsCache::instance().userInterface();
    interfaceSettings.setPlaymatFallbackList(playmats);
    interfaceSettings.setPlaymatFallbackMode(modeCombo->currentData().toInt());
    QDialog::accept();
}

int PlaymatCollectionDialog::currentRow() const
{
    return playmatList->currentRow();
}

void PlaymatCollectionDialog::setupUi()
{
    auto &interfaceSettings = SettingsCache::instance().userInterface();
    playmats = interfaceSettings.getPlaymatFallbackList();

    playmatList = new QListWidget;
    for (const StoredPlaymat &entry : playmats) {
        playmatList->addItem(entry.name);
    }
    connect(playmatList, &QListWidget::itemSelectionChanged, this, &PlaymatCollectionDialog::selectionChanged);
    connect(playmatList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) { editPlaymat(); });

    addButton = new QPushButton;
    editButton = new QPushButton;
    removeButton = new QPushButton;
    moveUpButton = new QPushButton;
    moveDownButton = new QPushButton;

    connect(addButton, &QPushButton::clicked, this, &PlaymatCollectionDialog::addPlaymat);
    connect(editButton, &QPushButton::clicked, this, &PlaymatCollectionDialog::editPlaymat);
    connect(removeButton, &QPushButton::clicked, this, &PlaymatCollectionDialog::removePlaymat);
    connect(moveUpButton, &QPushButton::clicked, this, &PlaymatCollectionDialog::movePlaymatUp);
    connect(moveDownButton, &QPushButton::clicked, this, &PlaymatCollectionDialog::movePlaymatDown);

    auto *listButtons = new QVBoxLayout;
    listButtons->addWidget(addButton);
    listButtons->addWidget(editButton);
    listButtons->addWidget(removeButton);
    listButtons->addWidget(moveUpButton);
    listButtons->addWidget(moveDownButton);
    listButtons->addStretch();

    auto *listRow = new QHBoxLayout;
    listRow->addWidget(playmatList, 1);
    listRow->addLayout(listButtons);

    modeCombo = new QComboBox;
    modeCombo->addItem(QString(), 0);
    modeCombo->addItem(QString(), 1);
    modeCombo->addItem(QString(), 2);
    const int modeIndex = modeCombo->findData(interfaceSettings.getPlaymatFallbackMode());
    if (modeIndex >= 0) {
        modeCombo->setCurrentIndex(modeIndex);
    }

    auto *modeRow = new QHBoxLayout;
    modeLabel = new QLabel;
    modeLabel->setBuddy(modeCombo);
    modeRow->addWidget(modeLabel);
    modeRow->addWidget(modeCombo, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PlaymatCollectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *root = new QVBoxLayout;
    root->addLayout(listRow);
    root->addLayout(modeRow);
    root->addWidget(buttonBox);
    setLayout(root);

    selectionChanged();
}

void PlaymatCollectionDialog::selectionChanged()
{
    const bool hasSelection = playmatList->currentRow() >= 0;
    editButton->setEnabled(hasSelection);
    removeButton->setEnabled(hasSelection);
    moveUpButton->setEnabled(hasSelection && playmatList->currentRow() > 0);
    moveDownButton->setEnabled(hasSelection && playmatList->currentRow() < playmatList->count() - 1);
}

void PlaymatCollectionDialog::appendEntry(const StoredPlaymat &entry)
{
    playmats.append(entry);
    playmatList->addItem(entry.name);
    playmatList->setCurrentRow(playmatList->count() - 1);
}

void PlaymatCollectionDialog::addPlaymat()
{
    PlaymatSettingsDialog dialog(CardRef{}, PlaymatParams{}, this);
    if (dialog.exec() == QDialog::Accepted) {
        const CardRef card = dialog.card();
        if (!card.isEmpty()) {
            appendEntry(storedFromResolution({card, dialog.params()}));
        }
    }
}

void PlaymatCollectionDialog::editPlaymat()
{
    const int row = currentRow();
    if (row < 0) {
        return;
    }

    const PlaymatResolution current = resolutionFromStoredPlaymat(playmats.at(row));
    PlaymatSettingsDialog dialog(current.card, current.params, this);
    if (dialog.exec() == QDialog::Accepted) {
        const CardRef card = dialog.card();
        if (card.isEmpty()) {
            return; // Removal is handled by the Remove button
        }
        playmats[row] = storedFromResolution({card, dialog.params()});
        playmatList->item(row)->setText(card.name);
    }
}

void PlaymatCollectionDialog::removePlaymat()
{
    const int row = currentRow();
    if (row < 0) {
        return;
    }
    playmats.removeAt(row);
    delete playmatList->takeItem(row);
    selectionChanged();
}

void PlaymatCollectionDialog::movePlaymatUp()
{
    const int row = currentRow();
    if (row <= 0) {
        return;
    }
    playmats.swapItemsAt(row, row - 1);
    playmatList->insertItem(row - 1, playmatList->takeItem(row));
    playmatList->setCurrentRow(row - 1);
    selectionChanged();
}

void PlaymatCollectionDialog::movePlaymatDown()
{
    const int row = currentRow();
    if (row < 0 || row >= playmats.size() - 1) {
        return;
    }
    playmats.swapItemsAt(row, row + 1);
    playmatList->insertItem(row + 1, playmatList->takeItem(row));
    playmatList->setCurrentRow(row + 1);
    selectionChanged();
}

void PlaymatCollectionDialog::retranslateUi()
{
    setWindowTitle(tr("Default Playmats"));
    addButton->setText(tr("Add..."));
    editButton->setText(tr("Edit..."));
    removeButton->setText(tr("Remove"));
    moveUpButton->setText(tr("Move Up"));
    moveDownButton->setText(tr("Move Down"));
    modeLabel->setText(tr("When a deck has no playmat:"));
    modeCombo->setItemText(0, tr("Always use the first playmat"));
    modeCombo->setItemText(1, tr("Cycle through playmats (one per game)"));
    modeCombo->setItemText(2, tr("Pick a random playmat per game"));
}
