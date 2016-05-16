#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolBar>
#include <QTranslator>
#include <QAction>
#include <QApplication>
#include <QInputDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QDebug>
#include <QSlider>
#include <QSpinBox>
#include "carddatabase.h"
#include "dlg_settings.h"
#include "main.h"
#include "settingscache.h"
#include "thememanager.h"
#include "priceupdater.h"
#include "soundengine.h"
#include "sequenceEdit/shortcutstab.h"

#define WIKI_CUSTOM_PIC_URL "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Download-URLs"

GeneralSettingsPage::GeneralSettingsPage()
{
    QString setLanguage = settingsCache->getLang();
    QStringList qmFiles = findQmFiles();
    for (int i = 0; i < qmFiles.size(); i++) {
        QString langName = languageName(qmFiles[i]);
        languageBox.addItem(langName, qmFiles[i]);
        if ((qmFiles[i] == setLanguage) || (setLanguage.isEmpty() && langName == QCoreApplication::translate("i18n", DEFAULT_LANG_NAME)))
            languageBox.setCurrentIndex(i);
    }

    picDownloadCheckBox.setChecked(settingsCache->getPicDownload());
    updateNotificationCheckBox.setChecked(settingsCache->getNotifyAboutUpdates());

    pixmapCacheEdit.setMinimum(PIXMAPCACHE_SIZE_MIN);
    // 2047 is the max value to avoid overflowing of QPixmapCache::setCacheLimit(int size)
    pixmapCacheEdit.setMaximum(PIXMAPCACHE_SIZE_MAX);
    pixmapCacheEdit.setSingleStep(64);
    pixmapCacheEdit.setValue(settingsCache->getPixmapCacheSize());
    pixmapCacheEdit.setSuffix(" MB");
    
    defaultUrlEdit = new QLineEdit(settingsCache->getPicUrl());
    fallbackUrlEdit = new QLineEdit(settingsCache->getPicUrlFallback());

    connect(&clearDownloadedPicsButton, SIGNAL(clicked()), this, SLOT(clearDownloadedPicsButtonClicked()));
    connect(&languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));
    connect(&picDownloadCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPicDownload(int)));
    connect(&pixmapCacheEdit, SIGNAL(valueChanged(int)), settingsCache, SLOT(setPixmapCacheSize(int)));
    connect(&updateNotificationCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setNotifyAboutUpdate(int)));
    connect(&picDownloadCheckBox, SIGNAL(clicked(bool)), this, SLOT(setEnabledStatus(bool)));
    connect(defaultUrlEdit, SIGNAL(textChanged(QString)), settingsCache, SLOT(setPicUrl(QString)));
    connect(fallbackUrlEdit, SIGNAL(textChanged(QString)), settingsCache, SLOT(setPicUrlFallback(QString)));
    connect(&defaultUrlRestoreButton, SIGNAL(clicked()), this, SLOT(defaultUrlRestoreButtonClicked()));
    connect(&fallbackUrlRestoreButton, SIGNAL(clicked()), this, SLOT(fallbackUrlRestoreButtonClicked()));

    setEnabledStatus(settingsCache->getPicDownload());

    QGridLayout *personalGrid = new QGridLayout;
    personalGrid->addWidget(&languageLabel, 0, 0);
    personalGrid->addWidget(&languageBox, 0, 1);
    personalGrid->addWidget(&pixmapCacheLabel, 1, 0);
    personalGrid->addWidget(&pixmapCacheEdit, 1, 1);
    personalGrid->addWidget(&updateNotificationCheckBox, 2, 0);
    personalGrid->addWidget(&picDownloadCheckBox, 3, 0, 1, 3);
    personalGrid->addWidget(&defaultUrlLabel, 4, 0, 1, 1);
    personalGrid->addWidget(defaultUrlEdit, 4, 1, 1, 1);
    personalGrid->addWidget(&defaultUrlRestoreButton, 4, 2, 1, 1);
    personalGrid->addWidget(&fallbackUrlLabel, 5, 0, 1, 1);
    personalGrid->addWidget(fallbackUrlEdit, 5, 1, 1, 1);
    personalGrid->addWidget(&fallbackUrlRestoreButton, 5, 2, 1, 1);
    personalGrid->addWidget(&urlLinkLabel, 6, 1, 1, 1);
    personalGrid->addWidget(&clearDownloadedPicsButton, 7, 0, 1, 3);
    
    urlLinkLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    urlLinkLabel.setOpenExternalLinks(true);

    personalGroupBox = new QGroupBox;
    personalGroupBox->setLayout(personalGrid);

    deckPathEdit = new QLineEdit(settingsCache->getDeckPath());
    deckPathEdit->setReadOnly(true);
    QPushButton *deckPathButton = new QPushButton("...");
    connect(deckPathButton, SIGNAL(clicked()), this, SLOT(deckPathButtonClicked()));

    replaysPathEdit = new QLineEdit(settingsCache->getReplaysPath());
    replaysPathEdit->setReadOnly(true);
    QPushButton *replaysPathButton = new QPushButton("...");
    connect(replaysPathButton, SIGNAL(clicked()), this, SLOT(replaysPathButtonClicked()));
    
    picsPathEdit = new QLineEdit(settingsCache->getPicsPath());
    picsPathEdit->setReadOnly(true);
    QPushButton *picsPathButton = new QPushButton("...");
    connect(picsPathButton, SIGNAL(clicked()), this, SLOT(picsPathButtonClicked()));
    
    cardDatabasePathEdit = new QLineEdit(settingsCache->getCardDatabasePath());
    cardDatabasePathEdit->setReadOnly(true);
    QPushButton *cardDatabasePathButton = new QPushButton("...");
    connect(cardDatabasePathButton, SIGNAL(clicked()), this, SLOT(cardDatabasePathButtonClicked()));
    
    tokenDatabasePathEdit = new QLineEdit(settingsCache->getTokenDatabasePath());
    tokenDatabasePathEdit->setReadOnly(true);
    QPushButton *tokenDatabasePathButton = new QPushButton("...");
    connect(tokenDatabasePathButton, SIGNAL(clicked()), this, SLOT(tokenDatabasePathButtonClicked()));
    
    QGridLayout *pathsGrid = new QGridLayout;
    pathsGrid->addWidget(&deckPathLabel, 0, 0);
    pathsGrid->addWidget(deckPathEdit, 0, 1);
    pathsGrid->addWidget(deckPathButton, 0, 2);
    pathsGrid->addWidget(&replaysPathLabel, 1, 0);
    pathsGrid->addWidget(replaysPathEdit, 1, 1);
    pathsGrid->addWidget(replaysPathButton, 1, 2);
    pathsGrid->addWidget(&picsPathLabel, 2, 0);
    pathsGrid->addWidget(picsPathEdit, 2, 1);
    pathsGrid->addWidget(picsPathButton, 2, 2);
    pathsGrid->addWidget(&cardDatabasePathLabel, 3, 0);
    pathsGrid->addWidget(cardDatabasePathEdit, 3, 1);
    pathsGrid->addWidget(cardDatabasePathButton, 3, 2);
    pathsGrid->addWidget(&tokenDatabasePathLabel, 4, 0);
    pathsGrid->addWidget(tokenDatabasePathEdit, 4, 1);
    pathsGrid->addWidget(tokenDatabasePathButton, 4, 2);
    pathsGroupBox = new QGroupBox;
    pathsGroupBox->setLayout(pathsGrid);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(personalGroupBox);
    mainLayout->addWidget(pathsGroupBox);
    
    setLayout(mainLayout);
}

