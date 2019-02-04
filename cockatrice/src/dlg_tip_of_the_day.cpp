#include <QCheckBox>
#include <QDate>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "dlg_tip_of_the_day.h"
#include "settingscache.h"
#include "tip_of_the_day.h"

#define MIN_TIP_IMAGE_HEIGHT 200
#define MIN_TIP_IMAGE_WIDTH 200
#define MAX_TIP_IMAGE_HEIGHT 300
#define MAX_TIP_IMAGE_WIDTH 500

DlgTipOfTheDay::DlgTipOfTheDay(QWidget *parent) : QDialog(parent)
{
    successfulInit = false;
    QString xmlPath = "theme:tips/tips_of_the_day.xml";
    tipDatabase = new TipsOfTheDay(xmlPath, this);

    if (tipDatabase->rowCount() == 0) {
        return;
    }

    title = new QLabel();
    title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tipTextContent = new QLabel();
    tipTextContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tipTextContent->setWordWrap(true);
    tipTextContent->setTextInteractionFlags(Qt::TextBrowserInteraction);
    tipTextContent->setOpenExternalLinks(true);
    imageLabel = new QLabel();
    imageLabel->setFixedHeight(MAX_TIP_IMAGE_HEIGHT + 50);
    imageLabel->setFixedWidth(MAX_TIP_IMAGE_WIDTH + 50);
    image = new QPixmap();
    date = new QLabel();
    date->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tipNumber = new QLabel();
    tipNumber->setAlignment(Qt::AlignCenter);

    if (settingsCache->getSeenTips().size() != tipDatabase->rowCount()) {
        newTipsAvailable = true;
        QList<int> rangeToMaxTips;
        for (int i = 0; i < tipDatabase->rowCount(); i++) {
            rangeToMaxTips.append(i);
        }
        QSet<int> unseenTips = rangeToMaxTips.toSet() - settingsCache->getSeenTips().toSet();
        currentTip = *std::min_element(unseenTips.begin(), unseenTips.end());
    } else {
        newTipsAvailable = false;
        currentTip = 0;
    }

    connect(this, SIGNAL(newTipRequested(int)), this, SLOT(updateTip(int)));
    newTipRequested(currentTip);

    content = new QVBoxLayout;
    content->addWidget(title);
    content->addWidget(tipTextContent);
    content->addWidget(imageLabel);
    content->addWidget(date);

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    nextButton = new QPushButton(tr("Next"));
    previousButton = new QPushButton(tr("Previous"));
    buttonBox->addButton(previousButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));

    showTipsOnStartupCheck = new QCheckBox("Show tips on startup");
    showTipsOnStartupCheck->setChecked(settingsCache->getShowTipsOnStartup());
    connect(showTipsOnStartupCheck, SIGNAL(clicked(bool)), settingsCache, SLOT(setShowTipsOnStartup(bool)));
    buttonBar = new QHBoxLayout();
    buttonBar->addWidget(showTipsOnStartupCheck);
    buttonBar->addWidget(tipNumber);
    buttonBar->addWidget(buttonBox);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(content);
    mainLayout->addLayout(buttonBar);
    setLayout(mainLayout);

    setWindowTitle(tr("Tip of the Day"));
    setMinimumWidth(500);
    setMinimumHeight(300);
    successfulInit = true;
}

DlgTipOfTheDay::~DlgTipOfTheDay()
{
    tipDatabase->deleteLater();
    title->deleteLater();
    tipTextContent->deleteLater();
    imageLabel->deleteLater();
    tipNumber->deleteLater();
    showTipsOnStartupCheck->deleteLater();
    content->deleteLater();
    mainLayout->deleteLater();
    buttonBox->deleteLater();
    nextButton->deleteLater();
    previousButton->deleteLater();
    buttonBar->deleteLater();
    delete image;
}

void DlgTipOfTheDay::nextClicked()
{
    emit newTipRequested(currentTip + 1);
}

void DlgTipOfTheDay::previousClicked()
{
    emit newTipRequested(currentTip - 1);
}

void DlgTipOfTheDay::updateTip(int tipId)
{
    QString titleText, contentText, imagePath;

    if (tipId < 0) {
        tipId = tipDatabase->rowCount() - 1;
    } else if (tipId >= tipDatabase->rowCount()) {
        tipId = tipId % tipDatabase->rowCount();
    }

    // Store tip id as seen
    QList<int> seenTips = settingsCache->getSeenTips();
    if (!seenTips.contains(tipId)) {
        seenTips.append(tipId);
        settingsCache->setSeenTips(seenTips);
    }

    TipOfTheDay tip = tipDatabase->getTip(tipId);
    titleText = tip.getTitle();
    contentText = tip.getContent();
    imagePath = tip.getImagePath();

    title->setText("<h2>" + titleText + "</h2>");
    tipTextContent->setText(contentText);

    if (!image->load(imagePath)) {
        qDebug() << "Image failed to load from" << imagePath;
        imageLabel->clear();
    } else {
        int h = std::min(std::max(imageLabel->height(), MIN_TIP_IMAGE_HEIGHT), MAX_TIP_IMAGE_HEIGHT);
        int w = std::min(std::max(imageLabel->width(), MIN_TIP_IMAGE_WIDTH), MAX_TIP_IMAGE_WIDTH);
        imageLabel->setPixmap(image->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    date->setText("<i>Tip added on: " + tip.getDate().toString("yyyy.MM.dd") + "</i>");

    tipNumber->setText("Tip " + QString::number(tipId + 1) + " / " + QString::number(tipDatabase->rowCount()));

    currentTip = static_cast<unsigned int>(tipId);
}

void DlgTipOfTheDay::resizeEvent(QResizeEvent *event)
{
    imageLabel->setPixmap(image->scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QWidget::resizeEvent(event);
}
