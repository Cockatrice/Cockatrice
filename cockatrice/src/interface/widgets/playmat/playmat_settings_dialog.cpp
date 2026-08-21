#include "playmat_settings_dialog.h"

#include "../../card_picture_loader/card_picture_loader.h"
#include "../cards/art_crop_attribution.h"
#include "../utility/completer_utils.h"
#include "card_database_display_model.h"
#include "card_database_model.h"
#include "playmat_preview_widget.h"

#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>

PlaymatSettingsDialog::PlaymatSettingsDialog(const CardRef &initialCard,
                                             const PlaymatParams &initialParams,
                                             QWidget *parent)
    : QDialog(parent), currentCard(initialCard), currentParams(initialParams)
{
    setMinimumWidth(500);
    setupUi();

    // Seed UI from initial values
    if (!initialCard.name.isEmpty()) {
        searchBar->setText(initialCard.name);
        onCardNameChanged(initialCard.name);

        // onCardNameChanged leaves the printing combo on the first printing in
        // the database, which would silently change the deck's stored playmat
        // card on accept. Restore the stored printing when it resolves locally.
        const int storedPrintingIndex = providerComboBox->findData(initialCard.providerId);
        if (storedPrintingIndex != -1) {
            providerComboBox->setCurrentIndex(storedPrintingIndex);
        } else {
            // Stored printing not in the local database: keep it rather than
            // silently substituting the first printing.
            currentCard.providerId = initialCard.providerId;
            reloadPreview();
        }
    }

    retranslateUi();
}

CardRef PlaymatSettingsDialog::card() const
{
    return currentCard;
}

PlaymatParams PlaymatSettingsDialog::params() const
{
    return currentParams;
}

void PlaymatSettingsDialog::initializeSearchBar()
{
    searchBar = new QLineEdit;

    cardDatabaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
    cardDatabaseDisplayModel = new CardDatabaseDisplayModel(this);
    cardDatabaseDisplayModel->setSourceModel(cardDatabaseModel);

    const CardCompleterSetup cardSetup = createCardCompleter(cardDatabaseDisplayModel, this, 15);
    searchModel = cardSetup.searchModel;
    proxyModel = cardSetup.proxyModel;
    completer = cardSetup.completer;
    searchBar->setCompleter(completer);

    connectCardCompleterSearch(searchBar, cardSetup);

    connect(completer, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated), this,
            [this](const QString &completion) {
                if (searchBar->text() != completion) {
                    searchBar->setText(completion);
                    searchBar->setCursorPosition(searchBar->text().length());
                }
                onCardNameChanged(completion);
            });

    connect(searchBar, &QLineEdit::returnPressed, this, [this]() { onCardNameChanged(searchBar->text()); });
}

void PlaymatSettingsDialog::setupUi()
{
    initializeSearchBar();

    providerComboBox = new QComboBox;
    connect(providerComboBox, &QComboBox::currentIndexChanged, this, [this]() {
        currentCard.providerId = providerComboBox->currentData().toString();
        reloadPreview();
    });

    auto *form = new QFormLayout;
    cardNameLabel = new QLabel;
    printingLabel = new QLabel;
    form->addRow(cardNameLabel, searchBar);
    form->addRow(printingLabel, providerComboBox);

    controlsGroup = new QGroupBox;
    controlsGroup->setLayout(form);

    preview = new PlaymatPreviewWidget;
    preview->setParams(currentParams);

    auto *previewLayout = new QVBoxLayout;
    previewLayout->addWidget(preview);
    previewCaptionLabel = new QLabel;
    previewCaptionLabel->setAlignment(Qt::AlignCenter);
    previewCaptionLabel->setWordWrap(true);
    previewLayout->addWidget(previewCaptionLabel);
    previewGroup = new QGroupBox;
    previewGroup->setLayout(previewLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    removeButton = new QPushButton;
    buttons->addButton(removeButton, QDialogButtonBox::ResetRole);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(removeButton, &QPushButton::clicked, this, [this]() {
        currentCard = CardRef{}; // empty signals removal
        accept();
    });

    // The crop surface is the primary control: dragging pans, wheel/keys zoom
    //, editing exactly the same stored parameters the old numeric fields did.
    connect(preview, &PlaymatPreviewWidget::paramsEdited, this,
            [this](const PlaymatParams &edited) { currentParams = edited; });

    // The crop surface leads visually, card selection supports it below.
    auto *root = new QVBoxLayout;
    root->addWidget(previewGroup);
    root->addWidget(controlsGroup);
    root->addWidget(buttons);
    setLayout(root);
}

void PlaymatSettingsDialog::populateProviderCombo(const QString &cardName)
{
    providerComboBox->clear();

    auto card = CardDatabaseManager::query()->getCard({cardName});

    const auto &sets = card.getInfo().getSets();

    for (const auto &printings : sets) {
        for (const auto &p : printings) {
            QString setName = p.getSet()->getLongName();
            QString collector = p.getProperty("num");
            QString uuid = p.getUuid();

            QString label = setName;
            if (!collector.isEmpty()) {
                label += " #" + collector;
            }

            providerComboBox->addItem(label, uuid);
        }
    }
}

void PlaymatSettingsDialog::onCardNameChanged(const QString &name)
{
    if (name.isEmpty()) {
        currentPixmap = QPixmap();
        preview->setPixmap(currentPixmap);
        return;
    }

    const ExactCard card = CardDatabaseManager::query()->getCard({name});
    if (!card) {
        currentPixmap = QPixmap();
        preview->setPixmap(currentPixmap);
        providerComboBox->clear();
        return;
    }

    currentCard.name = name;

    populateProviderCombo(name);

    if (providerComboBox->count() == 0) {
        currentPixmap = QPixmap();
        preview->setPixmap(currentPixmap);
        currentCard.providerId.clear();
        return;
    }

    currentCard.providerId = providerComboBox->currentData().toString();
    reloadPreview();
}

void PlaymatSettingsDialog::reloadPreview()
{
    if (currentCard.name.isEmpty()) {
        return;
    }

    ExactCard card = CardDatabaseManager::query()->getCard({currentCard.name, currentCard.providerId});
    if (!card) {
        return;
    }

    disconnect(pixmapUpdatedConnection);

    QPixmap fullRes;
    CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));

    if (fullRes.isNull()) {
        CardInfo *cardInfo = card.getCardPtr().data();
        if (cardInfo) {
            pixmapUpdatedConnection = connect(cardInfo, &CardInfo::pixmapUpdated, this, [this]() { reloadPreview(); });
        }
        return;
    }

    currentPixmap = fullRes;
    preview->setPixmap(currentPixmap);
    preview->setParams(currentParams);
    preview->setAttribution(buildArtAttribution(card));
}

void PlaymatSettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Playmat Settings"));
    searchBar->setPlaceholderText(tr("Type a card name..."));
    cardNameLabel->setText(tr("Card name:"));
    printingLabel->setText(tr("Printing:"));
    controlsGroup->setTitle(tr("Card"));
    previewGroup->setTitle(tr("Crop"));
    previewCaptionLabel->setText(
        tr("Drag to pan, scroll to zoom, arrow keys nudge, plus and minus zoom, Backspace or Esc restores. "
           "Dimmed strips mark where a wider table crops further."));
    removeButton->setText(tr("Remove Playmat"));
}
