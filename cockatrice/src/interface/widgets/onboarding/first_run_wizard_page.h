#ifndef FIRST_RUN_WIZARD_PAGE_H
#define FIRST_RUN_WIZARD_PAGE_H

#include <QWidget>

/** @brief Base class for a single step of FirstRunWizard.
 *
 * QWidget-based rather than QWizardPage-based: FirstRunWizard is a
 * QDialog + QStackedWidget shell (not a QWizard) so it can own the
 * banner/step-dot chrome that QWizard's native styles don't give us
 * consistent control over. Naming mirrors OracleWizardPage for
 * familiarity only -- the two hierarchies are unrelated. */
class FirstRunWizardPage : public QWidget
{
    Q_OBJECT

public:
    explicit FirstRunWizardPage(QWidget *parent = nullptr) : QWidget(parent)
    {
    }

    /** @brief Called every time the page becomes visible, including navigating back to it. */
    virtual void initializePage()
    {
    }

    /** @brief Called before advancing past this page. Return false to block navigation;
        the page itself is responsible for telling the user why. */
    virtual bool validatePage()
    {
        return true;
    }

    /** @brief Whether Next/Finish should currently be enabled. Pages doing async work
        can flip this mid-step; emit completeChanged() when they do. */
    virtual bool isComplete() const
    {
        return true;
    }

    /** @brief Whether the wizard's "Skip" button should be offered on this page. */
    virtual bool isSkippable() const
    {
        return false;
    }

    virtual QString stepTitle() const = 0;
    virtual QString stepSubtitle() const
    {
        return {};
    }

    /** @brief Override to replace the "Next"/"Finish" button text on this page.
        Return an empty string to use the default label. */
    virtual QString nextButtonText() const
    {
        return {};
    }

    /** @brief Called when the user presses the Next button. Return true to allow
        advancing to the next page, false to stay on this page (e.g. to
        trigger an async action first). */
    virtual bool handleNextClick()
    {
        return true;
    }

    virtual void retranslateUi() = 0;

signals:
    void completeChanged();
    void advanceRequested();
};

#endif // FIRST_RUN_WIZARD_PAGE_H
