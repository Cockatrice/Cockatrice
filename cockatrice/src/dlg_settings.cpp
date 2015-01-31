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
#include <QSettings>
#include <QApplication>
#include <QInputDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QDebug>
#include "carddatabase.h"
#include "dlg_settings.h"
#include "main.h"
#include "settingscache.h"
#include "priceupdater.h"
#include "soundengine.h"

GeneralSettingsPage::GeneralSettingsPage()
{
    QString setLanguage = settingsCache->getLang();
    QStringList qmFiles = findQmFiles();
    for (int i = 0; i < qmFiles.size(); i++) {
        QString langName = languageName(qmFiles[i]);
        languageBox.addItem(langName, qmFiles[i]);
        if ((qmFiles[i] == setLanguage) || (setLanguage.isEmpty() && langName == tr("English")))
            languageBox.setCurrentIndex(i);
    }

    picDownloadCheckBox.setChecked(settingsCache->getPicDownload());
    
    connect(&clearDownloadedPicsButton, SIGNAL(clicked()), this, SLOT(clearDownloadedPicsButtonClicked()));

    picDownloadHqCheckBox.setChecked(settingsCache->getPicDownloadHq());

    pixmapCacheEdit.setMinimum(PIXMAPCACHE_SIZE_MIN);
    // 2047 is the max value to avoid overflowing of QPixmapCache::setCacheLimit(int size)
    pixmapCacheEdit.setMaximum(PIXMAPCACHE_SIZE_MAX);
    pixmapCacheEdit.setSingleStep(64);
    pixmapCacheEdit.setValue(settingsCache->getPixmapCacheSize());
    pixmapCacheEdit.setSuffix(" MB");
    picDownloadHqCheckBox.setChecked(settingsCache->getPicDownloadHq());
    picDownloadCheckBox.setChecked(settingsCache->getPicDownload());

    connect(&clearDownloadedPicsButton, SIGNAL(clicked()), this, SLOT(clearDownloadedPicsButtonClicked()));
    connect(&languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));
    connect(&picDownloadCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPicDownload(int)));
    connect(&picDownloadHqCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPicDownloadHq(int)));
    connect(&pixmapCacheEdit, SIGNAL(valueChanged(int)), settingsCache, SLOT(setPixmapCacheSize(int)));

    QGridLayout *personalGrid = new QGridLayout;
    personalGrid->addWidget(&languageLabel, 0, 0);
    personalGrid->addWidget(&languageBox, 0, 1);
    personalGrid->addWidget(&pixmapCacheLabel, 1, 0, 1, 1);
    personalGrid->addWidget(&pixmapCacheEdit, 1, 1, 1, 1);
    personalGrid->addWidget(&picDownloadCheckBox, 2, 0, 1, 2);
    personalGrid->addWidget(&picDownloadHqCheckBox, 3, 0, 1, 2);
    personalGrid->addWidget(&clearDownloadedPicsButton, 4, 0, 1, 1);

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
    QTranslator translator;
    translator.load(translationPrefix + "_" + qmFile + ".qm", translationPath);
    
    return translator.translate("GeneralSettingsPage", "English");
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
    picDownloadHqCheckBox.setText(tr("Download high-quality card pictures"));
    pathsGroupBox->setTitle(tr("Paths"));
    deckPathLabel.setText(tr("Decks directory:"));
    replaysPathLabel.setText(tr("Replays directory:"));
    picsPathLabel.setText(tr("Pictures directory:"));
    cardDatabasePathLabel.setText(tr("Card database:"));
    tokenDatabasePathLabel.setText(tr("Token database:"));
    pixmapCacheLabel.setText(tr("Picture cache size:"));
    clearDownloadedPicsButton.setText(tr("Reset/Clear Downloaded Pictures"));
}

