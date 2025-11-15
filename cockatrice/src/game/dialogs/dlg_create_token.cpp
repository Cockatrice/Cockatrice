#include "dlg_create_token.h"

#include "../../client/settings/cache_settings.h"
#include "../../interface/widgets/cards/card_info_picture_widget.h"
#include "../../main.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/models/database/token/token_display_model.h>
#include <libcockatrice/utility/trice_limits.h>

DlgCreateToken::DlgCreateToken(const QStringList &_predefinedTokens, QWidget *parent)
    : QDialog(parent), predefinedTokens(_predefinedTokens)
{
    pic = new CardInfoPictureWidget();
    pic->setObjectName("pic");

    nameLabel = new QLabel(tr("&Name:"));
    nameEdit = new QLineEdit(tr("Token"));
    nameEdit->setMaxLength(MAX_NAME_LENGTH);
    QTimer::singleShot(100, this, [=, this]() {
        nameEdit->selectAll();
        nameEdit->setFocus();
    });

    connect(nameEdit, &QLineEdit::textChanged, this, &DlgCreateToken::updateSearch);
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
    ptEdit->setMaxLength(MAX_NAME_LENGTH);
    ptLabel->setBuddy(ptEdit);

    annotationLabel = new QLabel(tr("&Annotation:"));
    annotationEdit = new QLineEdit;
    annotationEdit->setMaxLength(MAX_NAME_LENGTH);
    annotationLabel->setBuddy(annotationEdit);

    destroyCheckBox = new QCheckBox(tr("&Destroy token when it leaves the table"));
    destroyCheckBox->setChecked(true);

    faceDownCheckBox = new QCheckBox(tr("Create face-down (Only hides name)"));
    connect(faceDownCheckBox, &QCheckBox::toggled, this, &DlgCreateToken::faceDownCheckBoxToggled);

    auto *grid = new QGridLayout;
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(nameEdit, 0, 1);
    grid->addWidget(colorLabel, 1, 0);
    grid->addWidget(colorEdit, 1, 1);
    grid->addWidget(ptLabel, 2, 0);
    grid->addWidget(ptEdit, 2, 1);
    grid->addWidget(annotationLabel, 3, 0);
    grid->addWidget(annotationEdit, 3, 1);
    grid->addWidget(destroyCheckBox, 4, 0, 1, 2);
    grid->addWidget(faceDownCheckBox, 5, 0, 1, 2);

    auto *tokenDataGroupBox = new QGroupBox(tr("Token data"));
    tokenDataGroupBox->setLayout(grid);

    cardDatabaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
    cardDatabaseDisplayModel = new TokenDisplayModel(this);
    cardDatabaseDisplayModel->setSourceModel(cardDatabaseModel);

    chooseTokenFromAllRadioButton = new QRadioButton(tr("Show &all tokens"));
    connect(chooseTokenFromAllRadioButton, &QRadioButton::toggled, this, &DlgCreateToken::actChooseTokenFromAll);
    chooseTokenFromDeckRadioButton = new QRadioButton(tr("Show tokens from this &deck"));
    connect(chooseTokenFromDeckRadioButton, &QRadioButton::toggled, this, &DlgCreateToken::actChooseTokenFromDeck);

    QByteArray deckHeaderState = SettingsCache::instance().layouts().getDeckEditorDbHeaderState();
    chooseTokenView = new QTreeView;
    chooseTokenView->setModel(cardDatabaseDisplayModel);
    chooseTokenView->setUniformRowHeights(true);
    chooseTokenView->setRootIsDecorated(false);
    chooseTokenView->setAlternatingRowColors(true);
    chooseTokenView->setSortingEnabled(true);
    chooseTokenView->sortByColumn(0, Qt::AscendingOrder);
    chooseTokenView->resizeColumnToContents(0);
    chooseTokenView->setWordWrap(true);

    if (!deckHeaderState.isNull())
        chooseTokenView->header()->restoreState(deckHeaderState);

    chooseTokenView->header()->setStretchLastSection(false);
    chooseTokenView->header()->hideSection(1);                                         // Sets
    chooseTokenView->header()->hideSection(2);                                         // Mana Cost
    chooseTokenView->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Color(s)
    connect(chooseTokenView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &DlgCreateToken::tokenSelectionChanged);
    connect(chooseTokenView, &QTreeView::doubleClicked, this, &DlgCreateToken::actOk);

    if (predefinedTokens.isEmpty()) {
        chooseTokenFromAllRadioButton->setChecked(true);
        chooseTokenFromDeckRadioButton->setDisabled(true); // No tokens in deck = no need for option
    } else {
        chooseTokenFromDeckRadioButton->setChecked(true);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>(predefinedTokens.begin(), predefinedTokens.end()));
#else
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>::fromList(predefinedTokens));
#endif
    }

    auto *tokenChooseLayout = new QVBoxLayout;
    tokenChooseLayout->addWidget(chooseTokenFromAllRadioButton);
    tokenChooseLayout->addWidget(chooseTokenFromDeckRadioButton);
    tokenChooseLayout->addWidget(chooseTokenView);

    auto *tokenChooseGroupBox = new QGroupBox(tr("Choose token from list"));
    tokenChooseGroupBox->setLayout(tokenChooseLayout);

    auto *hbox = new QGridLayout;
    hbox->addWidget(pic, 0, 0, 1, 1);
    hbox->addWidget(tokenDataGroupBox, 1, 0, 1, 1);
    hbox->addWidget(tokenChooseGroupBox, 0, 1, 2, 1);
    hbox->setColumnStretch(1, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgCreateToken::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgCreateToken::actReject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Create token"));

    resize(600, 500);
    restoreGeometry(SettingsCache::instance().getTokenDialogGeometry());
}

