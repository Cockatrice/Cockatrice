#include "messages_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/widgets/utility/get_text_with_max.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QToolBar>

MessagesSettingsPage::MessagesSettingsPage()
{
    chatMentionCheckBox.setChecked(SettingsCache::instance().getChatMention());
    connect(&chatMentionCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setChatMention);

    chatMentionCompleterCheckbox.setChecked(SettingsCache::instance().getChatMentionCompleter());
    connect(&chatMentionCompleterCheckbox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setChatMentionCompleter);

    explainMessagesLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    explainMessagesLabel.setOpenExternalLinks(true);

    ignoreUnregUsersMainChat.setChecked(SettingsCache::instance().getIgnoreUnregisteredUsers());
    ignoreUnregUserMessages.setChecked(SettingsCache::instance().getIgnoreUnregisteredUserMessages());
    connect(&ignoreUnregUsersMainChat, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setIgnoreUnregisteredUsers);
    connect(&ignoreUnregUserMessages, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setIgnoreUnregisteredUserMessages);

    invertMentionForeground.setChecked(SettingsCache::instance().getChatMentionForeground());
    connect(&invertMentionForeground, &QCheckBox::QT_STATE_CHANGED, this, &MessagesSettingsPage::updateTextColor);

    invertHighlightForeground.setChecked(SettingsCache::instance().getChatHighlightForeground());
    connect(&invertHighlightForeground, &QCheckBox::QT_STATE_CHANGED, this,
            &MessagesSettingsPage::updateTextHighlightColor);

    mentionColor = new QLineEdit();
    mentionColor->setText(SettingsCache::instance().getChatMentionColor());
    updateMentionPreview();
    connect(mentionColor, &QLineEdit::textChanged, this, &MessagesSettingsPage::updateColor);

    messagePopups.setChecked(SettingsCache::instance().getShowMessagePopup());
    connect(&messagePopups, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setShowMessagePopups);

    mentionPopups.setChecked(SettingsCache::instance().getShowMentionPopup());
    connect(&mentionPopups, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setShowMentionPopups);

    roomHistory.setChecked(SettingsCache::instance().getRoomHistory());
    connect(&roomHistory, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(), &SettingsCache::setRoomHistory);

    customAlertString = new QLineEdit();
    customAlertString->setText(SettingsCache::instance().getHighlightWords());
    connect(customAlertString, &QLineEdit::textChanged, &SettingsCache::instance(), &SettingsCache::setHighlightWords);

    auto *chatGrid = new QGridLayout;
    chatGrid->addWidget(&chatMentionCheckBox, 0, 0);
    chatGrid->addWidget(&invertMentionForeground, 0, 1);
    chatGrid->addWidget(mentionColor, 0, 2);
    chatGrid->addWidget(&chatMentionCompleterCheckbox, 1, 0);
    chatGrid->addWidget(&ignoreUnregUsersMainChat, 2, 0);
    chatGrid->addWidget(&hexLabel, 1, 2);
    chatGrid->addWidget(&ignoreUnregUserMessages, 3, 0);
    chatGrid->addWidget(&messagePopups, 4, 0);
    chatGrid->addWidget(&mentionPopups, 5, 0);
    chatGrid->addWidget(&roomHistory, 6, 0);
    chatGroupBox = new QGroupBox;
    chatGroupBox->setLayout(chatGrid);

    highlightColor = new QLineEdit();
    highlightColor->setText(SettingsCache::instance().getChatHighlightColor());
    updateHighlightPreview();
    connect(highlightColor, &QLineEdit::textChanged, this, &MessagesSettingsPage::updateHighlightColor);

    auto *highlightNotice = new QGridLayout;
    highlightNotice->addWidget(highlightColor, 0, 2);
    highlightNotice->addWidget(&invertHighlightForeground, 0, 1);
    highlightNotice->addWidget(&hexHighlightLabel, 1, 2);
    highlightNotice->addWidget(customAlertString, 0, 0);
    highlightNotice->addWidget(&customAlertStringLabel, 1, 0);
    highlightGroupBox = new QGroupBox;
    highlightGroupBox->setLayout(highlightNotice);

    messageList = new QListWidget;

    int count = SettingsCache::instance().messages().getCount();
    for (int i = 0; i < count; i++) {
        messageList->addItem(SettingsCache::instance().messages().getMessageAt(i));
    }

    aAdd = new QAction(this);
    aAdd->setIcon(QPixmap("theme:icons/increment"));
    connect(aAdd, &QAction::triggered, this, &MessagesSettingsPage::actAdd);

    aEdit = new QAction(this);
    aEdit->setIcon(QPixmap("theme:icons/pencil"));
    connect(aEdit, &QAction::triggered, this, &MessagesSettingsPage::actEdit);

    aRemove = new QAction(this);
    aRemove->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemove, &QAction::triggered, this, &MessagesSettingsPage::actRemove);

    auto *messageToolBar = new QToolBar;
    messageToolBar->setOrientation(Qt::Vertical);
    messageToolBar->addAction(aAdd);
    messageToolBar->addAction(aRemove);
    messageToolBar->addAction(aEdit);
    messageToolBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    auto *messageListLayout = new QHBoxLayout;
    messageListLayout->addWidget(messageToolBar);
    messageListLayout->addWidget(messageList);

    auto *messagesLayout = new QVBoxLayout; // combines the explainer label with the actual messages widget pieces
    messagesLayout->addLayout(messageListLayout);
    messagesLayout->addWidget(&explainMessagesLabel);

    messageGroupBox = new QGroupBox; // draws a box around the above layout and allows it to be titled
    messageGroupBox->setLayout(messagesLayout);

    auto *mainLayout = new QVBoxLayout; // combines the messages groupbox with the rest of the menu
    mainLayout->addWidget(messageGroupBox);
    mainLayout->addWidget(chatGroupBox);
    mainLayout->addWidget(highlightGroupBox);

    setLayout(mainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &MessagesSettingsPage::retranslateUi);
    retranslateUi();
}

