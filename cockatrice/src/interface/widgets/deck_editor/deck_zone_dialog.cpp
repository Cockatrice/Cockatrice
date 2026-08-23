#include "deck_zone_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <libcockatrice/utility/string_limits.h>

DeckZoneDialog::DeckZoneDialog(QWidget *parent,
                               const QString &initialBoardName,
                               const std::function<QString(const QString &)> &_nameValidator,
                               bool _allowBoardSelection)
    : QDialog(parent), nameValidator(_nameValidator), allowBoardSelection(_allowBoardSelection)
{
    nameLabel = new QLabel(this);
    nameEdit = new QLineEdit(this);
    nameEdit->setMaxLength(MAX_NAME_LENGTH);

    errorLabel = new QLabel(this);
    errorLabel->hide();

    boardLabel = new QLabel(this);
    boardCombo = new QComboBox(this);
    for (const QString &boardName : InnerDecklistNode::boardZoneNames()) {
        // Use the icon overload explicitly so `boardName` lands in the user data role
        // (visible text is applied below in retranslateUi). The two-argument form
        // addItem({}, boardName) would be ambiguous and resolve to the icon overload
        // with empty user data, yielding empty entries and an empty getBoardName().
        boardCombo->addItem({}, {}, boardName);
    }
    if (!initialBoardName.isEmpty()) {
        int idx = boardCombo->findData(initialBoardName);
        if (idx != -1) {
            boardCombo->setCurrentIndex(idx);
        }
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);
    layout->addWidget(errorLabel);
    if (allowBoardSelection) {
        layout->addWidget(boardLabel);
        layout->addWidget(boardCombo);
    }
    layout->addWidget(buttonBox);

    retranslateUi();

    connect(nameEdit, &QLineEdit::textChanged, this, [this] { validateName(); });
    validateName();

    nameEdit->setFocus();
}

QString DeckZoneDialog::getZoneName() const
{
    return nameEdit->text().trimmed();
}

QString DeckZoneDialog::getBoardName() const
{
    return boardCombo->currentData().toString();
}

void DeckZoneDialog::setZoneName(const QString &zoneName)
{
    nameEdit->setText(zoneName);
    nameEdit->selectAll();
}

void DeckZoneDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);

    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

void DeckZoneDialog::retranslateUi()
{
    setWindowTitle(allowBoardSelection ? tr("New zone") : tr("Rename zone"));

    nameLabel->setText(tr("Zone &name:"));
    nameLabel->setBuddy(nameEdit);

    boardLabel->setText(tr("&Parent zone:"));
    boardLabel->setBuddy(boardCombo);

    for (int i = 0; i < boardCombo->count(); i++) {
        boardCombo->setItemText(i, InnerDecklistNode::visibleNameFromName(boardCombo->itemData(i).toString()));
    }
}

void DeckZoneDialog::validateName()
{
    const QString zoneName = nameEdit->text().trimmed();
    QString error;
    if (zoneName.isEmpty()) {
        error = tr("Enter a zone name.");
    } else if (nameValidator) {
        error = nameValidator(zoneName);
    }

    errorLabel->setText(error);
    errorLabel->setVisible(!error.isEmpty());
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(error.isEmpty());
}

QString DeckZoneDialog::promptForNewZone(QWidget *parent,
                                         const QString &initialBoardName,
                                         QString *chosenBoardName,
                                         const std::function<QString(const QString &)> &nameValidator)
{
    DeckZoneDialog dialog(parent, initialBoardName, nameValidator);
    if (dialog.exec() != QDialog::Accepted) {
        return {};
    }

    if (chosenBoardName) {
        *chosenBoardName = dialog.getBoardName();
    }
    return dialog.getZoneName();
}

QString DeckZoneDialog::promptForRename(QWidget *parent,
                                        const QString &currentZoneName,
                                        const std::function<QString(const QString &)> &nameValidator)
{
    DeckZoneDialog dialog(parent, {}, nameValidator, false);
    dialog.setZoneName(currentZoneName);
    return dialog.exec() == QDialog::Accepted ? dialog.getZoneName() : QString();
}
