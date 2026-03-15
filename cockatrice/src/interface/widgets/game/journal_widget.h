#ifndef COCKATRICE_JOURNAL_WIDGET_H
#define COCKATRICE_JOURNAL_WIDGET_H
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <qplaintextedit.h>

/**
 * The widget that is used to show the journal restore prompt.
 */
class JournalRestoreWidget : public QWidget
{
    Q_OBJECT

    QLabel *textLabel;
    QPushButton *restoreButton;
    QPushButton *discardButton;

public:
    explicit JournalRestoreWidget(QWidget *parent = nullptr);

    void retranslateUi();

signals:
    void restorePushed();
    void discardPushed();
};

/**
 * The widget used in the Journal dock in the game tab.
 */
class JournalWidget : public QWidget
{
    Q_OBJECT

    QPlainTextEdit *textEdit;
    JournalRestoreWidget *restoreWidget;
    QTimer debounceTimer;

public:
    explicit JournalWidget(QWidget *parent = nullptr);

private slots:
    void showRestorePrompt(bool show);
    void backupText();
    void restoreText();
};

#endif // COCKATRICE_JOURNAL_WIDGET_H
