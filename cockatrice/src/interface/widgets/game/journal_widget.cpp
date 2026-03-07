#include "journal_widget.h"

#include "../../../client/settings/cache_settings.h"

#include <QLayout>
#include <QPlainTextEdit>
#include <QPushButton>

static const QString BACKUP_FILE_NAME = "journal.txt";

JournalRestoreWidget::JournalRestoreWidget(QWidget *parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(3);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    textLabel = new QLabel(this);
    restoreButton = new QPushButton(this);
    discardButton = new QPushButton(this);

    layout->addWidget(textLabel);
    layout->addWidget(restoreButton);
    layout->addWidget(discardButton);

    connect(restoreButton, &QPushButton::clicked, this, &JournalRestoreWidget::restorePushed);
    connect(discardButton, &QPushButton::clicked, this, &JournalRestoreWidget::discardPushed);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &JournalRestoreWidget::retranslateUi);

    retranslateUi();
}

void JournalRestoreWidget::retranslateUi()
{
    textLabel->setText(tr("Previous journal text detected."));
    restoreButton->setText(tr("Restore"));
    discardButton->setText(tr("Discard"));
}

static QString getJournalBackupPath()
{
    return SettingsCache::instance().getDataPath() + "/" + BACKUP_FILE_NAME;
}

static bool hasSavedBackupText()
{
    QFileInfo fileInfo = QFileInfo(getJournalBackupPath());
    return fileInfo.size() != 0;
}

JournalWidget::JournalWidget(QWidget *parent) : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    textEdit = new QPlainTextEdit(this);
    restoreWidget = new JournalRestoreWidget(this);

    layout->addWidget(textEdit);
    layout->addWidget(restoreWidget);

    showRestorePrompt(hasSavedBackupText());

    connect(restoreWidget, &JournalRestoreWidget::discardPushed, this, [this] { showRestorePrompt(false); });
    connect(restoreWidget, &JournalRestoreWidget::restorePushed, this, [this] {
        restoreText();
        showRestorePrompt(false);
    });

    // Debounce setup
    debounceTimer.setSingleShot(true);
    connect(&debounceTimer, &QTimer::timeout, this, &JournalWidget::backupText);
    connect(textEdit, &QPlainTextEdit::textChanged, this, [this] { debounceTimer.start(300); });
}

void JournalWidget::showRestorePrompt(bool show)
{
    restoreWidget->setVisible(show);
    textEdit->setVisible(!show);
}

void JournalWidget::backupText()
{
    QFile file(getJournalBackupPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QString text = textEdit->toPlainText();
    file.write(text.toUtf8());
}

void JournalWidget::restoreText()
{
    QFile file(getJournalBackupPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QByteArray byteArray = file.readAll();
    textEdit->setPlainText(QString::fromUtf8(byteArray));
}