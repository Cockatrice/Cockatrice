#include <QtGui>
#if QT_VERSION < 0x050000
    #include <QDesktopServices>
#else 
    #include <QStandardPaths>
    #include <QtConcurrent>
#endif
#include <QAbstractButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
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

#define ALLSETS_URL "http://mtgjson.com/json/AllSets.json"

OracleWizard::OracleWizard(QWidget *parent)
    : QWizard(parent)
{
    settings = new QSettings(this);

    importer = new OracleImporter(
#if QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation)
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first()
#endif
    , this);

    addPage(new IntroPage);
    addPage(new LoadSetsPage);
    addPage(new ChooseSetsPage);
    addPage(new SaveSetsPage);

    setWindowTitle(tr("Oracle Importer"));
    QWizard::setButtonText(QWizard::FinishButton, tr("Save"));
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

IntroPage::IntroPage(QWidget *parent)
    : OracleWizardPage(parent)
{
    setTitle(tr("Introduction"));

    label = new QLabel(tr("This wizard will import the list of sets and cards "
                          "that will be used by Cockatrice. You will need to "
                          "specify an url or a filename that will be used as a "
                          "source, and then choose the wanted sets from the list "
                          "of the available ones."),
                        this);
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);
}

LoadSetsPage::LoadSetsPage(QWidget *parent)
    : OracleWizardPage(parent), nam(0)
{
    setTitle(tr("Source selection"));
    setSubTitle(tr("Please specify a source for the list of sets and cards. "
                   "You can specify an url address that will be download or "
                   "use an existing file from your computer."));

    urlRadioButton = new QRadioButton(tr("Download url:"), this);
    fileRadioButton = new QRadioButton(tr("Local file:"), this);

    urlLineEdit = new QLineEdit(this);
    fileLineEdit = new QLineEdit(this);

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlRadioButton->setChecked(true);

    fileButton = new QPushButton(tr("Choose file..."), this);
    connect(fileButton, SIGNAL(clicked()), this, SLOT(actLoadSetsFile()));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(urlRadioButton, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(fileRadioButton, 1, 0);
    layout->addWidget(fileLineEdit, 1, 1);
    layout->addWidget(fileButton, 2, 1, Qt::AlignRight);
    layout->addWidget(progressLabel, 3, 0);
    layout->addWidget(progressBar, 3, 1);

    connect(&watcher, SIGNAL(finished()), this, SLOT(importFinished()));

    setLayout(layout);
}

void LoadSetsPage::initializePage()
{
    urlLineEdit->setText(wizard()->settings->value("allsetsurl", ALLSETS_URL).toString());

    progressLabel->hide();
    progressBar->hide();
}

void LoadSetsPage::actLoadSetsFile()
{
    QFileDialog dialog(this, tr("Load sets file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Sets JSON file (*.json)");

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
            QMessageBox::critical(this, tr("Error"), tr("The provided url is not valid."));
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

        if(!nam)
            nam = new QNetworkAccessManager(this);
        QNetworkReply *reply = nam->get(QNetworkRequest(url));
 
        connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinishedSetsFile()));
        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(actDownloadProgressSetsFile(qint64, qint64)));

    } else if(fileRadioButton->isChecked()) {
        QFile setsFile(fileLineEdit->text());
        if(!setsFile.exists())
        {
            QMessageBox::critical(this, tr("Error"), tr("Please choose a file."));
            return false;
        }

        if (!setsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, tr("Error"), tr("Cannot open file '%1'.").arg(fileLineEdit->text()));
            return false;
        }

        wizard()->disableButtons();
        setEnabled(false);

        readSetsFromByteArray(setsFile.readAll());

    }
    return false;
}

void LoadSetsPage::actDownloadProgressSetsFile(qint64 received, qint64 total)
{
    if(total > 0 && progressBar->maximum()==0)
    {
        progressBar->setMaximum(total);
        progressBar->setValue(received);
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int) received / 1048576));
}

