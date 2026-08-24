#ifndef CARD_DATABASE_SETUP_PAGE_H
#define CARD_DATABASE_SETUP_PAGE_H

#include "../first_run_wizard_page.h"

#include <QSize>

class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QWidget;

class CardDatabaseSetupPage : public FirstRunWizardPage
{
    Q_OBJECT

public:
    explicit CardDatabaseSetupPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    bool isSkippable() const override;
    QString stepTitle() const override;
    QString stepSubtitle() const override;
    QString nextButtonText() const override;
    bool handleNextClick() override;
    void retranslateUi() override;

    void onUpdateFinished(bool success);

signals:
    void updateRequested();
    void manualSetupRequested();

private:
    enum class State
    {
        NotStarted,
        Running,
        Succeeded,
        Failed,
    };

    void setState(State newState);
    bool alreadyHaveDatabase() const;

    QString oracleSettingsFilePath() const;
    QString readCustomUrl() const;
    void writeCustomUrl(const QString &url);

    void onToggleAdvanced(bool open);
    void onApplyCustomUrl();
    void onRestoreDefaultUrl();

    QLabel *statusLabel;
    QProgressBar *progressBar;
    QPushButton *retryButton;
    QPushButton *manualButton;

    QPushButton *advancedToggleButton;
    QWidget *advancedPanel;
    QLineEdit *urlLineEdit;
    QLabel *urlHintLabel;
    QPushButton *restoreDefaultUrlButton;
    QPushButton *applyAndRetryButton;

    QLabel *startupBehaviorLabel;
    QComboBox *startupBehaviorCombo;
    QLabel *checkIntervalLabel;
    QSpinBox *checkIntervalSpinBox;

    State state = State::NotStarted;
    QSize windowSizeBeforeExpansion;
};

#endif // CARD_DATABASE_SETUP_PAGE_H
