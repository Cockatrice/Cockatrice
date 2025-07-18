#include "dlg_edit_tokens.h"

#include "../client/get_text_with_max.h"
#include "../game/cards/card_database.h"
#include "../game/cards/card_database_manager.h"
#include "../game/cards/card_database_model.h"
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

DlgEditTokens::DlgEditTokens(QWidget *parent) : QDialog(parent), currentCard(nullptr)
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
    connect(colorEdit, qOverload<int>(&QComboBox::currentIndexChanged), this, &DlgEditTokens::colorChanged);

    ptLabel = new QLabel(tr("&P/T:"));
    ptEdit = new QLineEdit;
    ptEdit->setMaxLength(MAX_NAME_LENGTH);
    ptLabel->setBuddy(ptEdit);
    connect(ptEdit, &QLineEdit::textChanged, this, &DlgEditTokens::ptChanged);

    annotationLabel = new QLabel(tr("&Annotation:"));
    annotationEdit = new QLineEdit;
    annotationEdit->setMaxLength(MAX_NAME_LENGTH);
    annotationLabel->setBuddy(annotationEdit);
    connect(annotationEdit, &QLineEdit::textChanged, this, &DlgEditTokens::annotationChanged);

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

    databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
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

    connect(chooseTokenView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &DlgEditTokens::tokenSelectionChanged);

    QAction *aAddToken = new QAction(tr("Add token"), this);
    aAddToken->setIcon(QPixmap("theme:icons/increment"));
    connect(aAddToken, &QAction::triggered, this, &DlgEditTokens::actAddToken);
    QAction *aRemoveToken = new QAction(tr("Remove token"), this);
    aRemoveToken->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemoveToken, &QAction::triggered, this, &DlgEditTokens::actRemoveToken);

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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgEditTokens::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgEditTokens::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Edit custom tokens"));
}

void DlgEditTokens::tokenSelectionChanged(const QModelIndex &current, const QModelIndex & /* previous */)
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

void DlgEditTokens::actAddToken()
{
    QString name;
    for (;;) {
        name = getTextWithMax(this, tr("Add token"), tr("Please enter the name of the token:"));
        if (name.isEmpty())
            return;
        if (databaseModel->getDatabase()->getCardInfo(name)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The chosen name conflicts with an existing card or token.\nMake sure to enable "
                                     "the 'Token' set in the \"Manage sets\" dialog to display them correctly."));
        } else {
            break;
        }
    }

    QString setName = CardSet::TOKENS_SETNAME;
    SetToPrintingsMap sets;
    sets[setName].append(PrintingInfo(databaseModel->getDatabase()->getSet(setName)));
    CardInfoPtr card = CardInfo::newInstance(name, "", true, QVariantHash(), QList<CardRelation *>(),
                                             QList<CardRelation *>(), sets, false, false, -1, false);
    card->setCardType("Token");

    databaseModel->getDatabase()->addCard(card);
}

void DlgEditTokens::actRemoveToken()
{
    if (currentCard) {
        CardInfoPtr cardToRemove = currentCard; // the currentCard property gets modified during db->removeCard()
        currentCard.clear();
        databaseModel->getDatabase()->removeCard(cardToRemove);
    }
}

void DlgEditTokens::colorChanged(int colorIndex)
{
    if (currentCard)
        currentCard->setColors(QString(colorEdit->itemData(colorIndex).toChar()));
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
