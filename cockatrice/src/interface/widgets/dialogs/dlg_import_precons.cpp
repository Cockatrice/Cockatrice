#include "dlg_import_precons.h"

#include "../deck/deck_loader.h"
#include "../settings/cache_settings.h"

#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QTemporaryDir>
#include <decklist.h>

#ifdef HAS_LZMA
#include "../../src/utility/external/lzma/decompress.h"
#endif

#ifdef HAS_ZLIB
#include "../../src/utility/external/zip/unzip.h"
#endif

#define ZIP_SIGNATURE "PK"
// Xz stream header: 0xFD + "7zXZ"
#define XZ_SIGNATURE "\xFD\x37\x7A\x58\x5A"
#define MTGJSON_V4_URL_COMPONENT "mtgjson.com/files/"
#define MTGJSON_VERSION_URL "https://www.mtgjson.com/api/v5/Meta.json"

#define ALLDECKS_URL_FALLBACK "https://mtgjson.com/api/v5/AllDeckFiles.zip"

#ifdef HAS_LZMA
#define ALLDECKS_URL "https://mtgjson.com/api/v5/AllDeckFiles.tar.xz"
#elif defined(HAS_ZLIB)
#define ALLDECKS_URL "https://mtgjson.com/api/v5/AllDeckFiles.zip"
#else
#define ALLDECKS_URL ""
#endif

DlgImportPrecons::DlgImportPrecons(QWidget *parent) : QWizard(parent)
{
    // define a dummy context that will be used where needed
    QString dummy = QT_TRANSLATE_NOOP("i18n", "English");

    nam = new QNetworkAccessManager(this);

    addPage(new LoadPreconsPage);
    addPage(new SavePreconsPage);

    retranslateUi();
}

void DlgImportPrecons::retranslateUi()
{
    setWindowTitle(tr("Preconstructed Deck Importer"));
}

void DlgImportPrecons::accept()
{
    QDialog::accept();
}

void DlgImportPrecons::enableButtons()
{
    button(QWizard::NextButton)->setDisabled(false);
    button(QWizard::BackButton)->setDisabled(false);
}

void DlgImportPrecons::disableButtons()
{
    button(QWizard::NextButton)->setDisabled(true);
    button(QWizard::BackButton)->setDisabled(true);
}

