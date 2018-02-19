#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "tip_of_the_day.h"
#include "settingscache.h"
#include "dlg_tip_of_the_day.h"

DlgTipOfTheDay::DlgTipOfTheDay(QWidget *parent) : QDialog(parent)
{
    QString xmlPath = "D:/tips_of_the_day.xml";
    tipDatabase = new TipsOfTheDay(xmlPath, this);

    title = new QLabel();
    title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tipTextContent = new QLabel();
    tipTextContent->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    imageLabel = new QLabel();
    image = new QPixmap();

    currentTip = settingsCache->getLastShownTip() + 1;
    connect(this, SIGNAL(newTipRequested(int)), this, SLOT(updateTip(int)));
    newTipRequested(currentTip);


    QVBoxLayout *content = new QVBoxLayout();
    content->addWidget(title);
    content->addWidget(tipTextContent);
    content->addWidget(imageLabel);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    QPushButton *nextButton = new QPushButton(tr("Next"));
    QPushButton *previousButton = new QPushButton(tr("Previous"));
    buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(previousButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(previousClicked()));

    showTipsOnStartupCheck = new QCheckBox("Show tips on startup");
    showTipsOnStartupCheck->setChecked(true);
    connect(showTipsOnStartupCheck, SIGNAL(clicked(bool)), settingsCache, SLOT(setShowTipsOnStartup(bool)));
    QHBoxLayout *buttonBar = new QHBoxLayout();
    buttonBar->addWidget(showTipsOnStartupCheck);
    buttonBar->addWidget(buttonBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(content);
    mainLayout->addLayout(buttonBar);
    setLayout(mainLayout);

    setWindowTitle(tr("Tip of the Day"));
    setMinimumWidth(500);
    setMinimumHeight(300);
}

void DlgTipOfTheDay::nextClicked() {
    emit newTipRequested(currentTip + 1);
}

void DlgTipOfTheDay::previousClicked() {
    emit newTipRequested(currentTip - 1);
}

void DlgTipOfTheDay::updateTip(int tipId) {
    QString titleText, contentText, imagePath;

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

    currentTip = tipId;
    settingsCache->setLastShownTip(currentTip);
}

void DlgTipOfTheDay::resizeEvent(QResizeEvent *event) {
    int h = imageLabel->height();
    int w = imageLabel->width();
    imageLabel->setPixmap(image->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QWidget::resizeEvent(event);
}
