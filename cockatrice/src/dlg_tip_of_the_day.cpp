#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDate>

#include "dlg_tip_of_the_day.h"
#include "settingscache.h"
#include "tip_of_the_day.h"

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
    imageLabel = new QLabel();
	imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    image = new QPixmap();
	date = new QLabel();
	date->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	tipNumber = new QLabel();
	tipNumber->setAlignment(Qt::AlignCenter);

    currentTip = settingsCache->getLastShownTip() + 1;
    connect(this, SIGNAL(newTipRequested(int)), this, SLOT(updateTip(int)));
    newTipRequested(currentTip);

    auto *content = new QVBoxLayout();
    content->addWidget(title);
    content->addWidget(tipTextContent);
    content->addWidget(imageLabel);
	content->addWidget(date);

    auto *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    QPushButton *nextButton = new QPushButton(tr("Next"));
    QPushButton *previousButton = new QPushButton(tr("Previous"));
    buttonBox->addButton(previousButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
	buttonBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));

    showTipsOnStartupCheck = new QCheckBox("Show tips on startup");
    showTipsOnStartupCheck->setChecked(true);
    connect(showTipsOnStartupCheck, SIGNAL(clicked(bool)), settingsCache, SLOT(setShowTipsOnStartup(bool)));
    auto *buttonBar = new QHBoxLayout();
    buttonBar->addWidget(showTipsOnStartupCheck);
	buttonBar->addWidget(tipNumber);
    buttonBar->addWidget(buttonBox);

    auto *mainLayout = new QVBoxLayout;
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

    TipOfTheDay tip = tipDatabase->getTip(tipId);
    titleText = tip.getTitle();
    contentText = tip.getContent();
    imagePath = tip.getImagePath();

    title->setText("<h2>" + titleText + "</h2>");
    tipTextContent->setText(contentText);

    image->load(imagePath);
    int h = imageLabel->height();
    int w = imageLabel->width();
    imageLabel->setPixmap(image->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	date->setText("<i>Tip added on: " + tip.getDate().toString("yyyy.MM.dd") + "</i>");

	tipNumber->setText("Tip " + QString::number(tipId + 1) + " / " + QString::number(tipDatabase->rowCount()));

    currentTip = static_cast<unsigned int>(tipId);
    settingsCache->setLastShownTip(currentTip);
}

void DlgTipOfTheDay::resizeEvent(QResizeEvent *event)
{
    int h = imageLabel->height();
    int w = imageLabel->width();
    imageLabel->setPixmap(image->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QWidget::resizeEvent(event);
}