LoadPreconsPage::LoadPreconsPage(QWidget *parent) : QWizardPage(parent)
{
    urlRadioButton = new QRadioButton(this);
    fileRadioButton = new QRadioButton(this);

    urlLineEdit = new QLineEdit(this);
    fileLineEdit = new QLineEdit(this);

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlRadioButton->setChecked(true);

    urlButton = new QPushButton(this);
    connect(urlButton, &QPushButton::clicked, this, &LoadPreconsPage::actRestoreDefaultUrl);

    fileButton = new QPushButton(this);
    connect(fileButton, &QPushButton::clicked, this, &LoadPreconsPage::actLoadPreconsFile);

    auto *layout = new QGridLayout(this);
    layout->addWidget(urlRadioButton, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(urlButton, 1, 1, Qt::AlignRight);
    layout->addWidget(fileRadioButton, 2, 0);
    layout->addWidget(fileLineEdit, 2, 1);
    layout->addWidget(fileButton, 3, 1, Qt::AlignRight);
    layout->addWidget(progressLabel, 4, 0);
    layout->addWidget(progressBar, 4, 1);

    connect(&watcher, &QFutureWatcher<bool>::finished, this, &LoadPreconsPage::importFinished);

    setLayout(layout);

    retranslateUi();
}

void LoadPreconsPage::initializePage()
{
    urlLineEdit->setText(ALLDECKS_URL);

    progressLabel->hide();
    progressBar->hide();
}

void LoadPreconsPage::retranslateUi()
{
    setTitle(tr("Source selection"));
    setSubTitle(tr("Please specify a compatible source for the list of preconstructed Decks. "
                   "You can specify a URL address that will be downloaded or "
                   "use an existing file from your computer."));

    urlRadioButton->setText(tr("Download URL:"));
    fileRadioButton->setText(tr("Local file:"));
    urlButton->setText(tr("Restore default URL"));
    fileButton->setText(tr("Choose file..."));
}

void LoadPreconsPage::actRestoreDefaultUrl()
{
    urlLineEdit->setText(ALLDECKS_URL);
}

void LoadPreconsPage::actLoadPreconsFile()
{
    QFileDialog dialog(this, tr("Load preconstructed Deck file"));
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString extensions = "*.json *.xml";
#ifdef HAS_ZLIB
    extensions += " *.zip";
#endif
#ifdef HAS_LZMA
    extensions += " *.xz";
#endif
    dialog.setNameFilter(tr("Precons file (%1)").arg(extensions));

    if (!fileLineEdit->text().isEmpty() && QFile::exists(fileLineEdit->text())) {
        dialog.selectFile(fileLineEdit->text());
    }

    if (!dialog.exec()) {
        return;
    }

    fileLineEdit->setText(dialog.selectedFiles().at(0));
}

bool LoadPreconsPage::validatePage()
{
    // once the import is finished, we call next(); skip validation
    if (dynamic_cast<DlgImportPrecons *>(wizard())->doneWithParsing) {
        return true;
    }

    if (urlRadioButton->isChecked()) {
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

        dynamic_cast<DlgImportPrecons *>(wizard())->disableButtons();
        setEnabled(false);

        downloadPreconsFile(url);
    } else if (fileRadioButton->isChecked()) {
        QFile setsFile(fileLineEdit->text());
        if (!setsFile.exists()) {
            QMessageBox::critical(this, tr("Error"), tr("Please choose a file."));
            return false;
        }

        if (!setsFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(nullptr, tr("Error"), tr("Cannot open file '%1'.").arg(fileLineEdit->text()));
            return false;
        }

        dynamic_cast<DlgImportPrecons *>(wizard())->disableButtons();
        setEnabled(false);

        dynamic_cast<DlgImportPrecons *>(wizard())->setCardSourceUrl(setsFile.fileName());

        readPreconsFromByteArray(setsFile.readAll());
    }

    return false;
}

void LoadPreconsPage::downloadPreconsFile(const QUrl &url)
{
    const auto urlString = url.toString();
    if (urlString == ALLDECKS_URL || urlString == ALLDECKS_URL_FALLBACK) {
        const auto versionUrl = QUrl::fromUserInput(MTGJSON_VERSION_URL);
        auto *versionReply = dynamic_cast<DlgImportPrecons *>(wizard())->nam->get(QNetworkRequest(versionUrl));
        connect(versionReply, &QNetworkReply::finished, [versionReply]() {
            if (versionReply->error() == QNetworkReply::NoError) {
                auto jsonData = versionReply->readAll();
                QJsonParseError jsonError{};
                auto jsonResponse = QJsonDocument::fromJson(jsonData, &jsonError);

                if (jsonError.error == QJsonParseError::NoError) {
                    const auto jsonMap = jsonResponse.toVariant().toMap();

                    auto versionString = jsonMap.value("meta").toMap().value("version").toString();
                    if (versionString.isEmpty()) {
                        versionString = "unknown";
                    }
                }
            }

            versionReply->deleteLater();
        });
    }

    dynamic_cast<DlgImportPrecons *>(wizard())->setCardSourceUrl(url.toString());

    auto *reply = dynamic_cast<DlgImportPrecons *>(wizard())->nam->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, &LoadPreconsPage::actDownloadFinishedPreconsFile);
    connect(reply, &QNetworkReply::downloadProgress, this, &LoadPreconsPage::actDownloadProgressPreconsFile);
}

void LoadPreconsPage::actDownloadProgressPreconsFile(qint64 received, qint64 total)
{
    if (total > 0) {
        progressBar->setMaximum(static_cast<int>(total));
        progressBar->setValue(static_cast<int>(received));
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int)received / (1024 * 1024)));
}

void LoadPreconsPage::actDownloadFinishedPreconsFile()
{
    // check for a reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    auto errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));

        dynamic_cast<DlgImportPrecons *>(wizard())->enableButtons();
        setEnabled(true);

        reply->deleteLater();
        return;
    }

    auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        const auto redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "following redirect url:" << redirectUrl.toString();
        downloadPreconsFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    progressLabel->hide();
    progressBar->hide();

    readPreconsFromByteArray(reply->readAll());
    reply->deleteLater();
}

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

