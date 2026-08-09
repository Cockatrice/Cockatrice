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
#include <QDoubleSpinBox>
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
    setWindowTitle(tr("Playmat Settings"));
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
    marginLSpin->setValue(initialParams.marginPctL);
    marginRSpin->setValue(initialParams.marginPctR);
    verticalOffsetSpin->setValue(initialParams.verticalOffset);
    zoomSpin->setValue(initialParams.zoom);
}

CardRef PlaymatSettingsDialog::card() const
{
    return currentCard;
}

PlaymatParams PlaymatSettingsDialog::params() const
{
    return currentParams;
}

QDoubleSpinBox *PlaymatSettingsDialog::makeSpinBox(double min, double max, double value, double step)
{
    auto *spin = new QDoubleSpinBox;
    spin->setRange(min, max);
    spin->setSingleStep(step);
    spin->setDecimals(3);
    spin->setValue(value);
    return spin;
}

void PlaymatSettingsDialog::initializeSearchBar()
{
    searchBar = new QLineEdit;
    searchBar->setPlaceholderText(tr("Type a card name..."));

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
        onParamChanged();
    });

    marginLSpin = makeSpinBox(0.0, 0.95, currentParams.marginPctL, 0.01);
    marginRSpin = makeSpinBox(0.0, 0.95, currentParams.marginPctR, 0.01);
    verticalOffsetSpin = makeSpinBox(0.0, 1.0, currentParams.verticalOffset, 0.01);
    zoomSpin = makeSpinBox(0.1, 4.0, currentParams.zoom, 0.05);

    auto *form = new QFormLayout;
    form->addRow(tr("Card name:"), searchBar);
    form->addRow(tr("Printing:"), providerComboBox);
    form->addRow(tr("Left margin (%):"), marginLSpin);
    form->addRow(tr("Right margin (%):"), marginRSpin);
    form->addRow(tr("Vertical offset:"), verticalOffsetSpin);
    form->addRow(tr("Zoom:"), zoomSpin);

    auto *controlsGroup = new QGroupBox(tr("Parameters"));
    controlsGroup->setLayout(form);

    preview = new PlaymatPreviewWidget;

    auto *previewLayout = new QVBoxLayout;
    previewLayout->addWidget(preview);
    auto *previewGroup = new QGroupBox(tr("Preview"));
    previewGroup->setLayout(previewLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto *removeBtn = new QPushButton(tr("Remove Playmat"));
    buttons->addButton(removeBtn, QDialogButtonBox::ResetRole);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        currentCard = CardRef{}; // empty signals removal
        accept();
    });

    auto *root = new QVBoxLayout;
    root->addWidget(controlsGroup);
    root->addWidget(previewGroup);
    root->addWidget(buttons);
    setLayout(root);

    connect(marginLSpin, &QDoubleSpinBox::valueChanged, this, &PlaymatSettingsDialog::onParamChanged);
    connect(marginRSpin, &QDoubleSpinBox::valueChanged, this, &PlaymatSettingsDialog::onParamChanged);
    connect(verticalOffsetSpin, &QDoubleSpinBox::valueChanged, this, &PlaymatSettingsDialog::onParamChanged);
    connect(zoomSpin, &QDoubleSpinBox::valueChanged, this, &PlaymatSettingsDialog::onParamChanged);
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

void PlaymatSettingsDialog::onParamChanged()
{
    currentParams.marginPctL = marginLSpin->value();
    currentParams.marginPctR = marginRSpin->value();
    currentParams.verticalOffset = verticalOffsetSpin->value();
    currentParams.zoom = zoomSpin->value();
    preview->setParams(currentParams);
}
