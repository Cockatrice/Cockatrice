#include "home_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../tabs/tab_supervisor.h"
#include "home_styled_button.h"

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>

HomeWidget::HomeWidget(QWidget *parent, TabSupervisor *_tabSupervisor)
    : QWidget(parent), tabSupervisor(_tabSupervisor), background("theme:backgrounds/home"), overlay("theme:cockatrice")
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    layout = new QGridLayout(this);

    backgroundSource = new CardInfoPictureArtCropWidget(this);

    backgroundSource->setCard(CardDatabaseManager::getInstance()->getRandomCard());

    background = backgroundSource->getProcessedBackground(size());

    gradientColors = extractDominantColors(background);

    layout->addWidget(createSettingsButtonGroup("Settings"), 0, 0, Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(createUpdatesButtonGroup("Updates"), 0, 2, Qt::AlignTop | Qt::AlignRight);
    layout->addWidget(createNavigationButtonGroup("Navigation"), 2, 0, Qt::AlignBottom | Qt::AlignLeft);
    layout->addWidget(createPlayButtonGroup("Play"), 2, 2, Qt::AlignBottom | Qt::AlignRight);

    layout->setRowStretch(0, 1);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);

    setLayout(layout);

    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
            &HomeWidget::startCardShuffleTimer);

    if (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok) {
        startCardShuffleTimer();
    }
}

void HomeWidget::startCardShuffleTimer()
{
    cardChangeTimer = new QTimer(this);
    connect(cardChangeTimer, &QTimer::timeout, this, &HomeWidget::updateRandomCard);
    cardChangeTimer->start(5000); // 20 seconds
}

void HomeWidget::updateRandomCard()
{
    ExactCard newCard = CardDatabaseManager::getInstance()->getRandomCard();
    if (!newCard)
        return;

    connect(newCard.getCardPtr().data(), &CardInfo::pixmapUpdated, this, &HomeWidget::updateBackgroundProperties);
    backgroundSource->setCard(newCard);
}

void HomeWidget::updateBackgroundProperties()
{
    background = backgroundSource->getProcessedBackground(size());

    gradientColors = extractDominantColors(background);
    for (HomeStyledButton *button : findChildren<HomeStyledButton *>()) {
        button->updateStylesheet(gradientColors);
        button->update();
    }
    update(); // Triggers repaint
}

QGroupBox *HomeWidget::createSettingsButtonGroup(const QString &title)
{
    QGroupBox *box = new QGroupBox(title);
    box->setStyleSheet(R"(
    QGroupBox {
        font-size: 20px;
        color: white;         /* Title text color */
        background: transparent;
    }

    QGroupBox::title {
        color: white;
        subcontrol-origin: margin;
        subcontrol-position: top center;  /* or top left / right */
    }
)");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter); // Center widgets horizontally
    layout->addWidget(new HomeStyledButton("Settings", gradientColors));
    box->setLayout(layout);
    return box;
}

QGroupBox *HomeWidget::createUpdatesButtonGroup(const QString &title)
{
    QGroupBox *box = new QGroupBox(title);
    box->setStyleSheet(R"(
    QGroupBox {
        font-size: 20px;
        color: white;         /* Title text color */
        background: transparent;
    }

    QGroupBox::title {
        color: white;
        subcontrol-origin: margin;
        subcontrol-position: top center;  /* or top left / right */
    }
)");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter); // Center widgets horizontally
    layout->addWidget(new HomeStyledButton("Updates", gradientColors));
    box->setLayout(layout);
    return box;
}