QStringList GeneralSettingsPage::findQmFiles()
{
    QDir dir(translationPath);
    QStringList fileNames = dir.entryList(QStringList(translationPrefix + "_*.qm"), QDir::Files, QDir::Name);
    fileNames.replaceInStrings(QRegExp(translationPrefix + "_(.*)\\.qm"), "\\1");
    return fileNames;
}

QString GeneralSettingsPage::languageName(const QString &qmFile)
{
    if(qmFile == DEFAULT_LANG_CODE)
        return DEFAULT_LANG_NAME;

    QTranslator translator;
    translator.load(translationPrefix + "_" + qmFile + ".qm", translationPath);
    
    return translator.translate("i18n", DEFAULT_LANG_NAME);
}

void GeneralSettingsPage::defaultUrlRestoreButtonClicked()
{
    QString path = PIC_URL_DEFAULT;
    defaultUrlEdit->setText(path);
    settingsCache->setPicUrl(path);
}

void GeneralSettingsPage::fallbackUrlRestoreButtonClicked()
{
    QString path = PIC_URL_FALLBACK;
    fallbackUrlEdit->setText(path);
    settingsCache->setPicUrlFallback(path);
}

void GeneralSettingsPage::deckPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    deckPathEdit->setText(path);
    settingsCache->setDeckPath(path);
}

void GeneralSettingsPage::replaysPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    replaysPathEdit->setText(path);
    settingsCache->setReplaysPath(path);
}

void GeneralSettingsPage::picsPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    picsPathEdit->setText(path);
    settingsCache->setPicsPath(path);
}