AppearanceSettingsPage::AppearanceSettingsPage()
{
    QIcon deleteIcon(":/resources/icon_delete.svg");
    
    handBgEdit = new QLineEdit(settingsCache->getHandBgPath());
    handBgEdit->setReadOnly(true);
    QPushButton *handBgClearButton = new QPushButton(deleteIcon, QString());
    connect(handBgClearButton, SIGNAL(clicked()), this, SLOT(handBgClearButtonClicked()));
    QPushButton *handBgButton = new QPushButton("...");
    connect(handBgButton, SIGNAL(clicked()), this, SLOT(handBgButtonClicked()));
    
    stackBgEdit = new QLineEdit(settingsCache->getStackBgPath());
    stackBgEdit->setReadOnly(true);
    QPushButton *stackBgClearButton = new QPushButton(deleteIcon, QString());
    connect(stackBgClearButton, SIGNAL(clicked()), this, SLOT(stackBgClearButtonClicked()));
    QPushButton *stackBgButton = new QPushButton("...");
    connect(stackBgButton, SIGNAL(clicked()), this, SLOT(stackBgButtonClicked()));

    tableBgEdit = new QLineEdit(settingsCache->getTableBgPath());
    tableBgEdit->setReadOnly(true);
    QPushButton *tableBgClearButton = new QPushButton(deleteIcon, QString());
    connect(tableBgClearButton, SIGNAL(clicked()), this, SLOT(tableBgClearButtonClicked()));
    QPushButton *tableBgButton = new QPushButton("...");
    connect(tableBgButton, SIGNAL(clicked()), this, SLOT(tableBgButtonClicked()));
    
    playerAreaBgEdit = new QLineEdit(settingsCache->getPlayerBgPath());
    playerAreaBgEdit->setReadOnly(true);
    QPushButton *playerAreaBgClearButton = new QPushButton(deleteIcon, QString());
    connect(playerAreaBgClearButton, SIGNAL(clicked()), this, SLOT(playerAreaBgClearButtonClicked()));
    QPushButton *playerAreaBgButton = new QPushButton("...");
    connect(playerAreaBgButton, SIGNAL(clicked()), this, SLOT(playerAreaBgButtonClicked()));
    
    cardBackPicturePathEdit = new QLineEdit(settingsCache->getCardBackPicturePath());
    cardBackPicturePathEdit->setReadOnly(true);
    QPushButton *cardBackPicturePathClearButton = new QPushButton(deleteIcon, QString());
    connect(cardBackPicturePathClearButton, SIGNAL(clicked()), this, SLOT(cardBackPicturePathClearButtonClicked()));
    QPushButton *cardBackPicturePathButton = new QPushButton("...");
    connect(cardBackPicturePathButton, SIGNAL(clicked()), this, SLOT(cardBackPicturePathButtonClicked()));
    
    QGridLayout *zoneBgGrid = new QGridLayout;
    zoneBgGrid->addWidget(&handBgLabel, 0, 0);
    zoneBgGrid->addWidget(handBgEdit, 0, 1);
    zoneBgGrid->addWidget(handBgClearButton, 0, 2);
    zoneBgGrid->addWidget(handBgButton, 0, 3);
    zoneBgGrid->addWidget(&stackBgLabel, 1, 0);
    zoneBgGrid->addWidget(stackBgEdit, 1, 1);
    zoneBgGrid->addWidget(stackBgClearButton, 1, 2);
    zoneBgGrid->addWidget(stackBgButton, 1, 3);
    zoneBgGrid->addWidget(&tableBgLabel, 2, 0);
    zoneBgGrid->addWidget(tableBgEdit, 2, 1);
    zoneBgGrid->addWidget(tableBgClearButton, 2, 2);
    zoneBgGrid->addWidget(tableBgButton, 2, 3);
    zoneBgGrid->addWidget(&playerAreaBgLabel, 3, 0);
    zoneBgGrid->addWidget(playerAreaBgEdit, 3, 1);
    zoneBgGrid->addWidget(playerAreaBgClearButton, 3, 2);
    zoneBgGrid->addWidget(playerAreaBgButton, 3, 3);
    zoneBgGrid->addWidget(&cardBackPicturePathLabel, 4, 0);
    zoneBgGrid->addWidget(cardBackPicturePathEdit, 4, 1);
    zoneBgGrid->addWidget(cardBackPicturePathClearButton, 4, 2);
    zoneBgGrid->addWidget(cardBackPicturePathButton, 4, 3);

    zoneBgGroupBox = new QGroupBox;
    zoneBgGroupBox->setLayout(zoneBgGrid);

    displayCardNamesCheckBox.setChecked(settingsCache->getDisplayCardNames());
    connect(&displayCardNamesCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setDisplayCardNames(int)));
    
    QGridLayout *cardsGrid = new QGridLayout;
    cardsGrid->addWidget(&displayCardNamesCheckBox, 0, 0, 1, 2);
    
    cardsGroupBox = new QGroupBox;
    cardsGroupBox->setLayout(cardsGrid);
    
    horizontalHandCheckBox.setChecked(settingsCache->getHorizontalHand());
    connect(&horizontalHandCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setHorizontalHand(int)));
    
    QGridLayout *handGrid = new QGridLayout;
    handGrid->addWidget(&horizontalHandCheckBox, 0, 0, 1, 2);
    
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
    mainLayout->addWidget(zoneBgGroupBox);
    mainLayout->addWidget(cardsGroupBox);
    mainLayout->addWidget(handGroupBox);
    mainLayout->addWidget(tableGroupBox);
    
    setLayout(mainLayout);
}

