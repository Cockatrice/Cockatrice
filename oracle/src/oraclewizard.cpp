#include <QtGui>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QAbstractButton>
#include <QBuffer>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>

#include "oraclewizard.h"
#include "oracleimporter.h"
#include "main.h"
#include "settingscache.h"

#define ZIP_SIGNATURE "PK"
#define ALLSETS_URL_FALLBACK "https://mtgjson.com/json/AllSets.json"

#ifdef HAS_ZLIB
    #include "zip/unzip.h"
    #define ALLSETS_URL "https://mtgjson.com/json/AllSets.json.zip"
#else
    #define ALLSETS_URL "https://mtgjson.com/json/AllSets.json"
#endif

#define TOKENS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml"


OracleWizard::OracleWizard(QWidget *parent)
    : QWizard(parent)
{
    settings = new QSettings(settingsCache->getSettingsPath()+"global.ini",QSettings::IniFormat, this);
    connect(settingsCache, SIGNAL(langChanged()), this, SLOT(updateLanguage()));

    importer = new OracleImporter(settingsCache->getDataPath(), this);

    addPage(new IntroPage);
    addPage(new LoadSetsPage);
    addPage(new SaveSetsPage);
    addPage(new LoadTokensPage);
    addPage(new SaveTokensPage);

    retranslateUi();
}

void OracleWizard::updateLanguage()
{
    qApp->removeTranslator(translator);
    installNewTranslator();
}

void OracleWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDialog::changeEvent(event);
}

void OracleWizard::retranslateUi()
{
    setWindowTitle(tr("Oracle Importer"));
    QWizard::setButtonText(QWizard::FinishButton, tr("Save"));
    
    for (int i = 0; i < pageIds().count(); i++)
        dynamic_cast<OracleWizardPage *>(page(i))->retranslateUi();
}

void OracleWizard::accept()
{
    QDialog::accept();
}

void OracleWizard::enableButtons()
{
    button(QWizard::NextButton)->setDisabled(false);
    button(QWizard::BackButton)->setDisabled(false);
}

void OracleWizard::disableButtons()
{
    button(QWizard::NextButton)->setDisabled(true);
    button(QWizard::BackButton)->setDisabled(true);
}

bool OracleWizard::saveTokensToFile(const QString & fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "File open (w) failed for" << fileName;
        return false;
    }
    if(file.write(tokensData) == -1)
    {
        qDebug() << "File write (w) failed for" << fileName;
        return false;
    }
    file.close();
    return true;
}

