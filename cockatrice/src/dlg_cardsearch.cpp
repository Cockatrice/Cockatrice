#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "dlg_cardsearch.h"
#include "carddatabase.h"
#include "main.h"

DlgCardSearch::DlgCardSearch(QWidget *parent)
    : QDialog(parent)
{
    QLabel *cardNameLabel = new QLabel(tr("Card name:"));
    cardNameEdit = new QLineEdit;

    QLabel *cardTextLabel = new QLabel(tr("Card text:"));
    cardTextEdit = new QLineEdit;

    //Adding Set Selector
    QLabel *cardSetsLabel = new QLabel(tr("Card Set (AND):"));
    cboSetList = new QComboBox();
    cboSetList->addItem("");
    SetList setList = db->getSetList();
    setList.sortByName();
    for (int intLoop = 0; intLoop < setList.size(); ++intLoop) {
        //Adding Translated Set Name, Just In Case
        cboSetList->addItem(tr("%1").arg(setList.at(intLoop)->getLongName()), setList.at(intLoop)->getShortName());
    }
    
    //Adding Set Selector
    QLabel *cardSetsLabel = new QLabel(tr("Card Set (AND):"));
    cboSetList = new QComboBox();
    cboSetList->addItem("");
    SetList setList = db->getSetList();
    for (int intLoop = 0; intLoop < setList.size(); ++intLoop) {
        //Adding Translated Set Name, Just In Case
        cboSetList->addItem(tr("%1").arg(setList.at(intLoop)->getLongName()), setList.at(intLoop)->getShortName());
    }
    
    QLabel *cardTypesLabel = new QLabel(tr("Card type (OR):"));
    const QStringList &cardTypes = db->getAllMainCardTypes();
    QVBoxLayout *cardTypesLayout = new QVBoxLayout;
    for (int i = 0; i < cardTypes.size(); ++i) {
        QCheckBox *cardTypeCheckBox = new QCheckBox(cardTypes[i]);
        cardTypeCheckBox->setChecked(true);
        cardTypeCheckBoxes.append(cardTypeCheckBox);
        cardTypesLayout->addWidget(cardTypeCheckBox);
    }
    
    QLabel *cardColorsLabel = new QLabel(tr("Color (OR):"));
    const QStringList &cardColors = db->getAllColors();
    QHBoxLayout *cardColorsLayout = new QHBoxLayout;
    for (int i = 0; i < cardColors.size(); ++i) {
        QCheckBox *cardColorCheckBox = new QCheckBox(cardColors[i]);
        cardColorCheckBox->setChecked(true);
        cardColorCheckBoxes.append(cardColorCheckBox);
        cardColorsLayout->addWidget(cardColorCheckBox);
    }
    
    QPushButton *okButton = new QPushButton(tr("O&K"));
    okButton->setDefault(true);
    okButton->setAutoDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    QHBoxLayout *buttonHBox = new QHBoxLayout;
    buttonHBox->addStretch();
    buttonHBox->addWidget(okButton);
    buttonHBox->addWidget(cancelButton);
    
    QGridLayout *optionsLayout = new QGridLayout;
    optionsLayout->addWidget(cardNameLabel, 0, 0);
    optionsLayout->addWidget(cardNameEdit, 0, 1);
    optionsLayout->addWidget(cardTextLabel, 1, 0);
    optionsLayout->addWidget(cardTextEdit, 1, 1);
    optionsLayout->addWidget(cardSetsLabel, 2, 0);
    optionsLayout->addWidget(cboSetList, 2, 1);
    optionsLayout->addWidget(cardTypesLabel, 3, 0);
    optionsLayout->addLayout(cardTypesLayout, 3, 1);
    optionsLayout->addWidget(cardColorsLabel, 4, 0);
    optionsLayout->addLayout(cardColorsLayout, 4, 1);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(optionsLayout);
    mainLayout->addLayout(buttonHBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Card search"));
}

QString DlgCardSearch::getCardName() const
{
    return cardNameEdit->text();
}

QString DlgCardSearch::getCardText() const
{
    return cardTextEdit->text();
}

QString DlgCardSearch::getCardSet() const {
    return cboSetList->itemData(cboSetList->currentIndex()).toString();
}

QSet<QString> DlgCardSearch::getCardTypes() const
{
    QStringList result;
    for (int i = 0; i < cardTypeCheckBoxes.size(); ++i)
        if (cardTypeCheckBoxes[i]->isChecked())
            result.append(cardTypeCheckBoxes[i]->text());
    return QSet<QString>::fromList(result);
}

QSet<QString> DlgCardSearch::getCardColors() const
{
    QStringList result;
    for (int i = 0; i < cardColorCheckBoxes.size(); ++i)
        if (cardColorCheckBoxes[i]->isChecked())
            result.append(cardColorCheckBoxes[i]->text());
    return QSet<QString>::fromList(result);
}
