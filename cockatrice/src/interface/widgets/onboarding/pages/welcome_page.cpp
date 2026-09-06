#include "welcome_page.h"

#include "../../../../main.h"
#include "../../client/settings/cache_settings.h"
#include "../../settings_page/general_settings_page.h"
#include "libcockatrice/settings/personal_settings.h"

#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QTranslator>
#include <QVBoxLayout>

WelcomePage::WelcomePage(QWidget *parent) : FirstRunWizardPage(parent)
{
    bodyLabel = new QLabel(this);
    bodyLabel->setWordWrap(true);
    bodyLabel->setAlignment(Qt::AlignCenter);

    languageLabel = new QLabel(this);
    langCombo = new QComboBox(this);
    for (const QString &code : GeneralSettingsPage::findQmFiles()) {
        langCombo->addItem(GeneralSettingsPage::languageName(code), code);
    }

    QString current = SettingsCache::instance().personal().getLang();
    if (current.isEmpty()) {
        current = QLocale::system().name();
    }
    int index = langCombo->findData(current);
    if (index < 0) {
        index = langCombo->findData(current.section('_', 0, 0));
    }
    if (index >= 0) {
        langCombo->setCurrentIndex(index);
    }

    connect(langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WelcomePage::languageChanged);

    auto *languageRow = new QHBoxLayout;
    languageRow->addStretch();
    languageRow->addWidget(languageLabel);
    languageRow->addWidget(langCombo);
    languageRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(bodyLabel);
    layout->addStretch();
    layout->addLayout(languageRow);

    retranslateUi();
}

void WelcomePage::languageChanged(int index)
{
    if (index < 0) {
        return;
    }
    SettingsCache::instance().personal().setLang(langCombo->itemData(index).toString());
    qApp->removeTranslator(translator); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    installNewTranslator();
}

QString WelcomePage::stepTitle() const
{
    return tr("Welcome!");
}

void WelcomePage::retranslateUi()
{
    bodyLabel->setText(tr("Let's get you set up. This will only take a minute — "
                          "we'll grab the card database, pick a look you like, "
                          "and get you ready to connect to a server.\n\n"
                          "You can change any of this later from Settings."));
    languageLabel->setText(tr("Language:"));
}
