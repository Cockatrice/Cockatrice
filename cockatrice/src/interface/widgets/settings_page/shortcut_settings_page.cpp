#include "shortcut_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcut_treeview.h"
#include "../interface/widgets/utility/custom_line_edit.h"
#include "../interface/widgets/utility/sequence_edit.h"

#include <QAbstractItemView>
#include <QMessageBox>

ShortcutSettingsPage::ShortcutSettingsPage()
{
    // search bar
    searchEdit = new SearchLineEdit;
    searchEdit->setObjectName("searchEdit");
    searchEdit->setClearButtonEnabled(true);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    // table
    shortcutsTable = new ShortcutTreeView(this);

    shortcutsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    shortcutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    shortcutsTable->setColumnWidth(0, width() / 3 * 2);
    searchEdit->setTreeView(shortcutsTable);

    connect(searchEdit, &SearchLineEdit::textChanged, shortcutsTable, &ShortcutTreeView::updateSearchString);

    // edit widget
    currentActionGroupLabel = new QLabel(this);
    currentActionGroupName = new QLabel(this);
    currentActionLabel = new QLabel(this);
    currentActionName = new QLabel(this);
    currentShortcutLabel = new QLabel(this);
    editTextBox = new SequenceEdit("", this);
    shortcutsTable->installEventFilter(editTextBox);

    // buttons
    faqLabel = new QLabel(this);
    faqLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    faqLabel->setOpenExternalLinks(true);

    btnResetAll = new QPushButton(this);
    btnClearAll = new QPushButton(this);

    btnResetAll->setIcon(QPixmap("theme:icons/update"));
    btnClearAll->setIcon(QPixmap("theme:icons/clearsearch"));

    // layout
    auto *_editLayout = new QGridLayout;
    _editLayout->addWidget(currentActionGroupLabel, 0, 0);
    _editLayout->addWidget(currentActionGroupName, 0, 1);
    _editLayout->addWidget(currentActionLabel, 1, 0);
    _editLayout->addWidget(currentActionName, 1, 1);
    _editLayout->addWidget(currentShortcutLabel, 2, 0);
    _editLayout->addWidget(editTextBox, 2, 1);

    editShortcutGroupBox = new QGroupBox;
    editShortcutGroupBox->setLayout(_editLayout);

    auto *_buttonsLayout = new QHBoxLayout;
    _buttonsLayout->addWidget(faqLabel);
    _buttonsLayout->addWidget(btnResetAll);
    _buttonsLayout->addWidget(btnClearAll);

    auto *_mainLayout = new QVBoxLayout;
    _mainLayout->addWidget(searchEdit);
    _mainLayout->addWidget(shortcutsTable);
    _mainLayout->addWidget(editShortcutGroupBox);
    _mainLayout->addLayout(_buttonsLayout);

    setLayout(_mainLayout);

    connect(btnResetAll, &QPushButton::clicked, this, &ShortcutSettingsPage::resetShortcuts);
    connect(btnClearAll, &QPushButton::clicked, this, &ShortcutSettingsPage::clearShortcuts);

    connect(shortcutsTable, &ShortcutTreeView::currentItemChanged, this, &ShortcutSettingsPage::currentItemChanged);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &ShortcutSettingsPage::retranslateUi);
    retranslateUi();
}

void ShortcutSettingsPage::currentItemChanged(const QString &key)
{
    if (key.isEmpty()) {
        currentActionGroupName->setText("");
        currentActionName->setText("");
        editTextBox->setShortcutName("");
    } else {
        QString group = SettingsCache::instance().shortcuts().getShortcut(key).getGroupName();
        QString action = SettingsCache::instance().shortcuts().getShortcut(key).getName();
        currentActionGroupName->setText(group);
        currentActionName->setText(action);
        editTextBox->setShortcutName(key);
    }
}

void ShortcutSettingsPage::resetShortcuts()
{
    if (QMessageBox::question(this, tr("Restore all default shortcuts"),
                              tr("Do you really want to restore all default shortcuts?")) == QMessageBox::Yes) {
        SettingsCache::instance().shortcuts().resetAllShortcuts();
    }
}

void ShortcutSettingsPage::clearShortcuts()
{
    if (QMessageBox::question(this, tr("Clear all default shortcuts"),
                              tr("Do you really want to clear all shortcuts?")) == QMessageBox::Yes) {
        SettingsCache::instance().shortcuts().clearAllShortcuts();
    }
}

void ShortcutSettingsPage::retranslateUi()
{
    shortcutsTable->retranslateUi();

    currentActionGroupLabel->setText(tr("Section:"));
    currentActionLabel->setText(tr("Action:"));
    currentShortcutLabel->setText(tr("Shortcut:"));
    editTextBox->retranslateUi();
    faqLabel->setText(QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_SHORTCUTS).arg(tr("How to set custom shortcuts")));
    btnResetAll->setText(tr("Restore all default shortcuts"));
    btnClearAll->setText(tr("Clear all shortcuts"));
    searchEdit->setPlaceholderText(tr("Search by shortcut name"));
}