void GeneralSettingsPage::clearDownloadedPicsButtonClicked()
{
    QString picsPath = settingsCache->getPicsPath() + "/downloadedPics/";
    QStringList dirs = QDir(picsPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    bool outerSuccessRemove = true;
    for (int i = 0; i < dirs.length(); i ++) {
        QString currentPath = picsPath + dirs.at(i) + "/";
        QStringList files = QDir(currentPath).entryList(QDir::Files);
        bool innerSuccessRemove = true;
        for (int j = 0; j < files.length(); j ++)
            if (!QDir(currentPath).remove(files.at(j))) {
                qDebug() << "Failed to remove " + currentPath.toUtf8() + files.at(j).toUtf8();
                outerSuccessRemove = false;
                innerSuccessRemove = false;
            }
        if (innerSuccessRemove)
            QDir(picsPath).rmdir(dirs.at(i));
    }
    if (outerSuccessRemove)
        QMessageBox::information(this, tr("Success"), tr("Downloaded card pictures have been reset."));
    else
        QMessageBox::critical(this, tr("Error"), tr("One or more downloaded card pictures could not be cleared."));
}

void GeneralSettingsPage::cardDatabasePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    cardDatabasePathEdit->setText(path);
    settingsCache->setCardDatabasePath(path);
}

void GeneralSettingsPage::tokenDatabasePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    tokenDatabasePathEdit->setText(path);
    settingsCache->setTokenDatabasePath(path);
}

void GeneralSettingsPage::languageBoxChanged(int index)
{
    settingsCache->setLang(languageBox.itemData(index).toString());
}

void GeneralSettingsPage::retranslateUi()
{
    personalGroupBox->setTitle(tr("Personal settings"));
    languageLabel.setText(tr("Language:"));
    picDownloadCheckBox.setText(tr("Download card pictures on the fly"));
    pathsGroupBox->setTitle(tr("Paths"));
    deckPathLabel.setText(tr("Decks directory:"));
    replaysPathLabel.setText(tr("Replays directory:"));
    picsPathLabel.setText(tr("Pictures directory:"));
    cardDatabasePathLabel.setText(tr("Card database:"));
    tokenDatabasePathLabel.setText(tr("Token database:"));
    pixmapCacheLabel.setText(tr("Picture cache size:"));
    defaultUrlLabel.setText(tr("Primary download URL:"));
    fallbackUrlLabel.setText(tr("Fallback download URL:"));
    urlLinkLabel.setText(QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_PIC_URL).arg(tr("How to set a custom picture url")));
    clearDownloadedPicsButton.setText(tr("Reset/Clear Downloaded Pictures"));
    updateNotificationCheckBox.setText(tr("Notify when new client features are available"));
    defaultUrlRestoreButton.setText(tr("Reset"));
    fallbackUrlRestoreButton.setText(tr("Reset"));
}

void GeneralSettingsPage::setEnabledStatus(bool status)
{
    defaultUrlEdit->setEnabled(status);
    fallbackUrlEdit->setEnabled(status);
    defaultUrlRestoreButton.setEnabled(status);
    fallbackUrlRestoreButton.setEnabled(status);
}

