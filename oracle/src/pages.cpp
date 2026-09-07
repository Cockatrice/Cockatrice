#include "pages.h"

#include "client/settings/cache_settings.h"
#include "main.h"
#include "oracleimporter.h"
#include "oraclewizard.h"
#include "pages.h"
#include "pagetemplates.h"
#include "version_string.h"

#include <QAbstractButton>
#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QtConcurrent>
#include <QtGui>
#include <cstdio>
#include <libcockatrice/settings/personal_settings.h>

#ifdef HAS_LZMA
#include "lzma/decompress.h"
#endif

#ifdef HAS_ZLIB
#include "zip/unzip.h"
#endif

#define ZIP_SIGNATURE "PK"
// Xz stream header: 0xFD + "7zXZ"
#define XZ_SIGNATURE "\xFD\x37\x7A\x58\x5A"
#define MTGJSON_V4_URL_COMPONENT "mtgjson.com/files/"
#define ALLSETS_URL_FALLBACK "https://www.mtgjson.com/api/v5/AllPrintings.json"
#define MTGJSON_VERSION_URL "https://www.mtgjson.com/api/v5/Meta.json"

#ifdef HAS_LZMA
#define ALLSETS_URL "https://www.mtgjson.com/api/v5/AllPrintings.json.xz"
#elif defined(HAS_ZLIB)
#define ALLSETS_URL "https://www.mtgjson.com/api/v5/AllPrintings.json.zip"
#else
#define ALLSETS_URL "https://www.mtgjson.com/api/v5/AllPrintings.json"
#endif

/**
 * @brief Emits one machine-readable background-run progress line to stdout.
 *
 * Used only in background mode, so the hosting Cockatrice client can parse these
 * lines to drive a determinate progress bar. stderr stays reserved for
 * human-readable log output.
 */
static void emitBackgroundProgress(const char *stage, qint64 done, qint64 total)
{
    QTextStream out(stdout);
    out << "PROGRESS " << stage << ' ' << done << ' ' << total << '\n';
    out.flush();
}

namespace
{

/**
 * @brief Decompresses and dispatches a sets-file payload on a worker thread.
 *
 * Iteratively unwraps xz/zip compression, then either hands the JSON to the
 * importer (which reports scan progress via dataReadProgress) or returns the raw
 * XML for the plain-XML path. Must never touch the wizard or the page: the caller
 * consumes the returned LoadSetsResult on the UI thread in importFinished().
 */
LoadSetsResult loadSetsData(const QPointer<OracleImporter> &importer, QByteArray data)
{
    LoadSetsResult result;

    while (true) {
        if (data.startsWith(XZ_SIGNATURE)) {
#ifdef HAS_LZMA
            QBuffer inBuffer(&data);
            QByteArray decompressed;
            QBuffer outBuffer(&decompressed);
            inBuffer.open(QBuffer::ReadOnly);
            outBuffer.open(QBuffer::WriteOnly);
            XzDecompressor xz;
            if (!xz.decompress(&inBuffer, &outBuffer)) {
                result.errorMessage = LoadSetsPage::tr("Xz extraction failed.");
                result.offerUncompressedFallback = true;
                return result;
            }
            data = decompressed;
            continue;
#else
            result.errorMessage =
                LoadSetsPage::tr("Sorry, this version of Oracle does not support xz compressed files.");
            result.offerUncompressedFallback = true;
            return result;
#endif
        }

        if (data.startsWith(ZIP_SIGNATURE)) {
#ifdef HAS_ZLIB
            QBuffer inBuffer(&data);
            UnZip uz;
            const UnZip::ErrorCode openEc = uz.openArchive(&inBuffer);
            if (openEc != UnZip::Ok) {
                result.errorMessage = LoadSetsPage::tr("Failed to open Zip archive: %1.").arg(uz.formatError(openEc));
                result.offerUncompressedFallback = true;
                return result;
            }
            if (uz.fileList().size() != 1) {
                result.errorMessage =
                    LoadSetsPage::tr("Zip extraction failed: the Zip archive doesn't contain exactly one file.");
                result.offerUncompressedFallback = true;
                return result;
            }
            const QString fileName = uz.fileList().at(0);
            QByteArray decompressed;
            QBuffer outBuffer(&decompressed);
            outBuffer.open(QBuffer::ReadWrite);
            const UnZip::ErrorCode ec = uz.extractFile(fileName, &outBuffer);
            uz.closeArchive();
            if (ec != UnZip::Ok) {
                result.errorMessage = LoadSetsPage::tr("Zip extraction failed: %1.").arg(uz.formatError(ec));
                result.offerUncompressedFallback = true;
                return result;
            }
            data = decompressed;
            continue;
#else
            result.errorMessage = LoadSetsPage::tr("Sorry, this version of Oracle does not support zipped files.");
            result.offerUncompressedFallback = true;
            return result;
#endif
        }
        break;
    }

    if (data.startsWith("<")) {
        result.ok = true;
        result.plainXml = true;
        result.xmlData = std::move(data);
        return result;
    }

    if (data.startsWith("{")) {
        result.ok = importer && importer->readSetsFromByteArray(std::move(data));
        return result;
    }

    result.errorMessage = LoadSetsPage::tr("Failed to interpret downloaded data.");
    return result;
}

} // namespace