void LoadSetsPage::actDownloadFinishedSetsFile()
{
    progressLabel->hide();
    progressBar->hide();

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

    // Start the computation.
    future = QtConcurrent::run(wizard()->importer, &OracleImporter::readSetsFromByteArray, data);
    watcher.setFuture(future);
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

ChooseSetsPage::ChooseSetsPage(QWidget *parent)
    : OracleWizardPage(parent)
{
    setTitle(tr("Sets selection"));
    setSubTitle(tr("The following sets has been found in the source file. "
                   "Please mark the sets that will be imported."));

    checkBoxLayout = new QVBoxLayout;
    
    QWidget *checkboxFrame = new QWidget(this);
    checkboxFrame->setLayout(checkBoxLayout);
    
    QScrollArea *checkboxArea = new QScrollArea(this);
    checkboxArea->setWidget(checkboxFrame);
    checkboxArea->setWidgetResizable(true);
    
    checkAllButton = new QPushButton(tr("&Check all"));
    connect(checkAllButton, SIGNAL(clicked()), this, SLOT(actCheckAll()));
    uncheckAllButton = new QPushButton(tr("&Uncheck all"));
    connect(uncheckAllButton, SIGNAL(clicked()), this, SLOT(actUncheckAll()));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(checkboxArea, 0, 0, 1, 2);
    layout->addWidget(checkAllButton, 1, 0);
    layout->addWidget(uncheckAllButton, 1, 1);

    setLayout(layout);
}

void ChooseSetsPage::initializePage()
{
    // populate checkbox list
    for (int i = 0; i < checkBoxList.size(); ++i)
        delete checkBoxList[i];
    checkBoxList.clear();
    
    QList<SetToDownload> &sets = wizard()->importer->getSets();
    for (int i = 0; i < sets.size(); ++i) {
        QCheckBox *checkBox = new QCheckBox(sets[i].getLongName());
        checkBox->setChecked(sets[i].getImport());
        connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
        checkBoxLayout->addWidget(checkBox);
        checkBoxList << checkBox;
    }
}

void ChooseSetsPage::checkBoxChanged(int state)
{
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
    QList<SetToDownload> &sets = wizard()->importer->getSets();
    for (int i = 0; i < sets.size(); ++i)
        if (sets[i].getLongName() == checkBox->text()) {
            sets[i].setImport(state);
            break;
        }
}

void ChooseSetsPage::actCheckAll()
{
    for (int i = 0; i < checkBoxList.size(); ++i)
        checkBoxList[i]->setChecked(true);
}

void ChooseSetsPage::actUncheckAll()
{
    for (int i = 0; i < checkBoxList.size(); ++i)
        checkBoxList[i]->setChecked(false);
}

bool ChooseSetsPage::validatePage()
{
    for (int i = 0; i < checkBoxList.size(); ++i)
    {
        if(checkBoxList[i]->isChecked())
            return true;
    }

    QMessageBox::critical(this, tr("Error"), tr("Please mark at least one set."));
    return false;        
}

SaveSetsPage::SaveSetsPage(QWidget *parent)
    : OracleWizardPage(parent)
{
    setTitle(tr("Sets imported"));
    setSubTitle(tr("The following sets has been imported. "
                   "Press \"Save\" to save the imported cards to the Cockatrice database."));

    defaultPathCheckBox = new QCheckBox(this);
    defaultPathCheckBox->setText(tr("Save to the default path (recommended)"));
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
    const QString dataDir = 
#if QT_VERSION < 0x050000
        QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
        QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
#endif
    QSettings* settings = new QSettings(this);
    QString savePath = settings->value("paths/carddatabase").toString();
    if (savePath.isEmpty()) {
        QDir().mkpath(dataDir);
    }
    QString windowName = tr("Save card database");
    QString fileType = tr("XML; card database (*.xml)");
    do {
        QString fileName;
        if (savePath.isEmpty()) {
            if (defaultPathCheckBox->isChecked())
                fileName = dataDir + "/cards.xml";
            else
                fileName = QFileDialog::getSaveFileName(this, windowName, dataDir + "/cards.xml", fileType);
            settings->setValue("paths/carddatabase", fileName);
        }
        else {
            if (defaultPathCheckBox->isChecked())
                fileName = savePath;
            else
                fileName = QFileDialog::getSaveFileName(this, windowName, savePath, fileType);
        }
        if (fileName.isEmpty()) {
            return false;
        }
        if (wizard()->importer->saveToFile(fileName))
        {
            ok = true;
            QMessageBox::information(this, tr("Success"), tr("The card database has been saved successfully."));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("The file could not be saved to the desired location."));
            if (defaultPathCheckBox->isChecked())
                defaultPathCheckBox->setChecked(false);
        }
    } while (!ok);

    return true;
}