AppearanceSettingsPage::AppearanceSettingsPage()
{
    QString themeName = settingsCache->getThemeName();

    QStringList themeDirs = themeManager->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName)
            themeBox.setCurrentIndex(i);
    }

    connect(&themeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(themeBoxChanged(int)));
    
    QGridLayout *themeGrid = new QGridLayout;
    themeGrid->addWidget(&themeLabel, 0, 0);
    themeGrid->addWidget(&themeBox, 0, 1);

    themeGroupBox = new QGroupBox;
    themeGroupBox->setLayout(themeGrid);

    displayCardNamesCheckBox.setChecked(settingsCache->getDisplayCardNames());
    connect(&displayCardNamesCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setDisplayCardNames(int)));

    cardScalingCheckBox.setChecked(settingsCache->getScaleCards());
    connect(&cardScalingCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setCardScaling(int)));
    
    QGridLayout *cardsGrid = new QGridLayout;
    cardsGrid->addWidget(&displayCardNamesCheckBox, 0, 0, 1, 2);
    cardsGrid->addWidget(&cardScalingCheckBox, 1, 0, 1, 2);
    
    cardsGroupBox = new QGroupBox;
    cardsGroupBox->setLayout(cardsGrid);
    
    horizontalHandCheckBox.setChecked(settingsCache->getHorizontalHand());
    connect(&horizontalHandCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setHorizontalHand(int)));

    leftJustifiedHandCheckBox.setChecked(settingsCache->getLeftJustified());
    connect(&leftJustifiedHandCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setLeftJustified(int)));
    
    QGridLayout *handGrid = new QGridLayout;
    handGrid->addWidget(&horizontalHandCheckBox, 0, 0, 1, 2);
    handGrid->addWidget(&leftJustifiedHandCheckBox, 1, 0, 1, 2);
    
    handGroupBox = new QGroupBox;
    handGroupBox->setLayout(handGrid);
    
    invertVerticalCoordinateCheckBox.setChecked(settingsCache->getInvertVerticalCoordinate());
    connect(&invertVerticalCoordinateCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setInvertVerticalCoordinate(int)));
    
    minPlayersForMultiColumnLayoutEdit.setMinimum(2);
    minPlayersForMultiColumnLayoutEdit.setValue(settingsCache->getMinPlayersForMultiColumnLayout());
    connect(&minPlayersForMultiColumnLayoutEdit, SIGNAL(valueChanged(int)), settingsCache, SLOT(setMinPlayersForMultiColumnLayout(int)));
    minPlayersForMultiColumnLayoutLabel.setBuddy(&minPlayersForMultiColumnLayoutEdit);
    
    QGridLayout *tableGrid = new QGridLayout;
    tableGrid->addWidget(&invertVerticalCoordinateCheckBox, 0, 0, 1, 2);
    tableGrid->addWidget(&minPlayersForMultiColumnLayoutLabel, 1, 0, 1, 1);
    tableGrid->addWidget(&minPlayersForMultiColumnLayoutEdit, 1, 1, 1, 1);
    
    tableGroupBox = new QGroupBox;
    tableGroupBox->setLayout(tableGrid);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(themeGroupBox);
    mainLayout->addWidget(cardsGroupBox);
    mainLayout->addWidget(handGroupBox);
    mainLayout->addWidget(tableGroupBox);
    
    setLayout(mainLayout);
}

void AppearanceSettingsPage::themeBoxChanged(int index)
{
    QStringList themeDirs = themeManager->getAvailableThemes().keys();
    if(index >= 0 && index < themeDirs.count())
        settingsCache->setThemeName(themeDirs.at(index));
}

void AppearanceSettingsPage::retranslateUi()
{
    themeGroupBox->setTitle(tr("Theme settings"));
    themeLabel.setText(tr("Current theme:"));
    
    cardsGroupBox->setTitle(tr("Card rendering"));
    displayCardNamesCheckBox.setText(tr("Display card names on cards having a picture"));
    cardScalingCheckBox.setText(tr("Scale cards on mouse over"));
    
    handGroupBox->setTitle(tr("Hand layout"));
    horizontalHandCheckBox.setText(tr("Display hand horizontally (wastes space)"));
    leftJustifiedHandCheckBox.setText(tr("Enable left justification"));
    
    tableGroupBox->setTitle(tr("Table grid layout"));
    invertVerticalCoordinateCheckBox.setText(tr("Invert vertical coordinate"));
    minPlayersForMultiColumnLayoutLabel.setText(tr("Minimum player count for multi-column layout:"));
}

UserInterfaceSettingsPage::UserInterfaceSettingsPage()
{
    notificationsEnabledCheckBox.setChecked(settingsCache->getNotificationsEnabled());
    connect(&notificationsEnabledCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setNotificationsEnabled(int)));
    connect(&notificationsEnabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setSpecNotificationEnabled(int)));

    specNotificationsEnabledCheckBox.setChecked(settingsCache->getSpectatorNotificationsEnabled());
    specNotificationsEnabledCheckBox.setEnabled(settingsCache->getNotificationsEnabled());
    connect(&specNotificationsEnabledCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setSpectatorNotificationsEnabled(int)));

    doubleClickToPlayCheckBox.setChecked(settingsCache->getDoubleClickToPlay());
    connect(&doubleClickToPlayCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setDoubleClickToPlay(int)));
    
    playToStackCheckBox.setChecked(settingsCache->getPlayToStack());
    connect(&playToStackCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPlayToStack(int)));

    annotateTokensCheckBox.setChecked(settingsCache->getAnnotateTokens());
    connect(&annotateTokensCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setAnnotateTokens(int)));

    QGridLayout *generalGrid = new QGridLayout;
    generalGrid->addWidget(&notificationsEnabledCheckBox, 0, 0);
    generalGrid->addWidget(&specNotificationsEnabledCheckBox, 1, 0);
    generalGrid->addWidget(&doubleClickToPlayCheckBox, 2, 0);
    generalGrid->addWidget(&playToStackCheckBox, 3, 0);
    generalGrid->addWidget(&annotateTokensCheckBox, 4, 0);
    
    generalGroupBox = new QGroupBox;
    generalGroupBox->setLayout(generalGrid);
    
    tapAnimationCheckBox.setChecked(settingsCache->getTapAnimation());
    connect(&tapAnimationCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setTapAnimation(int)));
    
    QGridLayout *animationGrid = new QGridLayout;
    animationGrid->addWidget(&tapAnimationCheckBox, 0, 0);
    
    animationGroupBox = new QGroupBox;
    animationGroupBox->setLayout(animationGrid);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(animationGroupBox);
    
    setLayout(mainLayout);
}