IntroPage::IntroPage(QWidget *parent)
    : OracleWizardPage(parent)
{
    label = new QLabel(this);
    label->setWordWrap(true);

    languageLabel = new QLabel(this);
    languageBox = new QComboBox(this);
    QString setLanguage = settingsCache->getLang();
    QStringList qmFiles = findQmFiles();
    for (int i = 0; i < qmFiles.size(); i++) {
        QString langName = languageName(qmFiles[i]);
        languageBox->addItem(langName, qmFiles[i]);
        if ((qmFiles[i] == setLanguage) || (setLanguage.isEmpty() && langName == QCoreApplication::translate("i18n", DEFAULT_LANG_NAME)))
            languageBox->setCurrentIndex(i);
    }
    connect(languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(label, 0, 0, 1, 2);
    layout->addWidget(languageLabel, 1, 0);
    layout->addWidget(languageBox, 1, 1);

    setLayout(layout);
}

QStringList IntroPage::findQmFiles()
{
    QDir dir(translationPath);
    QStringList fileNames = dir.entryList(QStringList(translationPrefix + "_*.qm"), QDir::Files, QDir::Name);
    fileNames.replaceInStrings(QRegExp(translationPrefix + "_(.*)\\.qm"), "\\1");
    return fileNames;
}

QString IntroPage::languageName(const QString &qmFile)
{
    if(qmFile == DEFAULT_LANG_CODE)
        return DEFAULT_LANG_NAME;

    QTranslator translator;
    translator.load(translationPrefix + "_" + qmFile + ".qm", translationPath);
    
    return translator.translate("i18n", DEFAULT_LANG_NAME);
}

void IntroPage::languageBoxChanged(int index)
{
    settingsCache->setLang(languageBox->itemData(index).toString());
}

void IntroPage::retranslateUi()
{
    setTitle(tr("Introduction"));
    label->setText(tr("This wizard will import the list of sets, cards, and tokens "
                      "that will be used by Cockatrice."
                      "\nYou will need to specify a URL or a filename that "
                      "will be used as a source."));
    languageLabel->setText(tr("Language:"));
}

LoadSetsPage::LoadSetsPage(QWidget *parent)
    : OracleWizardPage(parent), nam(0)
{
    urlRadioButton = new QRadioButton(this);
    fileRadioButton = new QRadioButton(this);

    urlLineEdit = new QLineEdit(this);
    fileLineEdit = new QLineEdit(this);

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlRadioButton->setChecked(true);

    urlButton = new QPushButton(this);
    connect(urlButton, SIGNAL(clicked()), this, SLOT(actRestoreDefaultUrl()));

    fileButton = new QPushButton(this);
    connect(fileButton, SIGNAL(clicked()), this, SLOT(actLoadSetsFile()));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(urlRadioButton, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(urlButton, 1, 1, Qt::AlignRight);
    layout->addWidget(fileRadioButton, 2, 0);
    layout->addWidget(fileLineEdit, 2, 1);
    layout->addWidget(fileButton, 3, 1, Qt::AlignRight);
    layout->addWidget(progressLabel, 4, 0);
    layout->addWidget(progressBar, 4, 1);

    connect(&watcher, SIGNAL(finished()), this, SLOT(importFinished()));

    setLayout(layout);
}

void LoadSetsPage::initializePage()
{
    urlLineEdit->setText(wizard()->settings->value("allsetsurl", ALLSETS_URL).toString());

    progressLabel->hide();
    progressBar->hide();
}

void LoadSetsPage::retranslateUi()
{
    setTitle(tr("Source selection"));
    setSubTitle(tr("Please specify a source for the list of sets and cards. "
                   "You can specify a URL address that will be downloaded or "
                   "use an existing file from your computer."));

    urlRadioButton->setText(tr("Download URL:"));
    fileRadioButton->setText(tr("Local file:"));
    urlButton->setText(tr("Restore default URL"));
    fileButton->setText(tr("Choose file..."));
}

void LoadSetsPage::actRestoreDefaultUrl()
{
    urlLineEdit->setText(ALLSETS_URL);
}

void LoadSetsPage::actLoadSetsFile()
{
    QFileDialog dialog(this, tr("Load sets file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    
#ifdef HAS_ZLIB
    dialog.setNameFilter(tr("Sets JSON file (*.json *.zip)"));
#else
    dialog.setNameFilter(tr("Sets JSON file (*.json)"));
#endif

    if(!fileLineEdit->text().isEmpty() && QFile::exists(fileLineEdit->text()))
        dialog.selectFile(fileLineEdit->text());

    if (!dialog.exec())
        return;

    fileLineEdit->setText(dialog.selectedFiles().at(0));
}

bool LoadSetsPage::validatePage()
{
    // once the import is finished, we call next(); skip validation
    if(wizard()->importer->getSets().count() > 0)
        return true;

    // else, try to import sets
    if(urlRadioButton->isChecked())
    {
        QUrl url = QUrl::fromUserInput(urlLineEdit->text());
        if(!url.isValid())
        {
            QMessageBox::critical(this, tr("Error"), tr("The provided URL is not valid."));
            return false;
        }

        progressLabel->setText(tr("Downloading (0MB)"));
        // show an infinite progressbar
        progressBar->setMaximum(0);
        progressBar->setMinimum(0);
        progressBar->setValue(0);
        progressLabel->show();
        progressBar->show();

        wizard()->disableButtons();
        setEnabled(false);

        downloadSetsFile(url);
    } else if(fileRadioButton->isChecked()) {
        QFile setsFile(fileLineEdit->text());
        if(!setsFile.exists())
        {
            QMessageBox::critical(this, tr("Error"), tr("Please choose a file."));
            return false;
        }

        if (!setsFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(0, tr("Error"), tr("Cannot open file '%1'.").arg(fileLineEdit->text()));
            return false;
        }

        wizard()->disableButtons();
        setEnabled(false);

        readSetsFromByteArray(setsFile.readAll());

    }
    return false;
}

void LoadSetsPage::downloadSetsFile(QUrl url)
{
    if(!nam)
        nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(url));

    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinishedSetsFile()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(actDownloadProgressSetsFile(qint64, qint64)));
}

void LoadSetsPage::actDownloadProgressSetsFile(qint64 received, qint64 total)
{
    if(total > 0)
    {
        progressBar->setMaximum(total);
        progressBar->setValue(received);
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int) received / (1024 * 1024)));
}

