#include "dlg_edit_tokens.h"
#include "carddatabase.h"
#include "carddatabasemodel.h"
#include "main.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTreeView>
#include <QHeaderView>
#include <QToolBar>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

DlgEditTokens::DlgEditTokens(QWidget *parent)
    : QDialog(parent), currentCard(0)
{
    nameLabel = new QLabel(tr("&Name:"));
    nameEdit = new QLineEdit;
    nameEdit->setEnabled(false);
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
    connect(colorEdit, SIGNAL(currentIndexChanged(int)), this, SLOT(colorChanged(int)));
    
    ptLabel = new QLabel(tr("&P/T:"));
    ptEdit = new QLineEdit;
    ptLabel->setBuddy(ptEdit);
    connect(ptEdit, SIGNAL(textChanged(QString)), this, SLOT(ptChanged(QString)));
    
    annotationLabel = new QLabel(tr("&Annotation:"));
    annotationEdit = new QLineEdit;
    annotationLabel->setBuddy(annotationEdit);
    connect(annotationEdit, SIGNAL(textChanged(QString)), this, SLOT(annotationChanged(QString)));
    
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(nameEdit, 0, 1);
    grid->addWidget(colorLabel, 1, 0);
    grid->addWidget(colorEdit, 1, 1);
    grid->addWidget(ptLabel, 2, 0);
    grid->addWidget(ptEdit, 2, 1);
    grid->addWidget(annotationLabel, 3, 0);
    grid->addWidget(annotationEdit, 3, 1);
    
    QGroupBox *tokenDataGroupBox = new QGroupBox(tr("Token data"));
    tokenDataGroupBox->setLayout(grid);

    databaseModel = new CardDatabaseModel(db, this);
    databaseModel->setObjectName("databaseModel");
    cardDatabaseDisplayModel = new TokenDisplayModel(this);
    cardDatabaseDisplayModel->setSourceModel(databaseModel);
    cardDatabaseDisplayModel->setIsToken(CardDatabaseDisplayModel::ShowTrue);
    
    chooseTokenView = new QTreeView;
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
    chooseTokenView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    chooseTokenView->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    connect(chooseTokenView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(tokenSelectionChanged(QModelIndex, QModelIndex)));
    
    QAction *aAddToken = new QAction(tr("Add token"), this);
    aAddToken->setIcon(QPixmap("theme:icons/increment"));
    connect(aAddToken, SIGNAL(triggered()), this, SLOT(actAddToken()));
    QAction *aRemoveToken = new QAction(tr("Remove token"), this);
    aRemoveToken->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemoveToken, SIGNAL(triggered()), this, SLOT(actRemoveToken()));
    
    QToolBar *databaseToolBar = new QToolBar;
    databaseToolBar->addAction(aAddToken);
    databaseToolBar->addAction(aRemoveToken);
    
    QVBoxLayout *leftVBox = new QVBoxLayout;
    leftVBox->addWidget(chooseTokenView);
    leftVBox->addWidget(databaseToolBar);
    
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addLayout(leftVBox);
    hbox->addWidget(tokenDataGroupBox);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Edit tokens"));
}

void DlgEditTokens::tokenSelectionChanged(const QModelIndex &current, const QModelIndex & /* previous */)
{
    const QModelIndex realIndex = cardDatabaseDisplayModel->mapToSource(current);
    currentCard = current.row() >= 0 ? databaseModel->getCard(realIndex.row()) : 0;

    if(currentCard)
    {
        nameEdit->setText(currentCard->getName());
        const QChar cardColor = currentCard->getColorChar();
        colorEdit->setCurrentIndex(colorEdit->findData(cardColor, Qt::UserRole, Qt::MatchFixedString));
        ptEdit->setText(currentCard->getPowTough());
        annotationEdit->setText(currentCard->getText());
    } else {
        nameEdit->setText("");
        colorEdit->setCurrentIndex(colorEdit->findData(QChar(), Qt::UserRole, Qt::MatchFixedString));
        ptEdit->setText("");
        annotationEdit->setText("");
    }
}

void DlgEditTokens::actAddToken()
{
    QString name;
    bool askAgain = true;
    do {
        name = QInputDialog::getText(this, tr("Add token"), tr("Please enter the name of the token:"));
        if(name.isEmpty())
            return;
        if (databaseModel->getDatabase()->getCard(name)) {
            QMessageBox::critical(this, tr("Error"), tr("The chosen name conflicts with an existing card or token.\nMake sure to enable the 'token set' in the 'Edit sets...' dialog to display them correctly."));
        } else {
            askAgain = false;
        }
    } while (askAgain);
    
    
    CardInfo *card = new CardInfo(name, true);
    card->addToSet(databaseModel->getDatabase()->getSet(CardDatabase::TOKENS_SETNAME));
    card->setCardType("Token");
    databaseModel->getDatabase()->addCard(card);
}

void DlgEditTokens::actRemoveToken()
{
    if (currentCard) {
        CardInfo *cardToRemove = currentCard; // the currentCard property gets modified during db->removeCard()
        currentCard = 0;
        databaseModel->getDatabase()->removeCard(cardToRemove);
        delete cardToRemove;
    }
}

void DlgEditTokens::colorChanged(int colorIndex)
{
    if (currentCard)
        currentCard->setColors(QStringList() << QString(colorEdit->itemData(colorIndex).toChar()));
}

void DlgEditTokens::ptChanged(const QString &_pt)
{
    if (currentCard)
        currentCard->setPowTough(_pt);
}

void DlgEditTokens::annotationChanged(const QString &_annotation)
{
    if (currentCard)
        currentCard->setText(_annotation);
}