void UserInterfaceSettingsPage::setSpecNotificationEnabled(int i) {
    specNotificationsEnabledCheckBox.setEnabled(i != 0);
}

void UserInterfaceSettingsPage::retranslateUi()
{
    generalGroupBox->setTitle(tr("General interface settings"));
    notificationsEnabledCheckBox.setText(tr("Enable notifications in taskbar"));
    specNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar for game events while you are spectating"));
    doubleClickToPlayCheckBox.setText(tr("&Double-click cards to play them (instead of single-click)"));
    playToStackCheckBox.setText(tr("&Play all nonlands onto the stack (not the battlefield) by default"));
    annotateTokensCheckBox.setText(tr("Annotate card text on tokens"));
    animationGroupBox->setTitle(tr("Animation settings"));
    tapAnimationCheckBox.setText(tr("&Tap/untap animation"));
}


DeckEditorSettingsPage::DeckEditorSettingsPage()
{
    //priceTagsCheckBox.setChecked(settingsCache->getPriceTagFeature());
    //connect(&priceTagsCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPriceTagFeature(int)));

    //connect(this, SIGNAL(priceTagSourceChanged(int)), settingsCache, SLOT(setPriceTagSource(int)));

    QGridLayout *generalGrid = new QGridLayout;
    //generalGrid->addWidget(&priceTagsCheckBox, 0, 0);
    
    generalGrid->addWidget(new QLabel(tr("Nothing is here... yet")), 0, 0);
    
    generalGroupBox = new QGroupBox;
    generalGroupBox->setLayout(generalGrid);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    
    setLayout(mainLayout);
}

void DeckEditorSettingsPage::retranslateUi()
{
    //priceTagsCheckBox.setText(tr("Enable &price tag feature from deckbrew.com"));
    generalGroupBox->setTitle(tr("General"));
}

/*
void DeckEditorSettingsPage::radioPriceTagSourceClicked(bool checked)
{
    if(!checked)
        return;

    int source=AbstractPriceUpdater::DBPriceSource;
    emit priceTagSourceChanged(source);
}
*/