void LoadSetsPage::actDownloadFinishedSetsFile()
{
    // check for a reply
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));

        wizard()->enableButtons();
        setEnabled(true);

        reply->deleteLater();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "following redirect url:" << redirectUrl.toString();
        downloadSetsFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    progressLabel->hide();
    progressBar->hide();

    // save allsets.json url, but only if the user customized it and download was successfull
    if(urlLineEdit->text() != QString(ALLSETS_URL))
        wizard()->settings->setValue("allsetsurl", urlLineEdit->text());
    else
        wizard()->settings->remove("allsetsurl");

    readSetsFromByteArray(reply->readAll());
    reply->deleteLater();
}

void LoadSetsPage::readSetsFromByteArray(QByteArray data)
{
    // show an infinite progressbar
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    progressBar->setValue(0);
    progressLabel->setText(tr("Parsing file"));
    progressLabel->show();
    progressBar->show();

    // unzip the file if needed
    if(data.startsWith(ZIP_SIGNATURE))
    {
#ifdef HAS_ZLIB
        // zipped file
        QBuffer *inBuffer = new QBuffer(&data);
        QBuffer *outBuffer = new QBuffer(this);
        QString fileName;
        UnZip::ErrorCode ec;
        UnZip uz;

        ec = uz.openArchive(inBuffer);
        if (ec != UnZip::Ok) {
            zipDownloadFailed(tr("Failed to open Zip archive: %1.").arg(uz.formatError(ec)));
            return;
        }

        if(uz.fileList().size() != 1)
        {
            zipDownloadFailed(tr("Zip extraction failed: the Zip archive doesn't contain exactly one file."));
            return;            
        }
        fileName = uz.fileList().at(0);

        outBuffer->open(QBuffer::ReadWrite);
        ec = uz.extractFile(fileName, outBuffer);
        if (ec != UnZip::Ok) {
            zipDownloadFailed(tr("Zip extraction failed: %1.").arg(uz.formatError(ec)));
            uz.closeArchive();
            return;
        }

        future = QtConcurrent::run(wizard()->importer, &OracleImporter::readSetsFromByteArray, outBuffer->data());
        watcher.setFuture(future);
        return;
#else
        zipDownloadFailed(tr("Sorry, this version of Oracle does not support zipped files."));

        wizard()->enableButtons();
        setEnabled(true);
        progressLabel->hide();
        progressBar->hide();
        return;
#endif
    } 
    // Start the computation.
    future = QtConcurrent::run(wizard()->importer, &OracleImporter::readSetsFromByteArray, data);
    watcher.setFuture(future);
}

void LoadSetsPage::zipDownloadFailed(const QString &message)
{
    wizard()->enableButtons();
    setEnabled(true);
    progressLabel->hide();
    progressBar->hide();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Error"), message + "<br/>" + tr("Do you want to try to download a fresh copy of the uncompressed file instead?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    if (reply == QMessageBox::Yes)
    {
        urlRadioButton->setChecked(true);
        urlLineEdit->setText(ALLSETS_URL_FALLBACK);

        wizard()->next();
    }
}

