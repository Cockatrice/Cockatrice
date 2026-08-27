#include "user_card_settings_dialog.h"

#include "../../../card_picture_loader/card_picture_loader.h"
#include "../../cards/art_crop_attribution.h"
#include "../../utility/completer_utils.h"
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
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <cmath>
#include <libcockatrice/card/database/card_database_manager.h>

namespace
{
// Gesture clamps and step sizes for this direct manipulation surface. The
// gesture zoom floor is 1.0: basescale already cover fits the art, so any
// smaller scale would underfill the strip.
constexpr qreal kMinGestureZoom = 1.0;
constexpr qreal kMaxZoom = 4.0;
constexpr qreal kKeyPanOffsetStep = 0.01;
constexpr qreal kKeyZoomStep = 1.05;
constexpr qreal kWheelZoomBase = 1.15; // zoom factor per wheel notch
} // namespace

CardArtPreviewWidget::CardArtPreviewWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(400, 72);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Banner preview"));
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
}

void CardArtPreviewWidget::focusInEvent(QFocusEvent *event)
{
    // Snapshot for the Esc or Backspace reset, restoring whatever the user
    // had when the surface took focus
    paramsAtFocusIn = params;
    QWidget::focusInEvent(event);
}

void CardArtPreviewWidget::setPixmap(const QPixmap &pixmap)
{
    sourcePixmap = pixmap;
    update();
}

void CardArtPreviewWidget::setParams(const CardArtParams &p)
{
    params = p;
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
    update();
}

void CardArtPreviewWidget::setAttribution(const QString &attribution)
{
    attributionText = attribution;
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

    // Visible keyboard focus per the focus cursor contract, Tab must show
    // where the keys land, including in the empty state
    const auto paintFocusRing = [&painter, &cardRect, this]() {
        if (!hasFocus()) {
            return;
        }
        QPen focusPen(palette().color(QPalette::Highlight), 2);
        painter.setPen(focusPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cardRect.adjusted(-1, -1, 1, 1), 6, 6);
    };

    if (sourcePixmap.isNull()) {
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(rect, Qt::AlignCenter, tr("No card selected"));
        paintFocusRing();
        return;
    }

    UserListPainter::drawCardArt(&painter, rect, rect.right() - 4,
                                 QString(), // userName not needed for override path
                                 nullptr,   // no cache
                                 params,
                                 &sourcePixmap // direct pixmap
    );

    // Avatar placeholder so the left margin interaction is visible
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

    paintArtAttribution(painter, cardRect, attributionText);

    paintFocusRing();
}

qreal CardArtPreviewWidget::bannerTravel() const
{
    if (sourcePixmap.isNull()) {
        return 0.0;
    }
    // Mirror UserListPainter::drawCardArt() exactly: same strip metrics, the
    // copy is drawn 1:1, so output pixels equal widget pixels here.
    const int cardH = rect().height() - 4;
    const int totalW = (rect().right() - 4) - rect().left();
    const int marginL = qRound(totalW * params.marginPctL);
    const int marginR = qRound(totalW * params.marginPctR);
    const int drawW = totalW - marginL - marginR;
    const double basescale = qMax(double(drawW) / sourcePixmap.width(), double(cardH) / sourcePixmap.height());
    // qRound for literal parity with drawCardArt, which rounds the scaled
    // height before computing travel
    const double scaledH = qRound(sourcePixmap.height() * basescale * params.zoom);
    return scaledH - cardH;
}

void CardArtPreviewWidget::applyCropDelta(qreal dOffset, qreal zoomFactor)
{
    CardArtParams next = params;
    next.verticalOffset = qBound(0.0, params.verticalOffset + dOffset, 1.0);
    next.zoom = qBound(kMinGestureZoom, params.zoom * zoomFactor, kMaxZoom);

    if (sameCrop(next, params)) {
        return;
    }

    params = next;
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
    update();
    emit paramsEdited(params);
}

