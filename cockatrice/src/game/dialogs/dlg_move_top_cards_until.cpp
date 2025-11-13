#include "dlg_move_top_cards_until.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/filters/filter_string.h>

DlgMoveTopCardsUntil::DlgMoveTopCardsUntil(QWidget *parent, QStringList exprs, uint _numberOfHits, bool autoPlay)
    : QDialog(parent)
{
    exprLabel = new QLabel(tr("Card name (or search expressions):"));

    exprComboBox = new QComboBox(this);
    exprComboBox->setFocus();
    exprComboBox->setEditable(true);
    exprComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    exprComboBox->insertItems(0, exprs);
    exprLabel->setBuddy(exprComboBox);

    numberOfHitsLabel = new QLabel(tr("Number of hits:"));
    numberOfHitsEdit = new QSpinBox(this);
    numberOfHitsEdit->setRange(1, 99);
    numberOfHitsEdit->setValue(_numberOfHits);
    numberOfHitsLabel->setBuddy(numberOfHitsEdit);

    auto *grid = new QGridLayout;
    grid->addWidget(numberOfHitsLabel, 0, 0);
    grid->addWidget(numberOfHitsEdit, 0, 1);

    autoPlayCheckBox = new QCheckBox(tr("Auto play hits"));
    autoPlayCheckBox->setChecked(autoPlay);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgMoveTopCardsUntil::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(exprLabel);
    mainLayout->addWidget(exprComboBox);
    mainLayout->addItem(grid);
    mainLayout->addWidget(autoPlayCheckBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Put top cards on stack until..."));
}

/**
 * @brief Checks if a card matching the expr exists in the card database.
 *
 * @returns true if a card matching the expression exists.
 */
static bool matchExistsInDb(const FilterString &filterString)
{
    const auto *cardDatabase = CardDatabaseManager::getInstance();
    const auto &allCards = cardDatabase->getCardList();

    const auto it = std::find_if(allCards.begin(), allCards.end(),
                                 [&filterString](const CardInfoPtr &card) { return filterString.check(card); });

    return it != allCards.end();
}

/**
 * @brief Validates that a card matching the expr exists in the card database.
 * If no match is found, then pop up a window to warn the user, giving them a chance to back out.
 *
 * @returns whether to proceed with the action
 */
bool DlgMoveTopCardsUntil::validateMatchExists(const FilterString &filterString)
{
    if (matchExistsInDb(filterString)) {
        return true;
    }

    const auto msg = tr("No cards matching the search expression exists in the card database. Proceed anyways?");
    const auto res =
        QMessageBox::warning(this, tr("Cockatrice"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::No) {
        return false;
    }

    return true;
}

void DlgMoveTopCardsUntil::validateAndAccept()
{
    auto movingCardsUntilFilter = FilterString(exprComboBox->currentText());
    if (!movingCardsUntilFilter.valid()) {
        QMessageBox::warning(this, tr("Invalid filter"), movingCardsUntilFilter.error(), QMessageBox::Ok);
        return;
    }

    if (!validateMatchExists(movingCardsUntilFilter)) {
        return;
    }

    // move currently selected text to top of history list
    if (exprComboBox->currentIndex() != 0) {
        QString currentExpr = exprComboBox->currentText();
        exprComboBox->removeItem(exprComboBox->currentIndex());
        exprComboBox->insertItem(0, currentExpr);
        exprComboBox->setCurrentIndex(0);
    }

    accept();
}

QString DlgMoveTopCardsUntil::getExpr() const
{
    return exprComboBox->currentText();
}

QStringList DlgMoveTopCardsUntil::getExprs() const
{
    QStringList exprs;
    for (int i = 0; i < exprComboBox->count(); ++i) {
        exprs.append(exprComboBox->itemText(i));
    }
    return exprs;
}

uint DlgMoveTopCardsUntil::getNumberOfHits() const
{
    return numberOfHitsEdit->text().toUInt();
}

bool DlgMoveTopCardsUntil::isAutoPlay() const
{
    return autoPlayCheckBox->isChecked();
}
