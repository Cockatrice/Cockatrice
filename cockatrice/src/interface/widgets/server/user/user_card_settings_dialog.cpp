#include "user_card_settings_dialog.h"

#include "../../../card_picture_loader/card_picture_loader.h"
#include "card/card_completer_proxy_model.h"
#include "card/card_search_model.h"
#include "card_database_display_model.h"
#include "card_database_model.h"
#include "user_card_art_provider.h"
#include "user_list_painter.h"

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
#include <QRegularExpression>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>

CardArtPreviewWidget::CardArtPreviewWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(400, 72);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void CardArtPreviewWidget::setPixmap(const QPixmap &pixmap)
{
    sourcePixmap = pixmap;
    update();
}

void CardArtPreviewWidget::setParams(const CardArtParams &p)
{
    params = p;
    update();
}

void CardArtPreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const QRect rect = this->rect();

    const QColor accentColor(100, 116, 139);
    const QRectF cardRect = QRectF(rect).adjusted(3, 2, -3, -2);

    QLinearGradient bg(cardRect.topLeft(), cardRect.topRight());
    bg.setColorAt(0, accentColor.darker(320));
    bg.setColorAt(1, QColor(18, 22, 30));
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);
    painter.drawRoundedRect(cardRect, 6, 6);
    painter.setBrush(accentColor);
    painter.drawRoundedRect(QRectF(cardRect.left(), cardRect.top(), 3, cardRect.height()), 2, 2);

    if (sourcePixmap.isNull()) {
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(rect, Qt::AlignCenter, tr("No card selected"));
        return;
    }

    UserListPainter::drawCardArt(&painter, rect, rect.right() - 4,
                                 QString(), // userName not needed for override path
                                 nullptr,   // no cache
                                 params,
                                 &sourcePixmap // direct pixmap
    );

    // Avatar placeholder so the left-margin interaction is visible
    const int avatarX = rect.left() + 14;
    const int avatarY = rect.top() + (rect.height() - 36) / 2;
    const QRect avatarRect(avatarX, avatarY, 36, 36);

    QPainterPath clip;
    clip.addEllipse(avatarRect);
    painter.save();
    painter.setClipPath(clip);
    painter.setBrush(accentColor.darker(200));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(avatarRect);
    painter.restore();

    painter.setPen(QPen(QColor(70, 80, 95), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(avatarRect.adjusted(-1, -1, 1, 1));
}

UserCardArtSettingsDialog::UserCardArtSettingsDialog(const CardArtParams &initial, QWidget *parent)
    : QDialog(parent), currentParams(initial)
{
    setWindowTitle(tr("Card Art Settings"));
    setMinimumWidth(500);
    setupUi();

    // Seed UI from initial params
    if (!initial.cardName.isEmpty()) {
        searchBar->setText(initial.cardName);
        onCardNameChanged(initial.cardName);
    }
    marginLSpin->setValue(initial.marginPctL);
    marginRSpin->setValue(initial.marginPctR);
    verticalOffsetSpin->setValue(initial.verticalOffset);
    zoomSpin->setValue(initial.zoom);
}

CardArtParams UserCardArtSettingsDialog::params() const
{
    return currentParams;
}

QDoubleSpinBox *UserCardArtSettingsDialog::makeSpinBox(double min, double max, double value, double step)
{
    auto *spin = new QDoubleSpinBox;
    spin->setRange(min, max);
    spin->setSingleStep(step);
    spin->setDecimals(3);
    spin->setValue(value);
    return spin;
}

void UserCardArtSettingsDialog::initializeSearchBar()
{
    searchBar = new QLineEdit;
    searchBar->setPlaceholderText(tr("Type a card name..."));

    cardDatabaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
    cardDatabaseDisplayModel = new CardDatabaseDisplayModel(this);
    cardDatabaseDisplayModel->setSourceModel(cardDatabaseModel);
    searchModel = new CardSearchModel(cardDatabaseDisplayModel, this);

    proxyModel = new CardCompleterProxyModel(this);
    proxyModel->setSourceModel(searchModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(Qt::DisplayRole);

    completer = new QCompleter(proxyModel, this);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setMaxVisibleItems(15);
    searchBar->setCompleter(completer);

    connect(searchBar, &QLineEdit::textEdited, searchModel, &CardSearchModel::updateSearchResults);
    connect(searchBar, &QLineEdit::textEdited, this, [this](const QString &text) {
        const QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty()) {
            completer->complete();
        }
    });

    connect(completer, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated), this,
            [this](const QString &completion) {
                if (searchBar->text() != completion) {
                    searchBar->setText(completion);
                    searchBar->setCursorPosition(searchBar->text().length());
                }
                onCardNameChanged(completion);
            });

    // Also trigger a load when the user hits Return on a typed name
    connect(searchBar, &QLineEdit::returnPressed, this, [this]() { onCardNameChanged(searchBar->text()); });
}

