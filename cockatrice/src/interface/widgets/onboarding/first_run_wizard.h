#ifndef FIRST_RUN_WIZARD_H
#define FIRST_RUN_WIZARD_H

#include <QDialog>
#include <QList>

class BannerHost;
class FirstRunWizardPage;
class StepIndicatorWidget;
class CardDatabaseSetupPage;
class QLabel;
class QPushButton;
class QStackedWidget;

/** @brief Polished first-run onboarding flow: card database setup, theme
 *         selection, server account setup, and a handful of key preferences.
 *
 * Deliberately ignorant of network/registration/download internals --
 * pages that need them emit request signals for MainWindow to fulfill.
 * Every choice is written to SettingsCache as it's made (via the pages
 * themselves, same as AppearanceSettingsPage does), so "Skip" or closing
 * the window never discards anything already confirmed. */
class FirstRunWizard : public QDialog
{
    Q_OBJECT

public:
    explicit FirstRunWizard(QWidget *parent = nullptr);

signals:
    void registerRequested();
    void connectRequested();
    void cardDatabaseUpdateRequested();
    void manualCardDatabaseSetupRequested();

public slots:
    /** @brief Forwarded from MainWindow once the background card database update process exits. */
    void onCardDatabaseUpdateFinished(bool success);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void goNext();
    void goBack();
    void skip();
    void updateChrome();

private:
    void addPage(FirstRunWizardPage *page);
    void showPage(int index);
    void retranslateUi();
    void finish();

    QStackedWidget *stack;
    StepIndicatorWidget *stepIndicator;
    BannerHost *bannerHost;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QPushButton *backButton;
    QPushButton *skipButton;
    QPushButton *nextButton;

    CardDatabaseSetupPage *cardDatabasePage = nullptr;

    QList<FirstRunWizardPage *> pages;
    int currentIndex = -1;
};

#endif // FIRST_RUN_WIZARD_H
