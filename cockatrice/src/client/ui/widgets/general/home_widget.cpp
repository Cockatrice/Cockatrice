#include "home_widget.h"

#include "../../../tabs/tab_supervisor.h"
#include "home_styled_button.h"

#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

HomeWidget::HomeWidget(QWidget *parent, TabSupervisor *_tabSupervisor)
    : QWidget(parent), tabSupervisor(_tabSupervisor), background("theme:backgrounds/home"), overlay("theme:cockatrice")
{
    layout = new QGridLayout(this);

    layout->addWidget(createSettingsButtonGroup("Settings"), 0, 0, Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(createUpdatesButtonGroup("Updates"), 0, 2, Qt::AlignTop | Qt::AlignRight);
    layout->addWidget(createNavigationButtonGroup("Navigation"), 2, 0, Qt::AlignBottom | Qt::AlignLeft);
    layout->addWidget(createPlayButtonGroup("Play"), 2, 2, Qt::AlignBottom | Qt::AlignRight);

    layout->setRowStretch(0, 1);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);

    setLayout(layout);
}

QGroupBox *HomeWidget::createSettingsButtonGroup(const QString &title)
{
    QGroupBox *box = new QGroupBox(title);
    box->setStyleSheet(R"(
    QGroupBox {
        font-size: 20px;
        color: white;         /* Title text color */
    }

    QGroupBox::title {
        color: white;
        subcontrol-origin: margin;
        subcontrol-position: top center;  /* or top left / right */
    }
)");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter); // Center widgets horizontally
    layout->addWidget(new HomeStyledButton("Settings"));
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
    }

    QGroupBox::title {
        color: white;
        subcontrol-origin: margin;
        subcontrol-position: top center;  /* or top left / right */
    }
)");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignHCenter); // Center widgets horizontally
    layout->addWidget(new HomeStyledButton("Updates"));
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
    auto replaybutton = new HomeStyledButton("View Replays");
    connect(replaybutton, &QPushButton::clicked, tabSupervisor, [this] { tabSupervisor->actTabReplays(true); });
    layout->addWidget(replaybutton);
    auto edhrecButton = new HomeStyledButton("Browse EDHRec");
    connect(edhrecButton, &QPushButton::clicked, tabSupervisor, &TabSupervisor::addEdhrecMainTab);
    layout->addWidget(edhrecButton);
    auto visualDatabaseDisplayButton = new HomeStyledButton("Browse Card Database");
    connect(visualDatabaseDisplayButton, &QPushButton::clicked, tabSupervisor,
            &TabSupervisor::addVisualDatabaseDisplayTab);
    layout->addWidget(visualDatabaseDisplayButton);
    auto visualDeckStorageButton = new HomeStyledButton("Browse Decks");
    connect(visualDeckStorageButton, &QPushButton::clicked, tabSupervisor,
            [this] { tabSupervisor->actTabVisualDeckStorage(true); });
    layout->addWidget(visualDeckStorageButton);
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

    auto connectButton = new HomeStyledButton("Connect");
    layout->addWidget(connectButton, 1); // stretch factor 1

    auto playButton = new HomeStyledButton("Play");
    layout->addWidget(playButton, 1);

    box->setLayout(layout);
    return box;
}

void HomeWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Draw background to fill the widget
    painter.drawPixmap(rect(), background);

    // Draw overlay image centered
    QSize overlaySize = overlay.size();
    QPoint center((width() - overlaySize.width()) / 2, (height() - overlaySize.height()) / 2);
    painter.drawPixmap(center, overlay);

    QWidget::paintEvent(event); // optional, depending on behavior
}