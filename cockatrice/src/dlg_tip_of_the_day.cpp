#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "dlg_tip_of_the_day.h"
#include "settingscache.h"

DlgTipOfTheDay::DlgTipOfTheDay(QWidget *parent) : QDialog(parent)
{
    title = new QLabel();
    title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tipTextContent = new QLabel();
    tipTextContent->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    imageLabel = new QLabel();
    image = new QPixmap();

    // TODO: read this from settings:
    currentTip = 0;

    connect(this, SIGNAL(newTipRequested(int)), this, SLOT(updateTip(int)));
    newTipRequested(currentTip);


    QVBoxLayout *content = new QVBoxLayout();
    content->addWidget(title);
    content->addWidget(tipTextContent);
    content->addWidget(imageLabel);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(tr("Next"), QDialogButtonBox::ActionRole);
    buttonBox->addButton(tr("Previous"), QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(nextOrPrevClicked(QAbstractButton*)));

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

void DlgTipOfTheDay::nextOrPrevClicked(QAbstractButton* button) {
    if (button->text().compare(tr("Next")) == 0) {
        emit newTipRequested(currentTip + 1);
    }
    else {
        emit newTipRequested(currentTip - 1);
    }
}

void DlgTipOfTheDay::updateTip(int tipId) {
    // TODO: these should be parsed from the xml (by tip_of_the_day.cpp)
    QString *titleText = new QString(tr("Title of the tip"));
    QString *contentText = new QString(tr("This is the data part of the tip.\nI hope you found it helpful!\n\nThanks for taking the time to read it!"));
    QString *imagePath = new QString("d:/test_img.png");

    *titleText = "<h2>" + *titleText + "</h2>";

    title->setText(*titleText);
    tipTextContent->setText(*contentText);

    image->load(*imagePath);
    int h = imageLabel->height();
    int w = imageLabel->width();
    imageLabel->setPixmap(image->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void DlgTipOfTheDay::resizeEvent(QResizeEvent *event) {
    int h = imageLabel->height();
    int w = imageLabel->width();
    imageLabel->setPixmap(image->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QWidget::resizeEvent(event);
}