#define TOKENS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml"
#define SPOILERS_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

IntroPage::IntroPage(QWidget *parent) : OracleWizardPage(parent)
{
    label = new QLabel(this);
    label->setWordWrap(true);

    languageLabel = new QLabel(this);
    versionLabel = new QLabel(this);
    languageBox = new QComboBox(this);

    QStringList languageCodes = findQmFiles();
    for (const QString &code : languageCodes) {
        QString langName = languageName(code);
        languageBox->addItem(langName, code);
    }

    QString setLanguage = QCoreApplication::translate("i18n", DEFAULT_LANG_NAME);
    int index = languageBox->findText(setLanguage, Qt::MatchExactly);
    if (index == -1) {
        qWarning() << "could not find language" << setLanguage;
    } else {
        languageBox->setCurrentIndex(index);
    }

    connect(languageBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &IntroPage::languageBoxChanged);

    auto *layout = new QGridLayout(this);
    layout->addWidget(label, 0, 0, 1, 2);
    layout->addWidget(languageLabel, 1, 0);
    layout->addWidget(languageBox, 1, 1);
    layout->addWidget(versionLabel, 2, 0, 1, 2);

    setLayout(layout);
}

void IntroPage::initializePage()
{
    if (wizard()->backgroundMode) {
        emit readyToContinue();
    }
}

QStringList IntroPage::findQmFiles()
{
    QDir dir(translationPath);
    QStringList fileNames = dir.entryList(QStringList(translationPrefix + "_*.qm"), QDir::Files, QDir::Name);
    fileNames.replaceInStrings(QRegularExpression(translationPrefix + "_(.*)\\.qm"), "\\1");
    return fileNames;
}

QString IntroPage::languageName(const QString &lang)
{
    QTranslator qTranslator;

    QString appNameHint = translationPrefix + "_" + lang;
    bool appTranslationLoaded = qTranslator.load(appNameHint, translationPath);
    if (!appTranslationLoaded) {
        qDebug() << "Unable to load" << translationPrefix << "translation" << appNameHint << "at" << translationPath;
    }

    return qTranslator.translate("i18n", DEFAULT_LANG_NAME);
}

void IntroPage::languageBoxChanged(int index)
{
    SettingsCache::instance().personal().setLang(languageBox->itemData(index).toString());
}

void IntroPage::retranslateUi()
{
    setTitle(tr("Introduction"));
    label->setText(tr("This wizard will import the list of sets, cards, and tokens "
                      "that will be used by Cockatrice."));
    languageLabel->setText(tr("Interface language:"));
    versionLabel->setText(tr("Version:") + QString(" %1").arg(VERSION_STRING));
}

void OutroPage::retranslateUi()
{
    setTitle(tr("Finished"));
    setSubTitle(tr("The wizard has finished.") + "<br>" +
                tr("You can now start using Cockatrice with the newly updated cards.") + "<br><br>" +
                tr("If the card databases don't reload automatically, restart the Cockatrice client."));
}

void OutroPage::initializePage()
{
    if (wizard()->backgroundMode) {
        wizard()->accept();
        exit(0);
    }
}