MessagesSettingsPage::MessagesSettingsPage()
{
    chatMentionCheckBox.setChecked(settingsCache->getChatMention());
    connect(&chatMentionCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setChatMention(int)));

    chatMentionCompleterCheckbox.setChecked(settingsCache->getChatMentionCompleter());
    connect(&chatMentionCompleterCheckbox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setChatMentionCompleter(int)));
    
    ignoreUnregUsersMainChat.setChecked(settingsCache->getIgnoreUnregisteredUsers());
    ignoreUnregUserMessages.setChecked(settingsCache->getIgnoreUnregisteredUserMessages());
    connect(&ignoreUnregUsersMainChat, SIGNAL(stateChanged(int)), settingsCache, SLOT(setIgnoreUnregisteredUsers(int)));
    connect(&ignoreUnregUserMessages, SIGNAL(stateChanged(int)), settingsCache, SLOT(setIgnoreUnregisteredUserMessages(int)));
    
    invertMentionForeground.setChecked(settingsCache->getChatMentionForeground());
    connect(&invertMentionForeground, SIGNAL(stateChanged(int)), this, SLOT(updateTextColor(int)));

    invertHighlightForeground.setChecked(settingsCache->getChatHighlightForeground());
    connect(&invertHighlightForeground, SIGNAL(stateChanged(int)), this, SLOT(updateTextHighlightColor(int)));

    mentionColor = new QLineEdit();
    mentionColor->setText(settingsCache->getChatMentionColor());
    updateMentionPreview();
    connect(mentionColor, SIGNAL(textChanged(QString)), this, SLOT(updateColor(QString)));

    messagePopups.setChecked(settingsCache->getShowMessagePopup());
    connect(&messagePopups, SIGNAL(stateChanged(int)), settingsCache, SLOT(setShowMessagePopups(int)));

    mentionPopups.setChecked(settingsCache->getShowMentionPopup());
    connect(&mentionPopups, SIGNAL(stateChanged(int)), settingsCache, SLOT(setShowMentionPopups(int)));

    roomHistory.setChecked(settingsCache->getRoomHistory());
    connect(&roomHistory, SIGNAL(stateChanged(int)), settingsCache, SLOT(setRoomHistory(int)));

    customAlertString = new QLineEdit();
    customAlertString->setPlaceholderText("Word1 Word2 Word3");
    customAlertString->setText(settingsCache->getHighlightWords());
    connect(customAlertString, SIGNAL(textChanged(QString)), settingsCache, SLOT(setHighlightWords(QString)));

    QGridLayout *chatGrid = new QGridLayout;
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
    highlightColor->setText(settingsCache->getChatHighlightColor());
    updateHighlightPreview();
    connect(highlightColor, SIGNAL(textChanged(QString)), this, SLOT(updateHighlightColor(QString)));

    QGridLayout *highlightNotice = new QGridLayout;
    highlightNotice->addWidget(highlightColor, 0, 2);
    highlightNotice->addWidget(&invertHighlightForeground, 0, 1);
    highlightNotice->addWidget(&hexHighlightLabel, 1, 2);
    highlightNotice->addWidget(customAlertString, 0, 0);
    highlightNotice->addWidget(&customAlertStringLabel, 1, 0);
    highlightGroupBox = new QGroupBox;
    highlightGroupBox->setLayout(highlightNotice);

    messageList = new QListWidget;

    int count = settingsCache->messages().getCount();
    for (int i = 0; i < count; i++)
        messageList->addItem(settingsCache->messages().getMessageAt(i));
    
    aAdd = new QAction(this);
    aAdd->setIcon(QPixmap("theme:icons/increment"));
    connect(aAdd, SIGNAL(triggered()), this, SLOT(actAdd()));
    aRemove = new QAction(this);
    aRemove->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemove, SIGNAL(triggered()), this, SLOT(actRemove()));

    QToolBar *messageToolBar = new QToolBar;
    messageToolBar->setOrientation(Qt::Vertical);
    messageToolBar->addAction(aAdd);
    messageToolBar->addAction(aRemove);

    QHBoxLayout *messageListLayout = new QHBoxLayout;
    messageListLayout->addWidget(messageToolBar);
    messageListLayout->addWidget(messageList);

    messageShortcuts = new QGroupBox;
    messageShortcuts->setLayout(messageListLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(messageShortcuts);
    mainLayout->addWidget(chatGroupBox);
    mainLayout->addWidget(highlightGroupBox);

    setLayout(mainLayout);
    
    retranslateUi();
}

void MessagesSettingsPage::updateColor(const QString &value) {
    QColor colorToSet;
    colorToSet.setNamedColor("#" + value);
    if (colorToSet.isValid()) {
        settingsCache->setChatMentionColor(value);
        updateMentionPreview();
    }
}

void MessagesSettingsPage::updateHighlightColor(const QString &value) {
    QColor colorToSet;
    colorToSet.setNamedColor("#" + value);
    if (colorToSet.isValid()) {
        settingsCache->setChatHighlightColor(value);
        updateHighlightPreview();
    }
}

void MessagesSettingsPage::updateTextColor(int value) {
    settingsCache->setChatMentionForeground(value);
    updateMentionPreview();
}

void MessagesSettingsPage::updateTextHighlightColor(int value) {
    settingsCache->setChatHighlightForeground(value);
    updateHighlightPreview();
}

void MessagesSettingsPage::updateMentionPreview() {
    mentionColor->setStyleSheet("QLineEdit{background:#" + settingsCache->getChatMentionColor() + 
        ";color: " + (settingsCache->getChatMentionForeground() ? "white" : "black") + ";}");
}

void MessagesSettingsPage::updateHighlightPreview() {
    highlightColor->setStyleSheet("QLineEdit{background:#" + settingsCache->getChatHighlightColor() +
        ";color: " + (settingsCache->getChatHighlightForeground() ? "white" : "black") + ";}");
}

void MessagesSettingsPage::storeSettings()
{
    settingsCache->messages().setCount(messageList->count());
    for (int i = 0; i < messageList->count(); i++)
        settingsCache->messages().setMessageAt(i, messageList->item(i)->text());
}

void MessagesSettingsPage::actAdd()
{
    bool ok;
    QString msg = QInputDialog::getText(this, tr("Add message"), tr("Message:"), QLineEdit::Normal, QString(), &ok);
    if (ok) {
        messageList->addItem(msg);
        storeSettings();
    }
}

