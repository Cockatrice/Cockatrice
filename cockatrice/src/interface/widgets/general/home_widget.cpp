#include "home_widget.h"

#include "../../../interface/widgets/tabs/tab_supervisor.h"
#include "../../window_main.h"
#include "background_sources.h"
#include "home_styled_button.h"

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/remote/remote_client.h>
#include <libcockatrice/settings/cache_settings.h>

HomeWidget::HomeWidget(QWidget *parent, TabSupervisor *_tabSupervisor)
    : QWidget(parent), tabSupervisor(_tabSupervisor), background("theme:backgrounds/home"), overlay("theme:cockatrice")
{
    layout = new QGridLayout(this);

    backgroundSourceCard = new CardInfoPictureArtCropWidget(this);
    backgroundSourceDeck = new DeckLoader();

    backgroundSourceDeck->loadFromFile(SettingsCache::instance().getDeckPath() + "background.cod",
                                       DeckLoader::CockatriceFormat, false);

    gradientColors = extractDominantColors(background);

    layout->addWidget(createButtons(), 1, 1, Qt::AlignVCenter | Qt::AlignHCenter);

    layout->setRowStretch(0, 1);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);

    setLayout(layout);

    cardChangeTimer = new QTimer(this);
    connect(cardChangeTimer, &QTimer::timeout, this, &HomeWidget::updateRandomCard);

    initializeBackgroundFromSource();

    updateConnectButton(tabSupervisor->getClient()->getStatus());

    connect(tabSupervisor->getClient(), &RemoteClient::statusChanged, this, &HomeWidget::updateConnectButton);
    connect(&SettingsCache::instance(), &SettingsCache::homeTabBackgroundSourceChanged, this,
            &HomeWidget::initializeBackgroundFromSource);
    connect(&SettingsCache::instance(), &SettingsCache::homeTabBackgroundShuffleFrequencyChanged, this,
            &HomeWidget::onBackgroundShuffleFrequencyChanged);
}

void HomeWidget::initializeBackgroundFromSource()
{
    if (CardDatabaseManager::getInstance()->getLoadStatus() != LoadStatus::Ok) {
        connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
                &HomeWidget::initializeBackgroundFromSource);
        return;
    }

    auto backgroundSourceType = BackgroundSources::fromId(SettingsCache::instance().getHomeTabBackgroundSource());

    cardChangeTimer->stop();

    switch (backgroundSourceType) {
        case BackgroundSources::Theme:
            background = QPixmap("theme:backgrounds/home");
            updateButtonsToBackgroundColor();
            update();
            break;
        case BackgroundSources::RandomCardArt:
            cardChangeTimer->start(SettingsCache::instance().getHomeTabBackgroundShuffleFrequency() * 1000);
            break;
        case BackgroundSources::DeckFileArt:
            backgroundSourceDeck->loadFromFile(SettingsCache::instance().getDeckPath() + "background.cod",
                                               DeckLoader::CockatriceFormat, false);
            cardChangeTimer->start(SettingsCache::instance().getHomeTabBackgroundShuffleFrequency() * 1000);
            break;
    }
}

void HomeWidget::updateRandomCard()
{
    auto backgroundSourceType = BackgroundSources::fromId(SettingsCache::instance().getHomeTabBackgroundSource());

    ExactCard newCard;

    switch (backgroundSourceType) {
        case BackgroundSources::Theme:
            break;
        case BackgroundSources::RandomCardArt:
            do {
                newCard = CardDatabaseManager::query()->getRandomCard();
            } while (newCard == backgroundSourceCard->getCard() &&
                     newCard.getCardPtr()->getProperty("layout") != "normal");
            break;
        case BackgroundSources::DeckFileArt:
            QList<CardRef> cardRefs = backgroundSourceDeck->getCardRefList();
            ExactCard oldCard = backgroundSourceCard->getCard();

            if (!cardRefs.empty()) {
                if (cardRefs.size() == 1) {
                    newCard = CardDatabaseManager::query()->getCard(cardRefs.first());
                } else {
                    // Keep picking until different
                    do {
                        int idx = QRandomGenerator::global()->bounded(cardRefs.size());
                        newCard = CardDatabaseManager::query()->getCard(cardRefs.at(idx));
                    } while (newCard == oldCard);
                }
            } else {
                do {
                    newCard = CardDatabaseManager::query()->getRandomCard();
                } while (newCard == oldCard);
            }
            break;
    }
    if (!newCard)
        return;

    connect(newCard.getCardPtr().data(), &CardInfo::pixmapUpdated, this, &HomeWidget::updateBackgroundProperties);
    backgroundSourceCard->setCard(newCard);
    background = backgroundSourceCard->getProcessedBackground(size());

    if (SettingsCache::instance().getHomeTabBackgroundShuffleFrequency() <= 0) {
        cardChangeTimer->stop();
    }
}