bool LoadPreconsPage::parsePreconsFromByteArray(const QByteArray &data, QString folderPath)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    QJsonObject rootObj = doc.object();
    QJsonObject preconData = rootObj.value("data").toObject();

    QString deckName = preconData.value("name").toString();
    QString shortName = preconData.value("code").toString().toUpper();
    QString deckType = preconData.value("type").toString();
    QJsonArray mainBoard = preconData.value("mainBoard").toArray();
    int releaseYear = preconData.value("releaseDate").toString().split("-").at(0).toInt();

    qInfo() << "Importing '" << deckName << "' from" << shortName;

    auto *precon = new DeckLoader();

    for (const auto &cardVal : mainBoard) {
        QJsonObject cardObj = cardVal.toObject();
        QString name = cardObj.value("name").toString();
        QString setCode = cardObj.value("setCode").toString();
        QString number = cardObj.value("number").toString();
        int count = cardObj.value("count").toInt();
        QString scryfallId = cardObj.value("identifiers").toObject().value("scryfallId").toString();

        DecklistCardNode *addedCard = precon->addCard(name, "main", -1, setCode, number, scryfallId);
        if (count != 1) {
            addedCard->setNumber(count);
        }
    }

    precon->setName(deckName);

    QJsonArray commanderArray = preconData.value("commander").toArray();
    if (!commanderArray.isEmpty()) {
        QJsonObject commanderObj = commanderArray.first().toObject();
        QString commanderName = commanderObj.value("name").toString();
        QString commanderId = commanderObj.value("identifiers").toObject().value("scryfallId").toString();
        precon->setBannerCard(QPair<QString, QString>(commanderName, commanderId));
    } else {
        qInfo() << "No commander data found.";
    }

    QString dirPath = QDir::cleanPath(folderPath + QDir::separator() + deckType + QDir::separator() +
                                      QString::number(releaseYear) + QDir::separator() + shortName);

    QString fullPath = QDir(dirPath).filePath(precon->getName());

    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            qWarning() << "Failed to create directory:" << dirPath;
            return false;
        }
    }

    if (precon->getCardList().length() > 1) {
        precon->saveToFile(fullPath + ".cod", DeckLoader::CockatriceFormat);
    }

    return true;
}

void LoadPreconsPage::readPreconsFromByteArray(QByteArray _data)
{
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    progressBar->setValue(0);
    progressLabel->setText(tr("Parsing file"));
    progressLabel->show();
    progressBar->show();

    dynamic_cast<DlgImportPrecons *>(wizard())->doneWithParsing = false;
    dynamic_cast<DlgImportPrecons *>(wizard())->xmlData.clear();
    readPreconsFromByteArrayRef(_data);
}

QString LoadPreconsPage::createTmpDirectory()
{
    QTemporaryDir tempDir;
    if (tempDir.isValid()) {
        return tempDir.path();
    }
    QString tmpPath = QDir::cleanPath(SettingsCache::instance().getDeckPath() + "/Precons/tmp");
    QDir tmpDir(tmpPath);
    if (!tmpDir.exists()) {
        if (!QDir().mkpath(tmpPath)) {
            qWarning() << "Failed to create temporary directory.";
            return "";
        }
        return tmpPath;
    }
    return tmpPath;
}

void LoadPreconsPage::processTarArchive(const QByteArray &tarData)
{
    const int blockSize = 512;
    int offset = 0;

    dynamic_cast<DlgImportPrecons *>(wizard())->setTempDir(createTmpDirectory());

    while (offset + blockSize <= tarData.size()) {
        QByteArray header = tarData.mid(offset, blockSize);
        QString fileName = QString::fromLatin1(header.left(100).trimmed());
        if (fileName.isEmpty())
            break;

        QByteArray sizeField = header.mid(124, 12).trimmed();
        bool ok = false;
        int fileSize = sizeField.toInt(&ok, 8);
        if (!ok || fileSize < 0)
            break;

        int fileStart = offset + blockSize;
        QByteArray fileContents = tarData.mid(fileStart, fileSize);

        parsePreconsFromByteArray(fileContents, dynamic_cast<DlgImportPrecons *>(wizard())->getTempDir());

        offset = fileStart + ((fileSize + blockSize - 1) / blockSize) * blockSize;
    }

    dynamic_cast<DlgImportPrecons *>(wizard())->doneWithParsing = true;

    importFinished();
}