void MessagesSettingsPage::actRemove()
{
    if (messageList->currentItem()) {
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
    messageShortcuts->setTitle(tr("In-game message macros"));
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
}


SoundSettingsPage::SoundSettingsPage()
{
    soundEnabledCheckBox.setChecked(settingsCache->getSoundEnabled());
    connect(&soundEnabledCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setSoundEnabled(int)));

    QString themeName = settingsCache->getSoundThemeName();

    QStringList themeDirs = soundEngine->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName)
            themeBox.setCurrentIndex(i);
    }

    connect(&themeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(themeBoxChanged(int)));
    connect(&soundTestButton, SIGNAL(clicked()), soundEngine, SLOT(testSound()));

    masterVolumeSlider = new QSlider(Qt::Horizontal);
    masterVolumeSlider->setMinimum(0);
    masterVolumeSlider->setMaximum(100);
    masterVolumeSlider->setValue(settingsCache->getMasterVolume());
    masterVolumeSlider->setToolTip(QString::number(settingsCache->getMasterVolume()));
    connect(settingsCache, SIGNAL(masterVolumeChanged(int)), this, SLOT(masterVolumeChanged(int)));
    connect(masterVolumeSlider, SIGNAL(sliderReleased()), soundEngine, SLOT(testSound()));
    connect(masterVolumeSlider, SIGNAL(valueChanged(int)), settingsCache, SLOT(setMasterVolume(int)));

    masterVolumeSpinBox = new QSpinBox();
    masterVolumeSpinBox->setMinimum(0);
    masterVolumeSpinBox->setMaximum(100);
    masterVolumeSpinBox->setValue(settingsCache->getMasterVolume());
    connect(masterVolumeSlider, SIGNAL(valueChanged(int)), masterVolumeSpinBox, SLOT(setValue(int)));
    connect(masterVolumeSpinBox, SIGNAL(valueChanged(int)), masterVolumeSlider, SLOT(setValue(int)));

    QGridLayout *soundGrid = new QGridLayout;
    soundGrid->addWidget(&soundEnabledCheckBox, 0, 0, 1, 3);
    soundGrid->addWidget(&masterVolumeLabel, 1, 0);
    soundGrid->addWidget(masterVolumeSlider, 1, 1);
    soundGrid->addWidget(masterVolumeSpinBox, 1, 2);
    soundGrid->addWidget(&themeLabel, 2, 0);
    soundGrid->addWidget(&themeBox, 2, 1);
    soundGrid->addWidget(&soundTestButton, 3, 1);

    soundGroupBox = new QGroupBox;
    soundGroupBox->setLayout(soundGrid);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(soundGroupBox);

    setLayout(mainLayout);
}

void SoundSettingsPage::themeBoxChanged(int index)
{
    QStringList themeDirs = soundEngine->getAvailableThemes().keys();
    if(index >= 0 && index < themeDirs.count())
        settingsCache->setSoundThemeName(themeDirs.at(index));
}

void SoundSettingsPage::masterVolumeChanged(int value) {
    masterVolumeSlider->setToolTip(QString::number(value));
}

void SoundSettingsPage::retranslateUi() {
    soundEnabledCheckBox.setText(tr("Enable &sounds"));
    themeLabel.setText(tr("Current sounds theme:"));
    soundTestButton.setText(tr("Test system sound engine"));
    soundGroupBox->setTitle(tr("Sound settings"));
    masterVolumeLabel.setText(tr("Master volume"));    
}

DlgSettings::DlgSettings(QWidget *parent)
    : QDialog(parent)
{
    connect(settingsCache, SIGNAL(langChanged()), this, SLOT(updateLanguage()));
    
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(58, 50));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMinimumHeight(85);
    contentsWidget->setMaximumHeight(85);
    contentsWidget->setSpacing(5);
    
    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(new GeneralSettingsPage);
    pagesWidget->addWidget(new AppearanceSettingsPage);
    pagesWidget->addWidget(new UserInterfaceSettingsPage);
    pagesWidget->addWidget(new DeckEditorSettingsPage);
    pagesWidget->addWidget(new MessagesSettingsPage);
    pagesWidget->addWidget(new SoundSettingsPage);
    pagesWidget->addWidget(new ShortcutsTab);
    
    createIcons();
    contentsWidget->setCurrentRow(0);
    
    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(contentsWidget);
    vboxLayout->addWidget(pagesWidget);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(vboxLayout);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    
    retranslateUi();
    
    adjustSize();
}