void UserCardArtSettingsDialog::setupUi()
{
    initializeSearchBar();

    providerComboBox = new QComboBox;
    connect(providerComboBox, &QComboBox::currentIndexChanged, this, [this]() {
        currentParams.cardProviderId = providerComboBox->currentData().toString();
        reloadPreview();
        onParamChanged();
    });

    marginLSpin = makeSpinBox(0.0, 0.95, currentParams.marginPctL, 0.01);
    marginRSpin = makeSpinBox(0.0, 0.95, currentParams.marginPctR, 0.01);
    verticalOffsetSpin = makeSpinBox(0.0, 1.0, currentParams.verticalOffset, 0.01);
    zoomSpin = makeSpinBox(0.1, 4.0, currentParams.zoom, 0.05);

    auto *form = new QFormLayout;
    form->addRow(tr("Card name:"), searchBar);
    form->addRow(tr("Card ProviderId:"), providerComboBox);
    form->addRow(tr("Left margin (%):"), marginLSpin);
    form->addRow(tr("Right margin (%):"), marginRSpin);
    form->addRow(tr("Vertical offset:"), verticalOffsetSpin);
    form->addRow(tr("Zoom:"), zoomSpin);

    auto *controlsGroup = new QGroupBox(tr("Parameters"));
    controlsGroup->setLayout(form);

    preview = new CardArtPreviewWidget;

    auto *previewLayout = new QVBoxLayout;
    previewLayout->addWidget(preview);
    auto *previewGroup = new QGroupBox(tr("Preview"));
    previewGroup->setLayout(previewLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto *removeBtn = new QPushButton(tr("Remove Banner Card"));
    buttons->addButton(removeBtn, QDialogButtonBox::ResetRole);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        currentParams = CardArtParams{}; // empty cardName signals removal
        accept();
    });

    auto *root = new QVBoxLayout;
    root->addWidget(controlsGroup);
    root->addWidget(previewGroup);
    root->addWidget(buttons);
    setLayout(root);

    connect(marginLSpin, &QDoubleSpinBox::valueChanged, this, &UserCardArtSettingsDialog::onParamChanged);
    connect(marginRSpin, &QDoubleSpinBox::valueChanged, this, &UserCardArtSettingsDialog::onParamChanged);
    connect(verticalOffsetSpin, &QDoubleSpinBox::valueChanged, this, &UserCardArtSettingsDialog::onParamChanged);
    connect(zoomSpin, &QDoubleSpinBox::valueChanged, this, &UserCardArtSettingsDialog::onParamChanged);
}

void UserCardArtSettingsDialog::populateProviderCombo(const QString &cardName)
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

void UserCardArtSettingsDialog::onCardNameChanged(const QString &name)
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

    currentParams.cardName = name;

    populateProviderCombo(name);

    if (providerComboBox->count() == 0) {
        // No printings found for this card; nothing to preview.
        currentPixmap = QPixmap();
        preview->setPixmap(currentPixmap);
        currentParams.cardProviderId.clear();
        return;
    }

    currentParams.cardProviderId = providerComboBox->currentData().toString();
    reloadPreview();
}

void UserCardArtSettingsDialog::reloadPreview()
{
    if (currentParams.cardName.isEmpty()) {
        return;
    }

    ExactCard card = CardDatabaseManager::query()->getCard({currentParams.cardName, currentParams.cardProviderId});
    if (!card) {
        return;
    }

    // CardPictureLoader::getPixmap() is async on a cache miss: it enqueues a
    // background download and returns a null pixmap immediately. When that
    // download finishes, CardPictureLoader::imageLoaded() caches the result
    // and calls card.emitPixmapUpdated(), which emits pixmapUpdated() on the
    // underlying CardInfo (see exact_card.h). Listen for that, scoped to
    // whichever CardInfo we just asked for, so the preview catches up once
    // the image actually arrives instead of staying on the placeholder.
    //
    // Disconnect any previous listener first -- otherwise switching cards
    // repeatedly stacks up connections to old CardInfo objects, each of
    // which would still fire reloadPreview() (harmlessly, but wastefully)
    // whenever ITS art finishes loading later.
    disconnect(pixmapUpdatedConnection);

    QPixmap fullRes;
    CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));

    if (fullRes.isNull()) {
        // Not loaded yet -- wait for the signal instead of giving up.
        // card.getCardPtr() is a CardInfoPtr (QSharedPointer<CardInfo>);
        // .data() gives the raw QObject* needed for connect().
        CardInfo *cardInfo = card.getCardPtr().data();
        if (cardInfo) {
            pixmapUpdatedConnection = connect(cardInfo, &CardInfo::pixmapUpdated, this, [this]() { reloadPreview(); });
        }
        return;
    }

    currentPixmap = UserCardArtProvider::cropCardArt(fullRes);
    preview->setPixmap(currentPixmap);
    preview->setParams(currentParams);
}

void UserCardArtSettingsDialog::onParamChanged()
{
    currentParams.marginPctL = marginLSpin->value();
    currentParams.marginPctR = marginRSpin->value();
    currentParams.verticalOffset = verticalOffsetSpin->value();
    currentParams.zoom = zoomSpin->value();
    preview->setParams(currentParams);
}