QGroupBox *HomeWidget::createNavigationButtonGroup(const QString &title)
{
    QGroupBox *box = new QGroupBox(title);
    box->setStyleSheet(R"(
    QGroupBox {
        font-size: 20px;
        color: white;         /* Title text color */
        background: transparent;
    }

    QGroupBox::title {
        color: white;
        subcontrol-origin: margin;
        subcontrol-position: top center;  /* or top left / right */
    }
)");
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter); // Center widgets horizontally
    auto replaybutton = new HomeStyledButton("View Replays", gradientColors);
    connect(replaybutton, &QPushButton::clicked, tabSupervisor, [this] { tabSupervisor->actTabReplays(true); });
    layout->addWidget(replaybutton);
    auto edhrecButton = new HomeStyledButton("Browse EDHRec", gradientColors);
    connect(edhrecButton, &QPushButton::clicked, tabSupervisor, &TabSupervisor::addEdhrecMainTab);
    layout->addWidget(edhrecButton);
    auto visualDatabaseDisplayButton = new HomeStyledButton("Browse Card Database", gradientColors);
    connect(visualDatabaseDisplayButton, &QPushButton::clicked, tabSupervisor,
            &TabSupervisor::addVisualDatabaseDisplayTab);
    layout->addWidget(visualDatabaseDisplayButton);
    auto visualDeckStorageButton = new HomeStyledButton("Browse Decks", gradientColors);
    connect(visualDeckStorageButton, &QPushButton::clicked, tabSupervisor,
            [this] { tabSupervisor->actTabVisualDeckStorage(true); });
    layout->addWidget(visualDeckStorageButton);
    auto visualDeckEditorButton = new HomeStyledButton("Create New Deck", gradientColors);
    connect(visualDeckEditorButton, &QPushButton::clicked, tabSupervisor,
            [this] { tabSupervisor->addVisualDeckEditorTab(nullptr); });
    layout->addWidget(visualDeckEditorButton);
    box->setLayout(layout);
    return box;
}

QGroupBox *HomeWidget::createPlayButtonGroup(const QString &title)
{
    QGroupBox *box = new QGroupBox(title);
    box->setStyleSheet(R"(
    QGroupBox {
        font-size: 20px;
        color: white;         /* Title text color */
        background: transparent;
    }

    QGroupBox::title {
        color: white;
        subcontrol-origin: margin;
        subcontrol-position: top center;  /* or top left / right */
    }
)");
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter); // Center widgets horizontally

    auto connectButton = new HomeStyledButton("Connect", gradientColors);
    layout->addWidget(connectButton, 1); // stretch factor 1

    auto playButton = new HomeStyledButton("Play", gradientColors);
    layout->addWidget(playButton, 1);

    box->setLayout(layout);
    return box;
}

QPair<QColor, QColor> HomeWidget::extractDominantColors(const QPixmap &pixmap)
{
    // Step 1: Downscale image for performance
    QImage image = pixmap.toImage()
                       .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                       .convertToFormat(QImage::Format_RGB32);

    QMap<QRgb, int> colorCount;

    // Step 2: Count quantized colors
    for (int y = 0; y < image.height(); ++y) {
        const QRgb *scanLine = reinterpret_cast<const QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            QColor color = QColor::fromRgb(scanLine[x]);

            int r = color.red() & 0xF0;
            int g = color.green() & 0xF0;
            int b = color.blue() & 0xF0;

            QRgb quantized = qRgb(r, g, b);
            colorCount[quantized]++;
        }
    }

    // Step 3: Sort by frequency
    QVector<QPair<QRgb, int>> sortedColors;
    for (auto it = colorCount.constBegin(); it != colorCount.constEnd(); ++it) {
        sortedColors.append(qMakePair(it.key(), it.value()));
    }

    std::sort(sortedColors.begin(), sortedColors.end(),
              [](const QPair<QRgb, int> &a, const QPair<QRgb, int> &b) { return a.second > b.second; });

    // Step 4: Pick top two distinct colors
    QColor first = QColor(sortedColors.value(0).first);
    QColor second = first;

    for (int i = 1; i < sortedColors.size(); ++i) {
        QColor candidate = QColor(sortedColors[i].first);
        if (candidate != first) {
            second = candidate;
            break;
        }
    }

    return QPair<QColor, QColor>(first, second);
}

void HomeWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Update background if we have a source
    if (backgroundSource) {
        background = backgroundSource->getProcessedBackground(size());
    }

    // Draw already-scaled background centered
    QSize widgetSize = size();
    QSize bgSize = background.size();
    QPoint topLeft((widgetSize.width() - bgSize.width()) / 2, (widgetSize.height() - bgSize.height()) / 2);

    painter.drawPixmap(topLeft, background);

    // Draw translucent black overlay with rounded corners
    QRectF overlayRect(5, 5, width() - 10, height() - 10); // 5px inset
    QPainterPath roundedRectPath;
    roundedRectPath.addRoundedRect(overlayRect, 20, 20); // 20px corner radius

    QColor semiTransparentBlack(0, 0, 0, static_cast<int>(255 * 0.33)); // 33% opacity
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(roundedRectPath, semiTransparentBlack);

    // Draw centered overlay image
    QSize overlaySize = overlay.size();
    QPoint center((width() - overlaySize.width()) / 2, (height() - overlaySize.height()) / 2);
    painter.drawPixmap(center, overlay);

    QWidget::paintEvent(event);
}
