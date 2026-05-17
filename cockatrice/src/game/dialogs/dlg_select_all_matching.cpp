#include "dlg_select_all_matching.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database_manager.h"
#include "libcockatrice/filters/filter_string.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

DlgSelectAllMatching::DlgSelectAllMatching(QWidget *parent, const SelectAllMatchingOptions &options) : QDialog(parent)
{
    exprLabel = new QLabel(tr("Card name (or search expressions):"));

    exprComboBox = new QComboBox(this);
    exprComboBox->setFocus();
    exprComboBox->setEditable(true);
    exprComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    exprComboBox->insertItems(0, options.exprs);
    exprLabel->setBuddy(exprComboBox);

    clearSelectionCheckBox = new QCheckBox(tr("Clear existing selection first"));
    clearSelectionCheckBox->setChecked(options.clearSelection);

    toggleSelectedCheckBox = new QCheckBox(tr("Toggle already selected matches"));
    toggleSelectedCheckBox->setChecked(options.toggleSelected);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgSelectAllMatching::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(exprLabel);
    mainLayout->addWidget(exprComboBox);
    mainLayout->addWidget(clearSelectionCheckBox);
    mainLayout->addWidget(toggleSelectedCheckBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Select all cards matching..."));
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

bool DlgSelectAllMatching::validateMatchExists(const FilterString &filterString)
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

void DlgSelectAllMatching::validateAndAccept()
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

QString DlgSelectAllMatching::getExpr() const
{
    return exprComboBox->currentText();
}

SelectAllMatchingOptions DlgSelectAllMatching::getOptions() const
{
    return {.exprs = getExprs(),
            .clearSelection = clearSelectionCheckBox->isChecked(),
            .toggleSelected = toggleSelectedCheckBox->isChecked()};
}

QStringList DlgSelectAllMatching::getExprs() const
{
    QStringList exprs;
    for (int i = 0; i < exprComboBox->count(); ++i) {
        exprs.append(exprComboBox->itemText(i));
    }
    return exprs;
}