void DlgCreateToken::closeEvent(QCloseEvent *event)
{
    event->accept();
    SettingsCache::instance().setTokenDialogGeometry(saveGeometry());
}

void DlgCreateToken::faceDownCheckBoxToggled(bool checked)
{
    if (checked) {
        colorEdit->setCurrentIndex(6);
        colorEdit->setEnabled(false);
        ptEdit->clear();
        ptEdit->clearFocus();
        ptEdit->setEnabled(false);
    } else {
        colorEdit->setEnabled(true);
        ptEdit->setEnabled(true);
        annotationEdit->setEnabled(true);
    }
}

void DlgCreateToken::tokenSelectionChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QModelIndex realIndex = cardDatabaseDisplayModel->mapToSource(current);

    CardInfoPtr cardInfo;

    if (current.row() >= 0) {
        cardInfo = cardDatabaseModel->getCard(realIndex.row());
    }

    if (cardInfo) {
        updateSearchFieldWithoutUpdatingFilter(cardInfo->getName());
        const QChar cardColor = cardInfo->getColorChar();
        colorEdit->setCurrentIndex(colorEdit->findData(cardColor, Qt::UserRole, Qt::MatchFixedString));
        ptEdit->setText(cardInfo->getPowTough());
        if (SettingsCache::instance().getAnnotateTokens())
            annotationEdit->setText(cardInfo->getText());
    } else {
        nameEdit->setText("");
        colorEdit->setCurrentIndex(colorEdit->findData(QString(), Qt::UserRole, Qt::MatchFixedString));
        ptEdit->setText("");
        annotationEdit->setText("");
    }

    pic->setCard(CardDatabaseManager::query()->getPreferredCard(cardInfo));
}

void DlgCreateToken::updateSearchFieldWithoutUpdatingFilter(const QString &newValue) const
{
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
    if (checked) {
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>());
    }
}

void DlgCreateToken::actChooseTokenFromDeck(bool checked)
{
    if (checked) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>(predefinedTokens.begin(), predefinedTokens.end()));
#else
        cardDatabaseDisplayModel->setCardNameSet(QSet<QString>::fromList(predefinedTokens));
#endif
    }
}

void DlgCreateToken::actOk()
{
    SettingsCache::instance().setTokenDialogGeometry(saveGeometry());
    accept();
}

void DlgCreateToken::actReject()
{
    SettingsCache::instance().setTokenDialogGeometry(saveGeometry());
    reject();
}

TokenInfo DlgCreateToken::getTokenInfo() const
{
    return {.name = nameEdit->text(),
            .color = colorEdit->itemData(colorEdit->currentIndex()).toString(),
            .pt = ptEdit->text(),
            .annotation = annotationEdit->text(),
            .destroy = destroyCheckBox->isChecked(),
            .faceDown = faceDownCheckBox->isChecked(),
            .providerId = SettingsCache::instance().cardOverrides().getCardPreferenceOverride(nameEdit->text())};
}
