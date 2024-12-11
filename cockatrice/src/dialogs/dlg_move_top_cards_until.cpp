#include "dlg_move_top_cards_until.h"

#include "../game/filters/filter_string.h"
#include "trice_limits.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

DlgMoveTopCardsUntil::DlgMoveTopCardsUntil(QWidget *parent, QString _expr, uint _numberOfHits) : QDialog(parent)
{
    exprLabel = new QLabel(tr("Card name (or search expressions):"));

    exprEdit = new QLineEdit(this);
    exprEdit->setFocus();
    exprEdit->setText(_expr);
    exprLabel->setBuddy(exprEdit);

    numberOfHitsLabel = new QLabel(tr("Number of hits:"));
    numberOfHitsEdit = new QSpinBox(this);
    numberOfHitsEdit->setRange(1, 99);
    numberOfHitsEdit->setValue(_numberOfHits);
    numberOfHitsLabel->setBuddy(numberOfHitsEdit);

    auto *grid = new QGridLayout;
    grid->addWidget(numberOfHitsLabel, 0, 0);
    grid->addWidget(numberOfHitsEdit, 0, 1);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgMoveTopCardsUntil::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(exprLabel);
    mainLayout->addWidget(exprEdit);
    mainLayout->addItem(grid);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Put top cards on stack until..."));
}

void DlgMoveTopCardsUntil::validateAndAccept()
{
    auto movingCardsUntilFilter = FilterString(exprEdit->text());
    if (movingCardsUntilFilter.valid()) {
        accept();
    } else {
        QMessageBox::warning(this, tr("Invalid filter"), movingCardsUntilFilter.error(), QMessageBox::Ok);
    }
}

QString DlgMoveTopCardsUntil::getExpr() const
{
    return exprEdit->text();
}

uint DlgMoveTopCardsUntil::getNumberOfHits() const
{
    return numberOfHitsEdit->text().toUInt();
}
