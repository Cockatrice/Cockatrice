#include "first_run_wizard.h"

#include "first_run_wizard_page.h"
#include "pages/account_setup_page.h"
#include "pages/card_database_setup_page.h"
#include "pages/finish_page.h"
#include "pages/preferences_setup_page.h"
#include "pages/theme_setup_page.h"
#include "pages/welcome_page.h"
#include "shader_banner_widget.h"
#include "step_indicator_widget.h"

#include <QCloseEvent>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

FirstRunWizard::FirstRunWizard(QWidget *parent) : QDialog(parent)
{
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumSize(640, 490);
    resize(720, 550);

    bannerHost = new BannerHost(this);

    titleLabel = new QLabel(this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() * 1.4);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    subtitleLabel = new QLabel(this);
    subtitleLabel->setWordWrap(true);

    stack = new QStackedWidget(this);
    stepIndicator = new StepIndicatorWidget(this);

    backButton = new QPushButton(this);
    skipButton = new QPushButton(this);
    nextButton = new QPushButton(this);
    nextButton->setDefault(true);

    connect(backButton, &QPushButton::clicked, this, &FirstRunWizard::goBack);
    connect(skipButton, &QPushButton::clicked, this, &FirstRunWizard::skip);
    connect(nextButton, &QPushButton::clicked, this, &FirstRunWizard::goNext);

    auto *headerLayout = new QVBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(bannerHost);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    auto *navLayout = new QHBoxLayout;
    navLayout->addWidget(backButton);
    navLayout->addWidget(skipButton);
    navLayout->addStretch();
    navLayout->addWidget(stepIndicator);
    navLayout->addStretch();
    navLayout->addWidget(nextButton);

    auto *root = new QVBoxLayout(this);
    root->addLayout(headerLayout);
    root->addSpacing(8);
    root->addWidget(stack, 1);
    root->addSpacing(8);
    root->addLayout(navLayout);

    auto *welcome = new WelcomePage(this);
    auto *cardDb = new CardDatabaseSetupPage(this);
    auto *theme = new ThemeSetupPage(this);
    auto *account = new AccountSetupPage(this);
    auto *prefs = new PreferencesSetupPage(this);
    auto *finishPg = new FinishPage(this);

    cardDatabasePage = cardDb;

    connect(cardDb, &CardDatabaseSetupPage::updateRequested, this, &FirstRunWizard::cardDatabaseUpdateRequested);
    connect(cardDb, &CardDatabaseSetupPage::manualSetupRequested, this,
            &FirstRunWizard::manualCardDatabaseSetupRequested);
    connect(account, &AccountSetupPage::registerRequested, this, &FirstRunWizard::registerRequested);
    connect(account, &AccountSetupPage::connectRequested, this, &FirstRunWizard::connectRequested);

    connect(cardDb, &CardDatabaseSetupPage::advanceRequested, this, [this] {
        if (stack->currentWidget() == cardDatabasePage) {
            showPage(currentIndex + 1);
        }
    });

    addPage(welcome);
    addPage(cardDb);
    addPage(theme);
    addPage(account);
    addPage(prefs);
    addPage(finishPg);

    stepIndicator->setStepCount(pages.count());
    retranslateUi();
    showPage(0);
}

void FirstRunWizard::addPage(FirstRunWizardPage *page)
{
    pages.append(page);
    stack->addWidget(page);
    connect(page, &FirstRunWizardPage::completeChanged, this, &FirstRunWizard::updateChrome);
}

void FirstRunWizard::showPage(int index)
{
    if (index < 0 || index >= pages.count()) {
        return;
    }
    currentIndex = index;
    stack->setCurrentIndex(index);
    pages[index]->initializePage();
    stepIndicator->setCurrentStep(index);
    static const QList<BannerHost::Motif> motifs = {
        BannerHost::Motif::Welcome, BannerHost::Motif::CardDatabase, BannerHost::Motif::Theming,
        BannerHost::Motif::Account, BannerHost::Motif::Preferences,  BannerHost::Motif::Finish,
    };
    if (index < motifs.size()) {
        bannerHost->setMotif(motifs[index]);
    }
    titleLabel->setText(pages[index]->stepTitle());
    subtitleLabel->setText(pages[index]->stepSubtitle());
    subtitleLabel->setVisible(!pages[index]->stepSubtitle().isEmpty());
    updateChrome();
}

void FirstRunWizard::updateChrome()
{
    if (currentIndex < 0) {
        return;
    }
    FirstRunWizardPage *page = pages[currentIndex];
    const bool isLast = (currentIndex == pages.count() - 1);

    backButton->setVisible(currentIndex > 0);
    skipButton->setVisible(page->isSkippable());
    nextButton->setEnabled(page->isComplete());

    QString customText = page->nextButtonText();
    if (!customText.isEmpty()) {
        nextButton->setText(customText);
    } else {
        nextButton->setText(isLast ? tr("Finish") : tr("Next"));
    }
}

void FirstRunWizard::goNext()
{
    FirstRunWizardPage *page = pages[currentIndex];
    if (!page->validatePage() || !page->handleNextClick()) {
        return;
    }
    if (currentIndex == pages.count() - 1) {
        finish();
        return;
    }
    showPage(currentIndex + 1);
}

void FirstRunWizard::goBack()
{
    showPage(currentIndex - 1);
}

void FirstRunWizard::skip()
{
    showPage(currentIndex + 1);
}

void FirstRunWizard::onCardDatabaseUpdateFinished(bool success)
{
    if (cardDatabasePage) {
        cardDatabasePage->onUpdateFinished(success);
    }
}

void FirstRunWizard::finish()
{
    accept();
}

void FirstRunWizard::closeEvent(QCloseEvent *event)
{
    // Every step persists its own choice as it's made, so closing early
    // isn't destructive -- treat it exactly like reaching the end.
    QDialog::closeEvent(event);
}

void FirstRunWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void FirstRunWizard::retranslateUi()
{
    setWindowTitle(tr("Welcome to Cockatrice"));
    backButton->setText(tr("Back"));
    skipButton->setText(tr("Skip"));
    for (FirstRunWizardPage *page : std::as_const(pages)) {
        page->retranslateUi();
    }
    if (currentIndex >= 0) {
        titleLabel->setText(pages[currentIndex]->stepTitle());
        subtitleLabel->setText(pages[currentIndex]->stepSubtitle());
    }
    updateChrome();
}