void DlgSettings::createIcons()
{
    generalButton = new QListWidgetItem(contentsWidget);
    generalButton->setTextAlignment(Qt::AlignHCenter);
    generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    generalButton->setIcon(QPixmap("theme:config/general"));
    
    appearanceButton = new QListWidgetItem(contentsWidget);
    appearanceButton->setTextAlignment(Qt::AlignHCenter);
    appearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    appearanceButton->setIcon(QPixmap("theme:config/appearance"));
    
    userInterfaceButton = new QListWidgetItem(contentsWidget);
    userInterfaceButton->setTextAlignment(Qt::AlignHCenter);
    userInterfaceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    userInterfaceButton->setIcon(QPixmap("theme:config/interface"));
    
    deckEditorButton = new QListWidgetItem(contentsWidget);
    deckEditorButton->setTextAlignment(Qt::AlignHCenter);
    deckEditorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    deckEditorButton->setIcon(QPixmap("theme:config/deckeditor"));
    
    messagesButton = new QListWidgetItem(contentsWidget);
    messagesButton->setTextAlignment(Qt::AlignHCenter);
    messagesButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    messagesButton->setIcon(QPixmap("theme:config/messages"));

    soundButton = new QListWidgetItem(contentsWidget);
    soundButton->setTextAlignment(Qt::AlignHCenter);
    soundButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    soundButton->setIcon(QPixmap("theme:config/sound"));
    
    shortcutsButton = new QListWidgetItem(contentsWidget);
    shortcutsButton->setTextAlignment(Qt::AlignHCenter);
    shortcutsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    shortcutsButton->setIcon(QPixmap("theme:config/shorcuts"));

    connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem *)));
}

void DlgSettings::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    
    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void DlgSettings::setTab(int index) {
    if (index <= contentsWidget->count()-1 && index >= 0) {
        changePage(contentsWidget->item(index), contentsWidget->currentItem());
        contentsWidget->setCurrentRow(index);
    }
}

void DlgSettings::updateLanguage()
{
    qApp->removeTranslator(translator);
    installNewTranslator();
}

void DlgSettings::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDialog::changeEvent(event);
}

void DlgSettings::closeEvent(QCloseEvent *event)
{
    bool showLoadError = true;
    QString loadErrorMessage = tr("Unknown Error loading card database");
    LoadStatus loadStatus = db->getLoadStatus();
    qDebug() << "Card Database load status: " << loadStatus;
    switch(loadStatus) {
    case Ok:
        showLoadError = false;
        break;
    case Invalid:
        loadErrorMessage =
            tr("Your card database is invalid.\n\n"
               "Cockatrice may not function correctly with an invalid database\n\n"
               "You may need to rerun oracle to update your card database.\n\n"
               "Would you like to change your database location setting?");
        break;
    case VersionTooOld:
        loadErrorMessage =
            tr("Your card database version is too old.\n\n"
               "This can cause problems loading card information or images\n\n"
               "Usually this can be fixed by rerunning oracle to to update your card database.\n\n"
               "Would you like to change your database location setting?");
        break;
    case NotLoaded:
        loadErrorMessage =
            tr("Your card database did not finish loading\n\n"
               "Please file a ticket at http://github.com/Cockatrice/Cockatrice/issues with your cards.xml attached\n\n"
               "Would you like to change your database location setting?");
        break;
    case FileError:
        loadErrorMessage =
            tr("File Error loading your card database.\n\n"
               "Would you like to change your database location setting?");
        break;
    case NoCards:
        loadErrorMessage =
            tr("Your card database was loaded but contains no cards.\n\n"
               "Would you like to change your database location setting?");
        break;
    default:
        loadErrorMessage =
            tr("Unknown card database load status\n\n"
               "Please file a ticket at http://github.com/Cockatrice/Cockatrice/issues\n\n"
               "Would you like to change your database location setting?");

        break;
    }
    if (showLoadError)
        if (QMessageBox::critical(this, tr("Error"), loadErrorMessage, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    if (!QDir(settingsCache->getDeckPath()).exists() || settingsCache->getDeckPath().isEmpty())
        // TODO: Prompt to create it
        if (QMessageBox::critical(this, tr("Error"), tr("The path to your deck directory is invalid. Would you like to go back and set the correct path?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    if (!QDir(settingsCache->getPicsPath()).exists() || settingsCache->getPicsPath().isEmpty())
        // TODO: Prompt to create it
        if (QMessageBox::critical(this, tr("Error"), tr("The path to your card pictures directory is invalid. Would you like to go back and set the correct path?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    event->accept();
}

void DlgSettings::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    
    generalButton->setText(tr("General"));
    appearanceButton->setText(tr("Appearance"));
    userInterfaceButton->setText(tr("User Interface"));
    deckEditorButton->setText(tr("Deck Editor"));
    messagesButton->setText(tr("Chat"));
    soundButton->setText(tr("Sound"));
    shortcutsButton->setText(tr("Shortcuts"));
    
    for (int i = 0; i < pagesWidget->count(); i++)
        dynamic_cast<AbstractSettingsPage *>(pagesWidget->widget(i))->retranslateUi();

    contentsWidget->reset();
}

