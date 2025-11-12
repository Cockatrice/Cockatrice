#include "oraclewizard.h"

#include "client/settings/cache_settings.h"
#include "main.h"
#include "oracleimporter.h"
#include "pages.h"
#include "pagetemplates.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QScrollBar>
#include <QtConcurrent>
#include <QtGui>

OracleWizard::OracleWizard(QWidget *parent) : QWizard(parent)
{
    // define a dummy context that will be used where needed
    QString dummy = QT_TRANSLATE_NOOP("i18n", "English");

    settings = new QSettings(SettingsCache::instance().getSettingsPath() + "global.ini", QSettings::IniFormat, this);
    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &OracleWizard::updateLanguage);

    importer = new OracleImporter(this);

    nam = new QNetworkAccessManager(this);

    QList<OracleWizardPage *> pages;

    if (!isSpoilersOnly) {
        pages << new IntroPage << new LoadSetsPage << new SaveSetsPage << new LoadTokensPage << new OutroPage;
    } else {
        pages << new LoadSpoilersPage << new OutroPage;
    }

    for (OracleWizardPage *page : pages) {
        addPage(page);

        // Connect background auto-advance
        connect(page, &OracleWizardPage::readyToContinue, this, [this]() {
            if (backgroundMode) {
                next();
            }
        });
    }

    retranslateUi();
}

void OracleWizard::updateLanguage()
{
    qApp->removeTranslator(translator);
    installNewTranslator();
}

void OracleWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }

    QDialog::changeEvent(event);
}

void OracleWizard::retranslateUi()
{
    setWindowTitle(tr("Oracle Importer"));
    for (int i = 0; i < pageIds().count(); i++) {
        dynamic_cast<OracleWizardPage *>(page(i))->retranslateUi();
    }
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

bool OracleWizard::saveTokensToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "File open (w) failed for" << fileName;
        return false;
    }

    if (file.write(tokensData) == -1) {
        qDebug() << "File write (w) failed for" << fileName;
        return false;
    }

    file.close();
    return true;
}