void LoadPreconsPage::readPreconsFromByteArrayRef(QByteArray &_data)
{
    // XZ-compressed TAR archive
    if (_data.startsWith(XZ_SIGNATURE)) {
#ifdef HAS_LZMA
        qInfo() << "Unzipping precon tar.xz file";
        auto *inBuffer = new QBuffer(&_data);
        QByteArray tarData;
        auto *outBuffer = new QBuffer(&tarData);
        inBuffer->open(QBuffer::ReadOnly);
        outBuffer->open(QBuffer::WriteOnly);
        XzDecompressor xz;
        if (!xz.decompress(inBuffer, outBuffer)) {
            zipDownloadFailed(tr("Xz extraction failed."));
            return;
        }
        _data.clear();
        processTarArchive(tarData);
        return;
#else
        zipDownloadFailed(tr("Sorry, your computer does not support xz compressed files."));
        static_cast<DlgImportPrecons *>(wizard())->enableButtons();
        setEnabled(true);
        progressLabel->hide();
        progressBar->hide();
        return;
#endif
    }

    // ZIP archive
    else if (_data.startsWith(ZIP_SIGNATURE)) {
#ifdef HAS_ZLIB
        qInfo() << "Unzipping precon ZIP file";
        QBuffer inBuffer(&_data);
        UnZip uz;
        UnZip::ErrorCode ec = uz.openArchive(&inBuffer);

        if (ec != UnZip::Ok) {
            zipDownloadFailed(tr("Failed to open Zip archive: %1.").arg(uz.formatError(ec)));
            return;
        }

        const QStringList files = uz.fileList();
        if (files.isEmpty()) {
            zipDownloadFailed(tr("Zip extraction failed: the Zip archive is empty."));
            return;
        }

        for (const QString &fileName : files) {
            if (!fileName.endsWith(".json", Qt::CaseInsensitive))
                continue;

            QBuffer *outBuffer = new QBuffer();
            outBuffer->open(QIODevice::ReadWrite);
            ec = uz.extractFile(fileName, outBuffer);
            if (ec != UnZip::Ok) {
                zipDownloadFailed(tr("Zip extraction failed for file %1: %2").arg(fileName, uz.formatError(ec)));
                uz.closeArchive();
                return;
            }

            outBuffer->seek(0);
            delete outBuffer;
        }

        uz.closeArchive();
        importFinished(); // Continue processing
        return;
#else
        zipDownloadFailed(tr("Sorry, your computer does not support zipped files."));
        static_cast<DlgImportPrecons *>(wizard())->enableButtons();
        setEnabled(true);
        progressLabel->hide();
        progressBar->hide();
        return;
#endif
    }

    // Raw JSON content
    else if (_data.startsWith("{")) {
        jsonData = std::move(_data);
        watcher.setFuture(future);
    }

    // XML content
    else if (_data.startsWith("<")) {
        dynamic_cast<DlgImportPrecons *>(wizard())->doneWithParsing = true;
        dynamic_cast<DlgImportPrecons *>(wizard())->xmlData = std::move(_data);
        importFinished();
    }

    // Unknown format
    else {
        static_cast<DlgImportPrecons *>(wizard())->enableButtons();
        setEnabled(true);
        progressLabel->hide();
        progressBar->hide();
        QMessageBox::critical(this, tr("Error"), tr("Failed to interpret downloaded data."));
    }
}

