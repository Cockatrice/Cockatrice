#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include "dlg_tip_of_the_day.h"

DlgTipOfTheDay::DlgTipOfTheDay(QWidget *parent, TipOfTheDaySettings *settings) : QDialog(parent)
{
    setWindowTitle(tr("Tip of the Day"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    tipLayout = new QVBoxLayout;
    QVBoxLayout *controlLayout = new QVBoxLayout;
    QHBoxLayout *navButtons = new QHBoxLayout;
    QPushButton *nextTipButton = new QPushButton(tr("Next"));
    QPushButton *previousTipButton = new QPushButton(tr("Previous"));
    QCheckBox *shouldShowTipsOnStartup = new QCheckBox(tr("Show Tip of the Day when Cockatrice starts."));

    navButtons->addWidget(previousTipButton);
    navButtons->addWidget(nextTipButton);
    controlLayout->addItem(navButtons);
    controlLayout->addWidget(shouldShowTipsOnStartup);

    mainLayout->addItem(tipLayout);
    mainLayout->addItem(controlLayout);

    connect(nextTipButton, SIGNAL(clicked()), this, SLOT(showNextTip()));
    connect(previousTipButton, SIGNAL(clicked()), this, SLOT(showPreviousTip()));
    connect(shouldShowTipsOnStartup, SIGNAL(stateChanged(int)), settings, SLOT(setShouldShowTipsOnStartup(int)));
}

void DlgTipOfTheDay::addTips(QList<TipOfTheDay> &_tips)
{
    tips.append(_tips);
}

void DlgTipOfTheDay::showNextTip()
{
    if (tips.length() == 0) return;
    tipIndex = (tipIndex + 1) % tips.length();
    renderTip(tips.at(tipIndex));
};

void DlgTipOfTheDay::showPreviousTip()
{
    if (tips.length() == 0) return;
    // Add in tips.length again so that we always get a positive index
    tipIndex = (tipIndex - 1 + tips.length()) % tips.length();
    renderTip(tips.at(tipIndex));
}

void DlgTipOfTheDay::renderTip(TipOfTheDay tip)
{
    clearTipLayout();
    tipLayout->addItem(getLayoutForTip(tip));
    markTipAsRead(tip.settingName);
    tipLayout->update();
};

void DlgTipOfTheDay::clearTipLayout()
{
    if (!tipLayout) return;
    while (tipLayout->count() > 0) {
        QLayoutItem *item = tipLayout->takeAt(0);
        delete item->widget();
        delete item;
    }
}

QVBoxLayout* DlgTipOfTheDay::getLayoutForTip(TipOfTheDay &tip)
{
    QVBoxLayout* layout = new QVBoxLayout;
    QLabel* title = new QLabel(tr(qPrintable(tip.title)));
    // Add image
    // Add text
    return layout;
}

void DlgTipOfTheDay::markTipAsRead(const QString &tipSettingName)
{

}