void LoadSetsPage::importFinished()
{
    wizard()->enableButtons();
    setEnabled(true);
    progressLabel->hide();
    progressBar->hide();

    if(watcher.future().result())
    {
        wizard()->next();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("The file was retrieved successfully, but it does not contain any sets data."));
    }
}

SaveSetsPage::SaveSetsPage(QWidget *parent)
    : OracleWizardPage(parent)
{
    defaultPathCheckBox = new QCheckBox(this);
    defaultPathCheckBox->setChecked(true);

    messageLog = new QTextEdit(this);
    messageLog->setReadOnly(true);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(defaultPathCheckBox, 0, 0);
    layout->addWidget(messageLog, 1, 0);

    setLayout(layout);
}

void SaveSetsPage::cleanupPage()
{
    disconnect(wizard()->importer, SIGNAL(setIndexChanged(int, int, const QString &)), 0, 0);
}

void SaveSetsPage::initializePage()
{
    messageLog->clear();

    connect(wizard()->importer, SIGNAL(setIndexChanged(int, int, const QString &)), this, SLOT(updateTotalProgress(int, int, const QString &)));

    if (!wizard()->importer->startImport())
        QMessageBox::critical(this, tr("Error"), tr("No set has been imported."));
}

void SaveSetsPage::retranslateUi()
{
    setTitle(tr("Sets imported"));
    setSubTitle(tr("The following sets has been imported. "
                   "Press \"Save\" to save the imported cards to the Cockatrice database."));

    defaultPathCheckBox->setText(tr("Save to the default path (recommended)"));
    #ifdef PORTABLE_BUILD
    defaultPathCheckBox->setEnabled(false);
    #endif
}

void SaveSetsPage::updateTotalProgress(int cardsImported, int /* setIndex */, const QString &setName)
{
    if (setName.isEmpty()) {
        messageLog->append("<b>" + tr("Import finished: %1 cards.").arg(wizard()->importer->getCardList().size()) + "</b>");
    } else {
        messageLog->append(tr("%1: %2 cards imported").arg(setName).arg(cardsImported));        
    }
    messageLog->verticalScrollBar()->setValue(messageLog->verticalScrollBar()->maximum());
}

bool SaveSetsPage::validatePage()
{
    bool ok = false;
    QString defaultPath = settingsCache->getCardDatabasePath();
    QString windowName = tr("Save card database");
    QString fileType = tr("XML; card database (*.xml)");

    do {
        QString fileName;
        if (defaultPathCheckBox->isChecked())
            fileName = defaultPath;
        else
            fileName = QFileDialog::getSaveFileName(this, windowName, defaultPath, fileType);

        if (fileName.isEmpty())
            return false;

        QFileInfo fi(fileName);
        QDir fileDir(fi.path());
        if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
            return false;
        }
        if (wizard()->importer->saveToFile(fileName))
        {
            ok = true;
            QMessageBox::information(this,
              tr("Success"),
              tr("The card database has been saved successfully to\n%1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("The file could not be saved to %1").arg(fileName));;
            if (defaultPathCheckBox->isChecked())
                defaultPathCheckBox->setChecked(false);
        }
    } while (!ok);

    return true;
}

LoadTokensPage::LoadTokensPage(QWidget *parent)
    : OracleWizardPage(parent), nam(0)
{
    urlLabel = new QLabel(this);
    urlLineEdit = new QLineEdit(this);

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlButton = new QPushButton(this);
    connect(urlButton, SIGNAL(clicked()), this, SLOT(actRestoreDefaultUrl()));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(urlLabel, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(urlButton, 1, 1, Qt::AlignRight);
    layout->addWidget(progressLabel, 2, 0);
    layout->addWidget(progressBar, 2, 1);

    setLayout(layout);
}

void LoadTokensPage::initializePage()
{
    urlLineEdit->setText(wizard()->settings->value("tokensurl", TOKENS_URL).toString());

    progressLabel->hide();
    progressBar->hide();
}

void LoadTokensPage::retranslateUi()
{
    setTitle(tr("Tokens source selection"));
    setSubTitle(tr("Please specify a source for the list of tokens. "
                   "You can specify a URL address that will be downloaded or "
                   "use an existing file from your computer."));

    urlLabel->setText(tr("Download URL:"));
    urlButton->setText(tr("Restore default URL"));
}

void LoadTokensPage::actRestoreDefaultUrl()
{
    urlLineEdit->setText(TOKENS_URL);
}

bool LoadTokensPage::validatePage()
{
    // once the import is finished, we call next(); skip validation
    if(wizard()->hasTokensData())
        return true;

    QUrl url = QUrl::fromUserInput(urlLineEdit->text());
    if(!url.isValid())
    {
        QMessageBox::critical(this, tr("Error"), tr("The provided URL is not valid."));
        return false;
    }

    progressLabel->setText(tr("Downloading (0MB)"));
    // show an infinite progressbar
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    progressBar->setValue(0);
    progressLabel->show();
    progressBar->show();

    wizard()->disableButtons();
    setEnabled(false);

    downloadTokensFile(url);
    return false;
}

void LoadTokensPage::downloadTokensFile(QUrl url)
{
    if(!nam)
        nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(url));

    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinishedTokensFile()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(actDownloadProgressTokensFile(qint64, qint64)));
}