void AppearanceSettingsPage::retranslateUi()
{
    zoneBgGroupBox->setTitle(tr("Zone background pictures"));
    handBgLabel.setText(tr("Hand background:"));
    stackBgLabel.setText(tr("Stack background:"));
    tableBgLabel.setText(tr("Table background:"));
    playerAreaBgLabel.setText(tr("Player info background:"));
    cardBackPicturePathLabel.setText(tr("Card back:"));
    
    cardsGroupBox->setTitle(tr("Card rendering"));
    displayCardNamesCheckBox.setText(tr("Display card names on cards having a picture"));
    
    handGroupBox->setTitle(tr("Hand layout"));
    horizontalHandCheckBox.setText(tr("Display hand horizontally (wastes space)"));
    
    tableGroupBox->setTitle(tr("Table grid layout"));
    invertVerticalCoordinateCheckBox.setText(tr("Invert vertical coordinate"));
    minPlayersForMultiColumnLayoutLabel.setText(tr("Minimum player count for multi-column layout:"));
}

void AppearanceSettingsPage::handBgClearButtonClicked()
{
    handBgEdit->setText(QString());
    settingsCache->setHandBgPath(QString());
}

void AppearanceSettingsPage::handBgButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    handBgEdit->setText(path);
    settingsCache->setHandBgPath(path);
}

void AppearanceSettingsPage::stackBgClearButtonClicked()
{
    stackBgEdit->setText(QString());
    settingsCache->setStackBgPath(QString());
}

void AppearanceSettingsPage::stackBgButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    stackBgEdit->setText(path);
    settingsCache->setStackBgPath(path);
}

void AppearanceSettingsPage::tableBgClearButtonClicked()
{
    tableBgEdit->setText(QString());
    settingsCache->setTableBgPath(QString());
}

void AppearanceSettingsPage::tableBgButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;

    tableBgEdit->setText(path);
    settingsCache->setTableBgPath(path);
}

void AppearanceSettingsPage::playerAreaBgClearButtonClicked()
{
    playerAreaBgEdit->setText(QString());
    settingsCache->setPlayerBgPath(QString());
}

void AppearanceSettingsPage::playerAreaBgButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    playerAreaBgEdit->setText(path);
    settingsCache->setPlayerBgPath(path);
}

void AppearanceSettingsPage::cardBackPicturePathClearButtonClicked()
{
    cardBackPicturePathEdit->setText(QString());
    settingsCache->setCardBackPicturePath(QString());
}

void AppearanceSettingsPage::cardBackPicturePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    cardBackPicturePathEdit->setText(path);
    settingsCache->setCardBackPicturePath(path);
}