void LoadPreconsPage::zipDownloadFailed(const QString &message)
{
    dynamic_cast<DlgImportPrecons *>(wizard())->enableButtons();
    setEnabled(true);
    progressLabel->hide();
    progressBar->hide();

    QMessageBox::StandardButton reply;
    reply = static_cast<QMessageBox::StandardButton>(QMessageBox::question(
        this, tr("Error"), message + "<br>" + tr("Do you want to download the uncompressed file instead?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes));

    if (reply == QMessageBox::Yes) {
        urlRadioButton->setChecked(true);
        urlLineEdit->setText(ALLDECKS_URL_FALLBACK);

        wizard()->next();
    }
}

void LoadPreconsPage::importFinished()
{
    dynamic_cast<DlgImportPrecons *>(wizard())->enableButtons();
    setEnabled(true);
    progressLabel->hide();
    progressBar->hide();

    if (dynamic_cast<DlgImportPrecons *>(wizard())->doneWithParsing || watcher.future().result()) {
        wizard()->next();
    } else {
        QMessageBox::critical(
            this, tr("Error"),
            tr("The file was retrieved successfully, but it does not contain any preconstructed decks data."));
    }
}

SavePreconsPage::SavePreconsPage(QWidget *parent) : QWizardPage(parent)
{
    saveLabel = new QLabel(this);

    folderTreeWidget = new QTreeWidget(this);
    folderTreeWidget->setHeaderHidden(true);
    folderTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    folderTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *layout = new QGridLayout(this);
    layout->addWidget(saveLabel, 0, 0);
    layout->addWidget(folderTreeWidget, 1, 0);

    setLayout(layout);

    retranslateUi();
}

void SavePreconsPage::cleanupPage()
{
}

void SavePreconsPage::initializePage()
{
    QDir tempDir(dynamic_cast<DlgImportPrecons *>(wizard())->getTempDir());
    folderTreeWidget->clear();

    for (const QString &dirName : tempDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString absPath = tempDir.absoluteFilePath(dirName);
        QTreeWidgetItem *item = new QTreeWidgetItem(folderTreeWidget, QStringList() << dirName);
        item->setData(0, Qt::UserRole, absPath);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        item->setCheckState(0, Qt::Unchecked);

        populateFolderTree(item, absPath);
    }

    retranslateUi();
}

void SavePreconsPage::populateFolderTree(QTreeWidgetItem *parent, const QString &path)
{
    QDir dir(path);
    for (const QString &subdir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString absPath = dir.absoluteFilePath(subdir);
        QTreeWidgetItem *child = new QTreeWidgetItem(parent, QStringList() << subdir);
        child->setData(0, Qt::UserRole, absPath);
        child->setFlags(child->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        child->setCheckState(0, Qt::Unchecked);

        populateFolderTree(child, absPath);
    }
}

void SavePreconsPage::retranslateUi()
{
    setTitle(tr("Precons imported"));
    setSubTitle(tr("The following preconstructed deck types have been found:"));

    saveLabel->setText(tr("Select the product types you'd like to import and then press \"Save\" to store the imported "
                          "preconstructed decks in your deck folder. \n (Note: It is not recommended to import all "
                          "products unless you are sure your computer can handle it. \n It might cause Cockatrice to "
                          "load a very large amount of decks when using the visual deck storage"));

    setButtonText(QWizard::NextButton, tr("&Save"));
}

bool SavePreconsPage::validatePage()
{
    for (int i = 0; i < folderTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = folderTreeWidget->topLevelItem(i);
        copyCheckedFolders(item);
    }

    QDir(dynamic_cast<DlgImportPrecons *>(wizard())->getTempDir()).removeRecursively();
    return true;
}

void SavePreconsPage::copyCheckedFolders(QTreeWidgetItem *item)
{
    Qt::CheckState state = item->checkState(0);
    if (state == Qt::Unchecked)
        return;

    QString srcPath = item->data(0, Qt::UserRole).toString();
    QString relativePath = QDir(dynamic_cast<DlgImportPrecons *>(wizard())->getTempDir()).relativeFilePath(srcPath);
    QString destPath = QDir::cleanPath(SettingsCache::instance().getDeckPath() + QDir::separator() + "Precons" +
                                       QDir::separator() + relativePath);

    if (!copyDirectory(srcPath, destPath))
        qWarning() << "Failed to copy" << srcPath;

    for (int i = 0; i < item->childCount(); ++i) {
        copyCheckedFolders(item->child(i));
    }
}

bool SavePreconsPage::copyDirectory(const QString &srcPath, const QString &destPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return false;

    QDir destDir;
    if (!destDir.mkpath(destPath))
        return false;

    QFileInfoList entries = srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries) {
        QString src = entry.absoluteFilePath();
        QString dest = destPath + "/" + entry.fileName();

        if (entry.isDir()) {
            if (!copyDirectory(src, dest))
                return false;
        } else {
            if (!QFile::copy(src, dest))
                return false;
        }
    }

    return true;
}