void LoadTokensPage::actDownloadProgressTokensFile(qint64 received, qint64 total)
{
    if(total > 0)
    {
        progressBar->setMaximum(total);
        progressBar->setValue(received);
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int) received / (1024 * 1024)));
}

void LoadTokensPage::actDownloadFinishedTokensFile()
{
    // check for a reply
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));

        wizard()->enableButtons();
        setEnabled(true);

        reply->deleteLater();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "following redirect url:" << redirectUrl.toString();
        downloadTokensFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    progressLabel->hide();
    progressBar->hide();

    // save tokens.xml url, but only if the user customized it and download was successfull
    if(urlLineEdit->text() != QString(TOKENS_URL))
        wizard()->settings->setValue("tokensurl", urlLineEdit->text());
    else
        wizard()->settings->remove("tokensurl");

    wizard()->setTokensData(reply->readAll());
    reply->deleteLater();

    wizard()->enableButtons();
    setEnabled(true);
    progressLabel->hide();
    progressBar->hide();

    wizard()->next();
}

SaveTokensPage::SaveTokensPage(QWidget *parent)
    : OracleWizardPage(parent)
{
    defaultPathCheckBox = new QCheckBox(this);
    defaultPathCheckBox->setChecked(true);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(defaultPathCheckBox, 0, 0);

    setLayout(layout);
}

void SaveTokensPage::retranslateUi()
{
    setTitle(tr("Tokens imported"));
    setSubTitle(tr("The tokens has been imported. "
                   "Press \"Save\" to save the imported tokens to the Cockatrice tokens database."));

    defaultPathCheckBox->setText(tr("Save to the default path (recommended)"));
}

bool SaveTokensPage::validatePage()
{
    bool ok = false;
    QString defaultPath = settingsCache->getTokenDatabasePath();
    QString windowName = tr("Save token database");
    QString fileType = tr("XML; token database (*.xml)");

    do {
        QString fileName;
        if (defaultPathCheckBox->isChecked())
            fileName = defaultPath;
        else
            fileName = QFileDialog::getSaveFileName(this, windowName, defaultPath, fileType);

        if (fileName.isEmpty())
            return false;

        QFileInfo fi(fileName);
        QDir fileDir(fi.path());
        if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
            return false;
        }
        if (wizard()->saveTokensToFile(fileName))
        {
            ok = true;
            QMessageBox::information(this,
              tr("Success"),
              tr("The token database has been saved successfully to\n%1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("The file could not be saved to %1").arg(fileName));;
            if (defaultPathCheckBox->isChecked())
                defaultPathCheckBox->setChecked(false);
        }
    } while (!ok);

    return true;
}