bool CardArtPreviewWidget::sameCrop(const CardArtParams &a, const CardArtParams &b) const
{
    // Exact comparison on purpose: clamped assignments yield identical bits,
    // while qFuzzyCompare based equality misbehaves around zero
    return a.verticalOffset == b.verticalOffset && a.zoom == b.zoom;
}

void CardArtPreviewWidget::restoreSnapshot()
{
    // The snapshot only ever holds values that passed the gesture clamps,
    // so it is safe to restore verbatim
    if (sameCrop(paramsAtFocusIn, params)) {
        return;
    }
    params = paramsAtFocusIn;
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
    update();
    emit paramsEdited(params);
}

void CardArtPreviewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || sourcePixmap.isNull() || bannerTravel() <= 0.5) {
        QWidget::mousePressEvent(event);
        return;
    }
    dragging = true;
    lastDragPos = event->pos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void CardArtPreviewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging || sourcePixmap.isNull()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const qreal dy = event->pos().y() - lastDragPos.y();
    lastDragPos = event->pos();

    const qreal travel = bannerTravel();
    if (travel <= 0.5) {
        event->accept();
        return;
    }

    // Dragging moves the ART with the cursor, so the crop window slides the
    // other way through the available travel.
    applyCropDelta(-dy / travel, 1.0);
    event->accept();
}

void CardArtPreviewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        unsetCursor();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void CardArtPreviewWidget::wheelEvent(QWheelEvent *event)
{
    if (sourcePixmap.isNull()) {
        QWidget::wheelEvent(event);
        return;
    }
    const qreal notches = static_cast<qreal>(event->angleDelta().y()) / 120.0;
    if (notches == 0.0) {
        event->accept();
        return;
    }
    applyCropDelta(0.0, std::pow(kWheelZoomBase, notches));
    event->accept();
}

void CardArtPreviewWidget::keyPressEvent(QKeyEvent *event)
{
    if (sourcePixmap.isNull()) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
        case Qt::Key_Escape:
            if (sameCrop(params, paramsAtFocusIn)) {
                // Nothing to undo on this surface, let the event reach the
                // dialog so Esc keeps its close meaning there
                QWidget::keyPressEvent(event);
                return;
            }
            restoreSnapshot();
            break;
        case Qt::Key_Backspace:
            restoreSnapshot();
            break;
        case Qt::Key_Up:
            applyCropDelta(-kKeyPanOffsetStep, 1.0);
            break;
        case Qt::Key_Down:
            applyCropDelta(kKeyPanOffsetStep, 1.0);
            break;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            applyCropDelta(0.0, kKeyZoomStep);
            break;
        case Qt::Key_Minus:
            applyCropDelta(0.0, 1.0 / kKeyZoomStep);
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
    }
    event->accept();
}