UserInterfaceSettingsPage::UserInterfaceSettingsPage()
{
    QIcon deleteIcon(":/resources/icon_delete.svg");

    notificationsEnabledCheckBox.setChecked(settingsCache->getNotificationsEnabled());
    connect(&notificationsEnabledCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setNotificationsEnabled(int)));

    doubleClickToPlayCheckBox.setChecked(settingsCache->getDoubleClickToPlay());
    connect(&doubleClickToPlayCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setDoubleClickToPlay(int)));
    
    playToStackCheckBox.setChecked(settingsCache->getPlayToStack());
    connect(&playToStackCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPlayToStack(int)));
    
    QGridLayout *generalGrid = new QGridLayout;
    generalGrid->addWidget(&notificationsEnabledCheckBox, 0, 0);
    generalGrid->addWidget(&doubleClickToPlayCheckBox, 1, 0);
    generalGrid->addWidget(&playToStackCheckBox, 2, 0);
    
    generalGroupBox = new QGroupBox;
    generalGroupBox->setLayout(generalGrid);
    
    tapAnimationCheckBox.setChecked(settingsCache->getTapAnimation());
    connect(&tapAnimationCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setTapAnimation(int)));
    
    soundEnabledCheckBox.setChecked(settingsCache->getSoundEnabled());
    connect(&soundEnabledCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setSoundEnabled(int)));
    
    soundPathEdit = new QLineEdit(settingsCache->getSoundPath());
    soundPathEdit->setReadOnly(true);
    QPushButton *soundPathClearButton = new QPushButton(deleteIcon, QString());
    connect(soundPathClearButton, SIGNAL(clicked()), this, SLOT(soundPathClearButtonClicked()));
    QPushButton *soundPathButton = new QPushButton("...");
    connect(soundPathButton, SIGNAL(clicked()), this, SLOT(soundPathButtonClicked()));
    connect(&soundTestButton, SIGNAL(clicked()), soundEngine, SLOT(cuckoo()));
    
    QGridLayout *soundGrid = new QGridLayout;
    soundGrid->addWidget(&soundEnabledCheckBox, 0, 0, 1, 4);
    soundGrid->addWidget(&soundPathLabel, 1, 0);
    soundGrid->addWidget(soundPathEdit, 1, 1);
    soundGrid->addWidget(soundPathClearButton, 1, 2);
    soundGrid->addWidget(soundPathButton, 1, 3);
    soundGrid->addWidget(&soundTestButton, 2, 1);
    
    soundGroupBox = new QGroupBox;
    soundGroupBox->setLayout(soundGrid);
    
    QGridLayout *animationGrid = new QGridLayout;
    animationGrid->addWidget(&tapAnimationCheckBox, 0, 0);
    
    animationGroupBox = new QGroupBox;
    animationGroupBox->setLayout(animationGrid);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(animationGroupBox);
    mainLayout->addWidget(soundGroupBox);
    
    setLayout(mainLayout);
}

void UserInterfaceSettingsPage::retranslateUi()
{
    generalGroupBox->setTitle(tr("General interface settings"));
    notificationsEnabledCheckBox.setText(tr("Enable notifications in taskbar"));
    doubleClickToPlayCheckBox.setText(tr("&Double-click cards to play them (instead of single-click)"));
    playToStackCheckBox.setText(tr("&Play all nonlands onto the stack (not the battlefield) by default"));
    animationGroupBox->setTitle(tr("Animation settings"));
    tapAnimationCheckBox.setText(tr("&Tap/untap animation"));
    soundEnabledCheckBox.setText(tr("Enable &sounds"));
    soundPathLabel.setText(tr("Path to sounds directory:"));
    soundTestButton.setText(tr("Test system sound engine"));
    soundGroupBox->setTitle(tr("Sound settings"));
}

void UserInterfaceSettingsPage::soundPathClearButtonClicked()
{
    soundPathEdit->setText(QString());
    settingsCache->setSoundPath(QString());
}

void UserInterfaceSettingsPage::soundPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"));
    if (path.isEmpty())
        return;
    
    soundPathEdit->setText(path);
    settingsCache->setSoundPath(path);
}

DeckEditorSettingsPage::DeckEditorSettingsPage()
{
    priceTagsCheckBox.setChecked(settingsCache->getPriceTagFeature());
    connect(&priceTagsCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setPriceTagFeature(int)));

    connect(this, SIGNAL(priceTagSourceChanged(int)), settingsCache, SLOT(setPriceTagSource(int)));

    QGridLayout *generalGrid = new QGridLayout;
    generalGrid->addWidget(&priceTagsCheckBox, 0, 0);
    
    generalGroupBox = new QGroupBox;
    generalGroupBox->setLayout(generalGrid);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    
    setLayout(mainLayout);
}

void DeckEditorSettingsPage::retranslateUi()
{
    priceTagsCheckBox.setText(tr("Enable &price tag feature from deckbrew.com"));
    generalGroupBox->setTitle(tr("General"));
}

void DeckEditorSettingsPage::radioPriceTagSourceClicked(bool checked)
{
    if(!checked)
        return;

    int source=AbstractPriceUpdater::DBPriceSource;
    emit priceTagSourceChanged(source);
}