LoadSetsPage::LoadSetsPage(QWidget *parent) : OracleWizardPage(parent)
{
    urlRadioButton = new QRadioButton(this);
    fileRadioButton = new QRadioButton(this);

    urlLineEdit = new QLineEdit(this);
    fileLineEdit = new QLineEdit(this);

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlRadioButton->setChecked(true);

    urlButton = new QPushButton(this);
    connect(urlButton, &QPushButton::clicked, this, &LoadSetsPage::actRestoreDefaultUrl);

    fileButton = new QPushButton(this);
    connect(fileButton, &QPushButton::clicked, this, &LoadSetsPage::actLoadSetsFile);

    auto *layout = new QGridLayout(this);
    layout->addWidget(urlRadioButton, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(urlButton, 1, 1, Qt::AlignRight);
    layout->addWidget(fileRadioButton, 2, 0);
    layout->addWidget(fileLineEdit, 2, 1);
    layout->addWidget(fileButton, 3, 1, Qt::AlignRight);
    layout->addWidget(progressLabel, 4, 0);
    layout->addWidget(progressBar, 4, 1);

    connect(&watcher, &QFutureWatcher<bool>::finished, this, &LoadSetsPage::importFinished);

    setLayout(layout);
}

bool LoadSetsPage::isComplete() const
{
    return !loadActive;
}

void LoadSetsPage::initializePage()
{
    urlLineEdit->setText(wizard()->settings->value("allsetsurl", ALLSETS_URL).toString());

    progressLabel->hide();
    progressBar->hide();

    if (wizard()->backgroundMode) {
        if (isEnabled()) {
            validatePage();
        }
    }
}

void LoadSetsPage::retranslateUi()
{
    setTitle(tr("Source selection"));
    setSubTitle(tr("Please specify a compatible source for the list of sets and cards. "
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

    QString extensions = "*.json *.xml";
#ifdef HAS_ZLIB
    extensions += " *.zip";
#endif
#ifdef HAS_LZMA
    extensions += " *.xz";
#endif
    dialog.setNameFilter(tr("Sets file (%1)").arg(extensions));

    if (!fileLineEdit->text().isEmpty() && QFile::exists(fileLineEdit->text())) {
        dialog.selectFile(fileLineEdit->text());
    }

    if (!dialog.exec()) {
        return;
    }

    fileLineEdit->setText(dialog.selectedFiles().at(0));
}

bool LoadSetsPage::validatePage()
{
    // once the import is finished, we call next(); skip validation
    if (wizard()->downloadedPlainXml || wizard()->importer->getSets().count() > 0) {
        return true;
    }

    // else, try to import sets
    if (urlRadioButton->isChecked()) {
        // If a user attempts to download from V4, redirect them to V5
        if (urlLineEdit->text().contains(MTGJSON_V4_URL_COMPONENT)) {
            actRestoreDefaultUrl();
        }

        const auto url = QUrl::fromUserInput(urlLineEdit->text());

        if (!url.isValid()) {
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
    } else if (fileRadioButton->isChecked()) {
        QFile setsFile(fileLineEdit->text());
        if (!setsFile.exists()) {
            QMessageBox::critical(this, tr("Error"), tr("Please choose a file."));
            return false;
        }

        wizard()->disableButtons();
        setEnabled(false);

        wizard()->setCardSourceUrl(setsFile.fileName());
        wizard()->setCardSourceVersion("unknown");

        readSetsFromFile(setsFile.fileName());
    }

    return false;
}

void LoadSetsPage::downloadSetsFile(const QUrl &url)
{
    wizard()->setCardSourceVersion("unknown");

    const auto urlString = url.toString();
    if (urlString == ALLSETS_URL || urlString == ALLSETS_URL_FALLBACK) {
        const auto versionUrl = QUrl::fromUserInput(MTGJSON_VERSION_URL);
        QNetworkRequest request = QNetworkRequest(versionUrl);
        request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
        auto *versionReply = wizard()->nam->get(request);
        connect(versionReply, &QNetworkReply::finished, [this, versionReply]() {
            if (versionReply->error() == QNetworkReply::NoError) {
                auto data = versionReply->readAll();
                QJsonParseError jsonError{};
                auto jsonResponse = QJsonDocument::fromJson(data, &jsonError);

                if (jsonError.error == QJsonParseError::NoError) {
                    const auto jsonMap = jsonResponse.toVariant().toMap();

                    auto versionString = jsonMap.value("meta").toMap().value("version").toString();
                    if (versionString.isEmpty()) {
                        versionString = "unknown";
                    }
                    wizard()->setCardSourceVersion(versionString);
                }
            }

            versionReply->deleteLater();
        });
    }

    wizard()->setCardSourceUrl(url.toString());

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    auto *reply = wizard()->nam->get(request);

    connect(reply, &QNetworkReply::finished, this, &LoadSetsPage::actDownloadFinishedSetsFile);
    connect(reply, &QNetworkReply::downloadProgress, this, &LoadSetsPage::actDownloadProgressSetsFile);
}

void LoadSetsPage::actDownloadProgressSetsFile(qint64 received, qint64 total)
{
    if (wizard()->backgroundMode) {
        emitBackgroundProgress("download", received, total);
    }
    if (total > 0) {
        progressBar->setMaximum(static_cast<int>(total));
        progressBar->setValue(static_cast<int>(received));
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int)received / (1024 * 1024)));
}

void LoadSetsPage::actDownloadFinishedSetsFile()
{
    // check for a reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    auto errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));

        wizard()->enableButtons();
        setEnabled(true);

        reply->deleteLater();
        return;
    }

    auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        const auto redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "following redirect url:" << redirectUrl.toString();
        downloadSetsFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    progressLabel->hide();
    progressBar->hide();

    // save AllPrintings.json url, but only if the user customized it and download was successful
    if (urlLineEdit->text() != QString(ALLSETS_URL)) {
        wizard()->settings->setValue("allsetsurl", urlLineEdit->text());
    } else {
        wizard()->settings->remove("allsetsurl");
    }

    readSetsFromByteArray(reply->readAll());
    reply->deleteLater();
}

void LoadSetsPage::updateParsingProgress(int bytesRead, int totalBytes)
{
    if (totalBytes <= 0) {
        return;
    }
    progressBar->setRange(0, totalBytes);
    progressBar->setValue(bytesRead);
    const int percent = static_cast<int>((100.0 * bytesRead) / totalBytes);
    progressLabel->setText(tr("Parsing file (%1%)").arg(percent));
}

void LoadSetsPage::scanProgressToStdout(int bytesRead, int totalBytes)
{
    emitBackgroundProgress("scan", bytesRead, totalBytes);
}

void LoadSetsPage::beginLoadSets(bool compressedFile)
{
    // Show an infinite progressbar while the worker decompresses; the scan
    // steals the label via dataReadProgress as soon as it starts.
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    progressBar->setValue(0);
    progressLabel->setText(compressedFile ? tr("Extracting file...") : tr("Parsing file"));
    progressLabel->show();
    progressBar->show();

    // Keep Next disabled (via completeChanged) until the worker reports in;
    // updateButtonStates() re-evaluates button state whenever we re-enable.
    loadActive = true;
    emit completeChanged();

    wizard()->downloadedPlainXml = false;
    wizard()->xmlData.clear();

    if (wizard()->backgroundMode) {
        connect(wizard()->importer, &OracleImporter::dataReadProgress, this, &LoadSetsPage::scanProgressToStdout,
                Qt::UniqueConnection);
    } else {
        connect(wizard()->importer, &OracleImporter::dataReadProgress, this, &LoadSetsPage::updateParsingProgress,
                Qt::UniqueConnection);
    }
}

void LoadSetsPage::readSetsFromByteArray(QByteArray _data)
{
    const bool compressed = _data.startsWith(XZ_SIGNATURE) || _data.startsWith(ZIP_SIGNATURE);
    beginLoadSets(compressed);

    // Decompress and scan off the UI thread so a large download can't freeze the window.
    const QPointer<OracleImporter> importer = wizard()->importer;
    future = QtConcurrent::run(
        [importer, data = std::move(_data)]() mutable { return loadSetsData(importer, std::move(data)); });
    watcher.setFuture(future);
}

void LoadSetsPage::readSetsFromFile(const QString &fileName)
{
    // Peek at the header on the UI thread so the status text can distinguish
    // "Extracting file..." from a plain JSON parse; the full read happens in the worker.
    QFile headerFile(fileName);
    bool compressed = false;
    if (headerFile.open(QIODevice::ReadOnly)) {
        const QByteArray header = headerFile.read(6);
        compressed = header.startsWith(XZ_SIGNATURE) || header.startsWith(ZIP_SIGNATURE);
    }
    beginLoadSets(compressed);

    // Read, decompress and scan off the UI thread (a plain JSON can be hundreds
    // of MB, so even the read itself must not block the window).
    const QPointer<OracleImporter> importer = wizard()->importer;
    future = QtConcurrent::run([importer, fileName]() mutable -> LoadSetsResult {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            LoadSetsResult readError;
            readError.errorMessage = LoadSetsPage::tr("Cannot open file '%1'.").arg(fileName);
            return readError;
        }
        return loadSetsData(importer, file.readAll());
    });
    watcher.setFuture(future);
}