void HomeWidget::onBackgroundShuffleFrequencyChanged()
{
    cardChangeTimer->stop();
    if (SettingsCache::instance().getHomeTabBackgroundShuffleFrequency() <= 0) {
        return;
    }
    cardChangeTimer->start(SettingsCache::instance().getHomeTabBackgroundShuffleFrequency() * 1000);
}

void HomeWidget::updateBackgroundProperties()
{
    background = backgroundSourceCard->getProcessedBackground(size());
    updateButtonsToBackgroundColor();
    update(); // Triggers repaint
}

void HomeWidget::updateButtonsToBackgroundColor()
{
    gradientColors = extractDominantColors(background);
    for (HomeStyledButton *button : findChildren<HomeStyledButton *>()) {
        button->updateStylesheet(gradientColors);
        button->update();
    }
}

QGroupBox *HomeWidget::createButtons()
{
    QGroupBox *box = new QGroupBox(this);
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
    QVBoxLayout *boxLayout = new QVBoxLayout;
    boxLayout->setAlignment(Qt::AlignHCenter);

    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(overlay.scaledToWidth(200, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    boxLayout->addWidget(logoLabel);
    boxLayout->addSpacing(25);

    connectButton = new HomeStyledButton("Connect/Play", gradientColors);
    boxLayout->addWidget(connectButton, 1);

    auto visualDeckEditorButton = new HomeStyledButton(tr("Create New Deck"), gradientColors);
    connect(visualDeckEditorButton, &QPushButton::clicked, tabSupervisor,
            [this] { tabSupervisor->openDeckInNewTab(nullptr); });
    boxLayout->addWidget(visualDeckEditorButton);
    auto visualDeckStorageButton = new HomeStyledButton(tr("Browse Decks"), gradientColors);
    connect(visualDeckStorageButton, &QPushButton::clicked, tabSupervisor,
            [this] { tabSupervisor->actTabVisualDeckStorage(true); });
    boxLayout->addWidget(visualDeckStorageButton);
    auto visualDatabaseDisplayButton = new HomeStyledButton(tr("Browse Card Database"), gradientColors);
    connect(visualDatabaseDisplayButton, &QPushButton::clicked, tabSupervisor,
            &TabSupervisor::addVisualDatabaseDisplayTab);
    boxLayout->addWidget(visualDatabaseDisplayButton);
    auto edhrecButton = new HomeStyledButton(tr("Browse EDHRec"), gradientColors);
    connect(edhrecButton, &QPushButton::clicked, tabSupervisor, &TabSupervisor::addEdhrecMainTab);
    boxLayout->addWidget(edhrecButton);
    auto replaybutton = new HomeStyledButton(tr("View Replays"), gradientColors);
    connect(replaybutton, &QPushButton::clicked, tabSupervisor, [this] { tabSupervisor->actTabReplays(true); });
    boxLayout->addWidget(replaybutton);
    if (qobject_cast<MainWindow *>(tabSupervisor->parentWidget())) {
        auto exitButton = new HomeStyledButton(tr("Quit"), gradientColors);
        connect(exitButton, &QPushButton::clicked, qobject_cast<MainWindow *>(tabSupervisor->parentWidget()),
                &MainWindow::actExit);
        boxLayout->addWidget(exitButton);
    }

    box->setLayout(boxLayout);
    return box;
}

void HomeWidget::updateConnectButton(const ClientStatus status)
{
    disconnect(connectButton, &QPushButton::clicked, nullptr, nullptr);
    switch (status) {
        case StatusConnecting:
            connectButton->setText(tr("Connecting..."));
            connectButton->setEnabled(false);
            break;
        case StatusDisconnected:
            connectButton->setText(tr("Connect"));
            connectButton->setEnabled(true);
            connect(connectButton, &QPushButton::clicked, qobject_cast<MainWindow *>(tabSupervisor->parentWidget()),
                    &MainWindow::actConnect);
            break;
        case StatusLoggedIn:
            connectButton->setText(tr("Play"));
            connectButton->setEnabled(true);
            connect(connectButton, &QPushButton::clicked, tabSupervisor,
                    &TabSupervisor::switchToFirstAvailableNetworkTab);
            break;
        default:
            break;
    }
}

QPair<QColor, QColor> HomeWidget::extractDominantColors(const QPixmap &pixmap)
{
    if (SettingsCache::instance().getThemeName() == "Default" &&
        SettingsCache::instance().getHomeTabBackgroundSource() == BackgroundSources::toId(BackgroundSources::Theme)) {
        return QPair<QColor, QColor>(QColor::fromRgb(20, 140, 60), QColor::fromRgb(120, 200, 80));
    }

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

    background = background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

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

    QWidget::paintEvent(event);
}
