#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QTreeView>
#include <QRadioButton>
#include <QHeaderView>

#include "decklist.h"
#include "dlg_create_token.h"
#include "carddatabasemodel.h"
#include "main.h"
#include "settingscache.h"

DlgCreateToken::DlgCreateToken(const QStringList &_predefinedTokens, QWidget *parent)
    : QDialog(parent), predefinedTokens(_predefinedTokens)
{
    nameLabel = new QLabel(tr("&Name:"));
    nameEdit = new QLineEdit(tr("Token"));
    nameEdit->selectAll();
    connect(nameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
    nameLabel->setBuddy(nameEdit);

    colorLabel = new QLabel(tr("C&olor:"));
    colorEdit = new QComboBox;
    colorEdit->addItem(tr("white"), QChar('w'));
    colorEdit->addItem(tr("blue"), QChar('u'));
    colorEdit->addItem(tr("black"), QChar('b'));
    colorEdit->addItem(tr("red"), QChar('r'));
    colorEdit->addItem(tr("green"), QChar('g'));
    colorEdit->addItem(tr("multicolor"), QChar('m'));
    colorEdit->addItem(tr("colorless"), QChar());
    colorLabel->setBuddy(colorEdit);

    ptLabel = new QLabel(tr("&P/T:"));
    ptEdit = new QLineEdit;
    ptLabel->setBuddy(ptEdit);

    annotationLabel = new QLabel(tr("&Annotation:"));
    annotationEdit = new QLineEdit;
    annotationLabel->setBuddy(annotationEdit);
    
    destroyCheckBox = new QCheckBox(tr("&Destroy token when it leaves the table"));
    destroyCheckBox->setChecked(true);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(nameEdit, 0, 1);
    grid->addWidget(colorLabel, 1, 0);
    grid->addWidget(colorEdit, 1, 1);
    grid->addWidget(ptLabel, 2, 0);
    grid->addWidget(ptEdit, 2, 1);
    grid->addWidget(annotationLabel, 3, 0);
    grid->addWidget(annotationEdit, 3, 1);
    grid->addWidget(destroyCheckBox, 4, 0, 1, 2);
    
    QGroupBox *tokenDataGroupBox = new QGroupBox(tr("Token data"));
    tokenDataGroupBox->setLayout(grid);
    
    cardDatabaseModel = new CardDatabaseModel(db, this);
    cardDatabaseDisplayModel = new TokenDisplayModel(this);
    cardDatabaseDisplayModel->setSourceModel(cardDatabaseModel);
    
    chooseTokenFromAllRadioButton = new QRadioButton(tr("Show &all tokens"));
    connect(chooseTokenFromAllRadioButton, SIGNAL(toggled(bool)), this, SLOT(actChooseTokenFromAll(bool)));
    chooseTokenFromDeckRadioButton = new QRadioButton(tr("Show tokens from this &deck"));
    connect(chooseTokenFromDeckRadioButton, SIGNAL(toggled(bool)), this, SLOT(actChooseTokenFromDeck(bool)));
    QTreeView *chooseTokenView = new QTreeView;
    chooseTokenView->setModel(cardDatabaseDisplayModel);
    chooseTokenView->setUniformRowHeights(true);
    chooseTokenView->setRootIsDecorated(false);
    chooseTokenView->setAlternatingRowColors(true);
    chooseTokenView->setSortingEnabled(true);
    chooseTokenView->sortByColumn(0, Qt::AscendingOrder);
    chooseTokenView->resizeColumnToContents(0);
    chooseTokenView->header()->setStretchLastSection(false);
    chooseTokenView->header()->hideSection(1);
    chooseTokenView->header()->hideSection(2);
    chooseTokenView->setWordWrap(true);
    chooseTokenView->setColumnWidth(0, 130);
    chooseTokenView->setColumnWidth(3, 178);
    chooseTokenView->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    connect(chooseTokenView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(tokenSelectionChanged(QModelIndex, QModelIndex)));
    
    if (predefinedTokens.isEmpty())
        chooseTokenFromAllRadioButton->setChecked(true);
    else {
        chooseTokenFromDeckRadioButton->setChecked(true);
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>::fromList(predefinedTokens));
    }
    
    QVBoxLayout *tokenChooseLayout = new QVBoxLayout;
    tokenChooseLayout->addWidget(chooseTokenFromAllRadioButton);
    tokenChooseLayout->addWidget(chooseTokenFromDeckRadioButton);
    tokenChooseLayout->addWidget(chooseTokenView);
    
    QGroupBox *tokenChooseGroupBox = new QGroupBox(tr("Choose token from list"));
    tokenChooseGroupBox->setLayout(tokenChooseLayout);
    
    QVBoxLayout *leftVBox = new QVBoxLayout;
    leftVBox->addWidget(tokenDataGroupBox);
    leftVBox->addStretch();

    QGridLayout *hbox = new QGridLayout;
    hbox->addLayout(leftVBox, 0, 0);
    hbox->addWidget(tokenChooseGroupBox, 0, 1);
    hbox->setColumnStretch(1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Create token"));
    setFixedHeight(sizeHint().height());
    setFixedWidth(width());
}

void DlgCreateToken::tokenSelectionChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QModelIndex realIndex = cardDatabaseDisplayModel->mapToSource(current);
    const CardInfo *cardInfo = current.row() >= 0 ? cardDatabaseModel->getCard(realIndex.row()) : 0;
    
    if(cardInfo)
    {
        updateSearchFieldWithoutUpdatingFilter(cardInfo->getName());
        const QChar cardColor = cardInfo->getColorChar();
        colorEdit->setCurrentIndex(colorEdit->findData(cardColor, Qt::UserRole, Qt::MatchFixedString));
        ptEdit->setText(cardInfo->getPowTough());
        if(settingsCache->getAnnotateTokens())
            annotationEdit->setText(cardInfo->getText());
    } else {
        nameEdit->setText("");
        colorEdit->setCurrentIndex(colorEdit->findData(QString(), Qt::UserRole, Qt::MatchFixedString));
        ptEdit->setText("");
        annotationEdit->setText("");
    }
}

void DlgCreateToken::updateSearchFieldWithoutUpdatingFilter(const QString &newValue) const {
    nameEdit->blockSignals(true);
    nameEdit->setText(newValue);
    nameEdit->blockSignals(false);
}

void DlgCreateToken::updateSearch(const QString &search)
{
    cardDatabaseDisplayModel->setCardName(search);
}

void DlgCreateToken::actChooseTokenFromAll(bool checked)
{
    if (checked)
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>());
}

void DlgCreateToken::actChooseTokenFromDeck(bool checked)
{
    if (checked)
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>::fromList(predefinedTokens));
}

void DlgCreateToken::actOk()
{
    accept();
}

QString DlgCreateToken::getName() const
{
    return nameEdit->text();
}

QString DlgCreateToken::getColor() const
{
    return QString(colorEdit->itemData(colorEdit->currentIndex()).toChar());
}

QString DlgCreateToken::getPT() const
{
    return ptEdit->text();
}

QString DlgCreateToken::getAnnotation() const
{
    return annotationEdit->text();
}

bool DlgCreateToken::getDestroy() const
{
    return destroyCheckBox->isChecked();
}