void LoadSetsPage::cancelWork()
{
    // The scan is short-lived; just wait it out before the wizard (and its
    // importer) can be torn down underneath the worker thread.
    if (future.isRunning()) {
        future.cancel();
        watcher.cancel();
        future.waitForFinished();
    }
}

void LoadSetsPage::zipDownloadFailed(const QString &message)
{
    wizard()->enableButtons();
    setEnabled(true);
    progressLabel->hide();
    progressBar->hide();

    QMessageBox::StandardButton reply;
    reply = static_cast<QMessageBox::StandardButton>(QMessageBox::question(
        this, tr("Error"), message + "<br>" + tr("Do you want to download the uncompressed file instead?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes));

    if (reply == QMessageBox::Yes) {
        urlRadioButton->setChecked(true);
        urlLineEdit->setText(ALLSETS_URL_FALLBACK);

        wizard()->next();
    }
}

void LoadSetsPage::importFinished()
{
    loadActive = false;
    emit completeChanged();
    wizard()->enableButtons();
    setEnabled(true);

    const LoadSetsResult result = watcher.result();

    if (result.plainXml) {
        wizard()->downloadedPlainXml = true;
        wizard()->xmlData = result.xmlData;
    }

    if (wizard()->backgroundMode) {
        progressLabel->hide();
        progressBar->hide();
        if (!result.errorMessage.isEmpty()) {
            qWarning() << result.errorMessage;
        } else if (!result.ok && !result.plainXml) {
            qWarning() << tr("The file was retrieved successfully, but it does not contain any sets data.");
        }
        emit readyToContinue();
        return;
    }

    const auto fail = [this](const QString &message) {
        progressLabel->hide();
        progressBar->hide();
        QMessageBox::critical(this, tr("Error"), message);
    };

    if (!result.errorMessage.isEmpty()) {
        if (result.offerUncompressedFallback) {
            zipDownloadFailed(result.errorMessage);
            return;
        }
        fail(result.errorMessage);
        return;
    }

    if (!result.ok && !result.plainXml) {
        fail(tr("The file was retrieved successfully, but it does not contain any sets data."));
        return;
    }

    // Snap the bar to 100% and hold it there for a moment so the completed state
    // is actually visible before the next page's own import progress takes over
    // (a zero-length deferral can still fire before the repaint is delivered).
    progressBar->setMaximum(1);
    progressBar->setValue(1);
    progressLabel->setText(tr("Parsing file (100%)"));
    QTimer::singleShot(500, this, [this] {
        if (wizard()->currentPage() == this) {
            wizard()->next();
        }
    });
}

SaveSetsPage::SaveSetsPage(QWidget *parent) : OracleWizardPage(parent)
{
    pathLabel = new QLabel(this);
    saveLabel = new QLabel(this);

    progressBar = new QProgressBar(this);
    progressBar->hide();

    defaultPathCheckBox = new QCheckBox(this);

    messageLog = new QTextEdit(this);
    messageLog->setReadOnly(true);

    auto *layout = new QGridLayout(this);
    layout->addWidget(progressBar, 0, 0);
    layout->addWidget(messageLog, 1, 0);
    layout->addWidget(saveLabel, 2, 0);
    layout->addWidget(pathLabel, 3, 0);
    layout->addWidget(defaultPathCheckBox, 4, 0);

    setLayout(layout);
}

bool SaveSetsPage::isComplete() const
{
    return !importActive;
}

void SaveSetsPage::cleanupPage()
{
    cancelWork();
    disconnect(wizard()->importer, &OracleImporter::setIndexChanged, this, &SaveSetsPage::updateTotalProgress);
    disconnect(&importWatcher, &QFutureWatcher<int>::finished, this, &SaveSetsPage::importFinished);
    wizard()->importer->clear();
}

void SaveSetsPage::initializePage()
{
    retranslateUi();
    if (wizard()->downloadedPlainXml) {
        messageLog->hide();
        progressBar->hide();
        if (wizard()->backgroundMode) {
            emit readyToContinue();
        }
        return;
    }

    messageLog->clear();
    messageLog->show();
    progressBar->show();

    totalSets = wizard()->importer->getSets().size();
    progressBar->setRange(0, totalSets);
    progressBar->setValue(0);

    connect(wizard()->importer, &OracleImporter::setIndexChanged, this, &SaveSetsPage::updateTotalProgress,
            Qt::UniqueConnection);
    connect(&importWatcher, &QFutureWatcher<int>::finished, this, &SaveSetsPage::importFinished, Qt::UniqueConnection);

    wizard()->disableButtons();
    importActive = true;
    emit completeChanged();

    const QPointer<OracleImporter> importer = wizard()->importer;
    importFuture = QtConcurrent::run([importer] { return importer ? importer->startImport() : 0; });
    importWatcher.setFuture(importFuture);
}

void SaveSetsPage::cancelWork()
{
    if (!importActive) {
        return;
    }
    // Ask the worker to stop at the next set boundary, then wait it out so the
    // wizard (and the importer it owns) is never torn down under a running thread.
    importActive = false;
    emit completeChanged();
    wizard()->importer->cancelImport();
    importFuture.cancel();
    importWatcher.cancel();
    importFuture.waitForFinished();
}

void SaveSetsPage::importFinished()
{
    if (!importActive) {
        return;
    }
    importActive = false;
    emit completeChanged();

    wizard()->enableButtons();

    const int setsImported = importWatcher.result();
    const QPointer<OracleImporter> importer = wizard()->importer;
    if (importer) {
        importer->releaseSetData();
    }

    if (wizard()->backgroundMode) {
        if (setsImported == 0) {
            qWarning() << tr("No set has been imported.");
        }
        emit readyToContinue();
        return;
    }

    progressBar->setValue(progressBar->maximum());

    if (setsImported == 0) {
        QMessageBox::critical(this, tr("Error"), tr("No set has been imported."));
    }
}

void SaveSetsPage::retranslateUi()
{
    setTitle(tr("Sets imported"));
    if (wizard()->downloadedPlainXml) {
        setSubTitle(tr("A cockatrice database file of %1 MB has been downloaded.")
                        .arg(qRound(wizard()->xmlData.size() / 1000000.0)));
    } else {
        setSubTitle(tr("The following sets have been found:"));
    }

    saveLabel->setText(tr("Press \"Save\" to store the imported cards in the Cockatrice database."));
    pathLabel->setText(tr("The card database will be saved at the following location:") + "<br>" +
                       SettingsCache::instance().getCardDatabasePath());
    defaultPathCheckBox->setText(tr("Save to a custom path (not recommended)"));

    setButtonText(QWizard::NextButton, tr("&Save"));
}

void SaveSetsPage::updateTotalProgress(int cardsImported, int setIndex, const QString &setName)
{
    if (!importActive) {
        return;
    }
    if (setName.isEmpty()) {
        progressBar->setValue(progressBar->maximum());
        const int cardCount = wizard()->importer->getCardList().size();
        if (wizard()->backgroundMode) {
            qInfo() << tr("Import finished: %1 cards.").arg(cardCount);
            emitBackgroundProgress("import", totalSets, totalSets);
        } else {
            messageLog->append("<b>" + tr("Import finished: %1 cards.").arg(cardCount) + "</b>");
        }
    } else {
        progressBar->setValue(setIndex);
        if (wizard()->backgroundMode) {
            qInfo() << tr("%1: %2 cards imported").arg(setName).arg(cardsImported);
            emitBackgroundProgress("import", setIndex, totalSets);
        } else {
            messageLog->append(tr("%1: %2 cards imported").arg(setName).arg(cardsImported));
        }
    }

    messageLog->verticalScrollBar()->setValue(messageLog->verticalScrollBar()->maximum());
}

bool SaveSetsPage::validatePage()
{
    QString defaultPath = SettingsCache::instance().getCardDatabasePath();
    QString windowName = tr("Save card database");
    QString fileType = tr("XML; card database (*.xml)");

    QString fileName;
    if (defaultPathCheckBox->isChecked()) {
        fileName = QFileDialog::getSaveFileName(this, windowName, defaultPath, fileType);
    } else {
        fileName = defaultPath;
    }

    if (fileName.isEmpty()) {
        return false;
    }

    QFileInfo fi(fileName);
    QDir fileDir(fi.path());
    if (!fileDir.exists() && !fileDir.mkpath(fileDir.absolutePath())) {
        return false;
    }

    if (wizard()->downloadedPlainXml) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "File write (w) failed for" << fileName;
            return false;
        }
        if (file.write(wizard()->xmlData) < 1) {
            qDebug() << "File write (w) failed for" << fileName;
            return false;
        }
        wizard()->xmlData.clear();
    } else if (!wizard()->importer->saveToFile(fileName, wizard()->getCardSourceUrl(),
                                               wizard()->getCardSourceVersion())) {
        QMessageBox::critical(this, tr("Error"), tr("The file could not be saved to %1").arg(fileName));
        return false;
    }

    return true;
}

void LoadTokensPage::initializePage()
{
    SimpleDownloadFilePage::initializePage();

    if (wizard()->backgroundMode) {
        emit readyToContinue();
    }
}

QString LoadTokensPage::getDefaultUrl()
{
    return TOKENS_URL;
}

QString LoadTokensPage::getCustomUrlSettingsKey()
{
    return "tokensurl";
}

QString LoadTokensPage::getDefaultSavePath()
{
    return SettingsCache::instance().getTokenDatabasePath();
}

QString LoadTokensPage::getWindowTitle()
{
    return tr("Save token database");
}

QString LoadTokensPage::getFileType()
{
    return tr("XML; token database (*.xml)");
}

QString LoadTokensPage::getFilePromptName()
{
    return tr("tokens");
}

void LoadTokensPage::retranslateUi()
{
    setTitle(tr("Tokens import"));
    setSubTitle(tr("Please specify a compatible source for token data."));

    urlRadioButton->setText(tr("Download URL:"));
    fileRadioButton->setText(tr("Local file:"));
    urlButton->setText(tr("Restore default URL"));
    fileButton->setText(tr("Choose file..."));

    pathLabel->setText(tr("The token database will be saved at the following location:") + "<br>" +
                       SettingsCache::instance().getTokenDatabasePath());
    defaultPathCheckBox->setText(tr("Save to a custom path (not recommended)"));
}

QString LoadSpoilersPage::getDefaultUrl()
{
    return SPOILERS_URL;
}

QString LoadSpoilersPage::getCustomUrlSettingsKey()
{
    return "spoilersurl";
}

QString LoadSpoilersPage::getDefaultSavePath()
{
    return SettingsCache::instance().getTokenDatabasePath();
}

QString LoadSpoilersPage::getWindowTitle()
{
    return tr("Save spoiler database");
}

QString LoadSpoilersPage::getFileType()
{
    return tr("XML; spoiler database (*.xml)");
}

QString LoadSpoilersPage::getFilePromptName()
{
    return tr("spoiler");
}

void LoadSpoilersPage::retranslateUi()
{
    setTitle(tr("Spoilers import"));
    setSubTitle(tr("Please specify a compatible source for spoiler data."));

    urlRadioButton->setText(tr("Download URL:"));
    fileRadioButton->setText(tr("Local file:"));
    urlButton->setText(tr("Restore default URL"));
    fileButton->setText(tr("Choose file..."));

    pathLabel->setText(tr("The spoiler database will be saved at the following location:") + "<br>" +
                       SettingsCache::instance().getSpoilerCardDatabasePath());
    defaultPathCheckBox->setText(tr("Save to a custom path (not recommended)"));
}