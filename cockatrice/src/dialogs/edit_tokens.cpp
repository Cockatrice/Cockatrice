#include "edit_tokens.h"

#include "../game/cards/database.h"
#include "../game/cards/database_model.h"
#include "gettextwithmax.h"
#include "../main.h"
#include "trice_limits.h"

#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

EditTokens::EditTokens(QWidget *parent) : QDialog(parent), currentCard(nullptr)
{
    nameLabel = new QLabel(tr("&Name:"));
    nameEdit = new QLineEdit;
    nameEdit->setMaxLength(MAX_NAME_LENGTH);
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
    ptEdit->setMaxLength(MAX_NAME_LENGTH);
    ptLabel->setBuddy(ptEdit);
    connect(ptEdit, SIGNAL(textChanged(QString)), this, SLOT(ptChanged(QString)));

    annotationLabel = new QLabel(tr("&Annotation:"));
    annotationEdit = new QLineEdit;
    annotationEdit->setMaxLength(MAX_NAME_LENGTH);
    annotationLabel->setBuddy(annotationEdit);
    connect(annotationEdit, SIGNAL(textChanged(QString)), this, SLOT(annotationChanged(QString)));

    auto *grid = new QGridLayout;
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

    databaseModel = new CardDatabaseModel(db, false, this);
    databaseModel->setObjectName("databaseModel");
    cardDatabaseDisplayModel = new TokenEditModel(this);
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

    connect(chooseTokenView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this,
            SLOT(tokenSelectionChanged(QModelIndex, QModelIndex)));

    QAction *aAddToken = new QAction(tr("Add token"), this);
    aAddToken->setIcon(QPixmap("theme:icons/increment"));
    connect(aAddToken, SIGNAL(triggered()), this, SLOT(actAddToken()));
    QAction *aRemoveToken = new QAction(tr("Remove token"), this);
    aRemoveToken->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemoveToken, SIGNAL(triggered()), this, SLOT(actRemoveToken()));

    auto *databaseToolBar = new QToolBar;
    databaseToolBar->addAction(aAddToken);
    databaseToolBar->addAction(aRemoveToken);

    auto *leftVBox = new QVBoxLayout;
    leftVBox->addWidget(chooseTokenView);
    leftVBox->addWidget(databaseToolBar);

    auto *hbox = new QHBoxLayout;
    hbox->addLayout(leftVBox);
    hbox->addWidget(tokenDataGroupBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Edit custom tokens"));
}

void EditTokens::tokenSelectionChanged(const QModelIndex &current, const QModelIndex & /* previous */)
{
    const QModelIndex realIndex = cardDatabaseDisplayModel->mapToSource(current);

    if (current.row() >= 0) {
        currentCard = databaseModel->getCard(realIndex.row());
    } else {
        currentCard.clear();
    }

    if (currentCard) {
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

void EditTokens::actAddToken()
{
    QString name;
    for (;;) {
        name = getTextWithMax(this, tr("Add token"), tr("Please enter the name of the token:"));
        if (name.isEmpty())
            return;
        if (databaseModel->getDatabase()->getCard(name)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The chosen name conflicts with an existing card or token.\nMake sure to enable "
                                     "the 'Token' set in the \"Manage sets\" dialog to display them correctly."));
        } else {
            break;
        }
    }

    QString setName = CardDatabase::TOKENS_SETNAME;
    CardInfoPerSetMap sets;
    sets.insert(setName, CardInfoPerSet(databaseModel->getDatabase()->getSet(setName)));
    CardInfoPtr card = CardInfo::newInstance(name, "", true, QVariantHash(), QList<CardRelation *>(),
                                             QList<CardRelation *>(), sets, false, -1, false);
    card->setCardType("Token");

    databaseModel->getDatabase()->addCard(card);
}

void EditTokens::actRemoveToken()
{
    if (currentCard) {
        CardInfoPtr cardToRemove = currentCard; // the currentCard property gets modified during db->removeCard()
        currentCard.clear();
        databaseModel->getDatabase()->removeCard(cardToRemove);
    }
}

void EditTokens::colorChanged(int colorIndex)
{
    if (currentCard)
        currentCard->setColors(QString(colorEdit->itemData(colorIndex).toChar()));
}

void EditTokens::ptChanged(const QString &_pt)
{
    if (currentCard)
        currentCard->setPowTough(_pt);
}

void EditTokens::annotationChanged(const QString &_annotation)
{
    if (currentCard)
        currentCard->setText(_annotation);
}