void MessagesSettingsPage::updateColor(const QString &value)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor colorToSet = QColor::fromString("#" + value);
#else
    QColor colorToSet;
    colorToSet.setNamedColor("#" + value);
#endif
    if (colorToSet.isValid()) {
        SettingsCache::instance().setChatMentionColor(value);
        updateMentionPreview();
    }
}

void MessagesSettingsPage::updateHighlightColor(const QString &value)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor colorToSet = QColor::fromString("#" + value);
#else
    QColor colorToSet;
    colorToSet.setNamedColor("#" + value);
#endif
    if (colorToSet.isValid()) {
        SettingsCache::instance().setChatHighlightColor(value);
        updateHighlightPreview();
    }
}

void MessagesSettingsPage::updateTextColor(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setChatMentionForeground(value);
    updateMentionPreview();
}

void MessagesSettingsPage::updateTextHighlightColor(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setChatHighlightForeground(value);
    updateHighlightPreview();
}

void MessagesSettingsPage::updateMentionPreview()
{
    mentionColor->setStyleSheet(
        "QLineEdit{background:#" + SettingsCache::instance().getChatMentionColor() +
        ";color: " + (SettingsCache::instance().getChatMentionForeground() ? "white" : "black") + ";}");
}

void MessagesSettingsPage::updateHighlightPreview()
{
    highlightColor->setStyleSheet(
        "QLineEdit{background:#" + SettingsCache::instance().getChatHighlightColor() +
        ";color: " + (SettingsCache::instance().getChatHighlightForeground() ? "white" : "black") + ";}");
}

void MessagesSettingsPage::storeSettings()
{
    SettingsCache::instance().messages().setCount(messageList->count());
    for (int i = 0; i < messageList->count(); i++) {
        SettingsCache::instance().messages().setMessageAt(i, messageList->item(i)->text());
    }
    emit SettingsCache::instance().messages().messageMacrosChanged();
}

void MessagesSettingsPage::actAdd()
{
    bool ok;
    QString msg =
        getTextWithMax(this, tr("Add message"), tr("Message:"), QLineEdit::Normal, QString(), &ok, MAX_TEXT_LENGTH);
    if (ok) {
        messageList->addItem(msg);
        storeSettings();
    }
}

void MessagesSettingsPage::actEdit()
{
    if (messageList->currentItem()) {
        QString oldText = messageList->currentItem()->text();
        bool ok;
        QString msg =
            getTextWithMax(this, tr("Edit message"), tr("Message:"), QLineEdit::Normal, oldText, &ok, MAX_TEXT_LENGTH);
        if (ok) {
            messageList->currentItem()->setText(msg);
            storeSettings();
        }
    }
}

void MessagesSettingsPage::actRemove()
{
    if (messageList->currentItem() != nullptr) {
        delete messageList->takeItem(messageList->currentRow());
        storeSettings();
    }
}

void MessagesSettingsPage::retranslateUi()
{
    chatGroupBox->setTitle(tr("Chat settings"));
    highlightGroupBox->setTitle(tr("Custom alert words"));
    chatMentionCheckBox.setText(tr("Enable chat mentions"));
    chatMentionCompleterCheckbox.setText(tr("Enable mention completer"));
    messageGroupBox->setTitle(tr("In-game message macros"));
    explainMessagesLabel.setText(
        QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_SHORTCUTS).arg(tr("How to use in-game message macros")));
    ignoreUnregUsersMainChat.setText(tr("Ignore chat room messages sent by unregistered users"));
    ignoreUnregUserMessages.setText(tr("Ignore private messages sent by unregistered users"));
    invertMentionForeground.setText(tr("Invert text color"));
    invertHighlightForeground.setText(tr("Invert text color"));
    messagePopups.setText(tr("Enable desktop notifications for private messages"));
    mentionPopups.setText(tr("Enable desktop notification for mentions"));
    roomHistory.setText(tr("Enable room message history on join"));
    hexLabel.setText(tr("(Color is hexadecimal)"));
    hexHighlightLabel.setText(tr("(Color is hexadecimal)"));
    customAlertStringLabel.setText(tr("Separate words with a space, alphanumeric characters only"));
    customAlertString->setPlaceholderText(tr("Word1 Word2 Word3"));
    aAdd->setText(tr("Add New Message"));
    aEdit->setText(tr("Edit Message"));
    aRemove->setText(tr("Remove Message"));
}