UserCardArtSettingsDialog::UserCardArtSettingsDialog(const CardArtParams &initial, QWidget *parent)
    : QDialog(parent), currentParams(initial)
{
    // Legacy stored banners may carry zoom below the gesture floor or an out
    // of range offset. Normalize once on open so the preview renders filled
    // and Ok saves a state the gestures can reach again
    currentParams.zoom = qBound(kMinGestureZoom, currentParams.zoom, kMaxZoom);
    currentParams.verticalOffset = qBound(0.0, currentParams.verticalOffset, 1.0);

    setMinimumWidth(500);
    setupUi();

    // Seed UI from initial params
    if (!currentParams.cardName.isEmpty()) {
        // onCardNameChanged overwrites cardProviderId with the first printing,
        // so remember the stored one before it runs
        const QString storedProviderId = currentParams.cardProviderId;
        searchBar->setText(currentParams.cardName);
        onCardNameChanged(currentParams.cardName);

        // onCardNameChanged leaves the printing combo on the first printing in
        // the database, which would silently change the stored banner card on
        // accept. Restore the stored printing when it resolves locally.
        const int storedPrintingIndex = providerComboBox->findData(storedProviderId);
        if (storedPrintingIndex != -1) {
            providerComboBox->setCurrentIndex(storedPrintingIndex);
        } else if (!storedProviderId.isEmpty()) {
            // Stored printing not in the local database: keep it rather than
            // silently substituting the first printing.
            currentParams.cardProviderId = storedProviderId;
            reloadPreview();
        }
    }
    marginLSpin->setValue(currentParams.marginPctL);
    marginRSpin->setValue(currentParams.marginPctR);
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

    auto *form = new QFormLayout;
    cardNameLabel = new QLabel;
    printingLabel = new QLabel;
    marginLLabel = new QLabel;
    marginRLabel = new QLabel;
    form->addRow(cardNameLabel, searchBar);
    form->addRow(printingLabel, providerComboBox);
    form->addRow(marginLLabel, marginLSpin);
    form->addRow(marginRLabel, marginRSpin);

    controlsGroup = new QGroupBox;
    controlsGroup->setLayout(form);

    preview = new CardArtPreviewWidget;

    auto *previewLayout = new QVBoxLayout;
    previewLayout->addWidget(preview);
    previewCaptionLabel = new QLabel;
    previewCaptionLabel->setAlignment(Qt::AlignCenter);
    previewCaptionLabel->setWordWrap(true);
    previewLayout->addWidget(previewCaptionLabel);
    previewGroup = new QGroupBox;
    previewGroup->setLayout(previewLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    removeBtn = new QPushButton;
    buttons->addButton(removeBtn, QDialogButtonBox::ResetRole);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        currentParams = CardArtParams{}; // empty cardName signals removal
        accept();
    });

    // The banner leads visually, card selection and margins support it below.
    auto *root = new QVBoxLayout;
    root->addWidget(previewGroup);
    root->addWidget(controlsGroup);
    root->addWidget(buttons);
    setLayout(root);

    connect(marginLSpin, &QDoubleSpinBox::valueChanged, this, &UserCardArtSettingsDialog::onParamChanged);
    connect(marginRSpin, &QDoubleSpinBox::valueChanged, this, &UserCardArtSettingsDialog::onParamChanged);

    // Gestures are the only editors of offset and zoom on this surface.
    // Margins stay explicit numeric controls: they trim the strip's
    // sides and have no natural drag mapping.
    connect(preview, &CardArtPreviewWidget::paramsEdited, this,
            [this](const CardArtParams &edited) { currentParams = edited; });

    retranslateUi();
}

void UserCardArtSettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Card Art Settings"));
    searchBar->setPlaceholderText(tr("Type a card name..."));
    cardNameLabel->setText(tr("Card name:"));
    printingLabel->setText(tr("Printing:"));
    marginLLabel->setText(tr("Left margin (%):"));
    marginRLabel->setText(tr("Right margin (%):"));
    controlsGroup->setTitle(tr("Card"));
    previewCaptionLabel->setText(
        tr("Drag to pan, scroll to zoom, arrow keys nudge, plus and minus zoom, Backspace or Esc restores."));
    previewGroup->setTitle(tr("Banner"));
    removeBtn->setText(tr("Remove Banner Card"));
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
        // No printings found for this card, nothing to preview.
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
    // Disconnect any previous listener first, otherwise switching cards
    // repeatedly stacks up connections to old CardInfo objects, each of
    // which would still fire reloadPreview() (harmlessly, but wastefully)
    // whenever ITS art finishes loading later.
    disconnect(pixmapUpdatedConnection);

    QPixmap fullRes;
    CardPictureLoader::getPixmap(fullRes, card, QSize(745, 1040));

    if (fullRes.isNull()) {
        // Not loaded yet, wait for the signal instead of giving up.
        // card.getCardPtr() is a CardInfoPtr (QSharedPointer<CardInfo>),
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

    // Only attribute the art once the new pixmap is actually displayed, so a
    // cache miss (which keeps the previous pixmap on screen) doesn't pair the
    // new card's attribution with the old card's art.
    preview->setAttribution(buildArtAttribution(card));
}

void UserCardArtSettingsDialog::onParamChanged()
{
    currentParams.marginPctL = marginLSpin->value();
    currentParams.marginPctR = marginRSpin->value();
    preview->setParams(currentParams);
}