MessagesSettingsPage::MessagesSettingsPage()
{

    chatMentionCheckBox.setChecked(settingsCache->getChatMention());
    connect(&chatMentionCheckBox, SIGNAL(stateChanged(int)), settingsCache, SLOT(setChatMention(int)));
    
    ignoreUnregUsersMainChat.setChecked(settingsCache->getIgnoreUnregisteredUsers());
    connect(&ignoreUnregUsersMainChat, SIGNAL(stateChanged(int)), settingsCache, SLOT(setIgnoreUnregisteredUsers(int)));
    
    QGridLayout *chatGrid = new QGridLayout;
    chatGrid->addWidget(&chatMentionCheckBox, 0, 0);
    chatGrid->addWidget(&ignoreUnregUsersMainChat, 1, 0);
    chatGroupBox = new QGroupBox;
    chatGroupBox->setLayout(chatGrid);

    QSettings settings;
    messageList = new QListWidget;
    settings.beginGroup("messages");
    int count = settings.value("count", 0).toInt();
    for (int i = 0; i < count; i++)
        messageList->addItem(settings.value(QString("msg%1").arg(i)).toString());
    
    aAdd = new QAction(this);
    connect(aAdd, SIGNAL(triggered()), this, SLOT(actAdd()));
    aRemove = new QAction(this);
    connect(aRemove, SIGNAL(triggered()), this, SLOT(actRemove()));

    QToolBar *messageToolBar = new QToolBar;
    messageToolBar->setOrientation(Qt::Vertical);
    messageToolBar->addAction(aAdd);
    messageToolBar->addAction(aRemove);

    QHBoxLayout *messageListLayout = new QHBoxLayout;
    messageListLayout->addWidget(messageList);
    messageListLayout->addWidget(messageToolBar);

    messageShortcuts = new QGroupBox;
    messageShortcuts->setLayout(messageListLayout);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
     
    mainLayout->addWidget(messageShortcuts);
    mainLayout->addWidget(chatGroupBox);

    setLayout(mainLayout);
    
    retranslateUi();
}

void MessagesSettingsPage::storeSettings()
{
    QSettings settings;
    settings.beginGroup("messages");
    settings.setValue("count", messageList->count());
    for (int i = 0; i < messageList->count(); i++)
        settings.setValue(QString("msg%1").arg(i), messageList->item(i)->text());
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
    aAdd->setText(tr("&Add"));
    aRemove->setText(tr("&Remove"));
    chatGroupBox->setTitle(tr("Chat settings"));
    chatMentionCheckBox.setText(tr("Enable chat mentions ('@yourusername' in chat log will be highlighted)"));
    messageShortcuts->setTitle(tr("In-game message macros"));
    ignoreUnregUsersMainChat.setText(tr("Ignore unregistered users in main chat"));
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
    generalButton->setIcon(QIcon(":/resources/icon_config_general.svg"));
    
    appearanceButton = new QListWidgetItem(contentsWidget);
    appearanceButton->setTextAlignment(Qt::AlignHCenter);
    appearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    appearanceButton->setIcon(QIcon(":/resources/icon_config_appearance.svg"));
    
    userInterfaceButton = new QListWidgetItem(contentsWidget);
    userInterfaceButton->setTextAlignment(Qt::AlignHCenter);
    userInterfaceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    userInterfaceButton->setIcon(QIcon(":/resources/icon_config_interface.svg"));
    
    deckEditorButton = new QListWidgetItem(contentsWidget);
    deckEditorButton->setTextAlignment(Qt::AlignHCenter);
    deckEditorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    deckEditorButton->setIcon(QIcon(":/resources/icon_config_deckeditor.svg"));
    
    messagesButton = new QListWidgetItem(contentsWidget);
    messagesButton->setTextAlignment(Qt::AlignHCenter);
    messagesButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    messagesButton->setIcon(QIcon(":/resources/icon_config_messages.svg"));
    
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
               "Please file a ticket at http://github.com/Daenyth/Cockatrice/issues with your cards.xml attached\n\n"
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
               "Please file a ticket at http://github.com/Daenyth/Cockatrice/issues\n\n"
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
    userInterfaceButton->setText(tr("User interface"));
    deckEditorButton->setText(tr("Deck editor"));
    messagesButton->setText(tr("Chat Settings"));
    
    for (int i = 0; i < pagesWidget->count(); i++)
        dynamic_cast<AbstractSettingsPage *>(pagesWidget->widget(i))->retranslateUi();
}
