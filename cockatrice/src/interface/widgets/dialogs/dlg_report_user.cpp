#include "dlg_report_user.h"

#include "abstract_client.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <libcockatrice/protocol/pb/command_report.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/report_categories.h>

DlgReportUser::DlgReportUser(AbstractClient *_client,
                             const QString &_reportedUser,
                             int _gameId,
                             const QString &_autoChatLog,
                             QWidget *parent)
    : QDialog(parent), client(_client), reportedUser(_reportedUser), gameId(_gameId)
{
    setWindowTitle(tr("Report User"));
    setMinimumWidth(500);

    auto *infoLabel =
        new QLabel(tr("Reports are reviewed by moderators. False reports may result in account penalties."));
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: palette(placeholderText); padding: 5px;");

    auto *reportGroup = new QGroupBox(tr("Report Details"));
    auto *reportGrid = new QGridLayout(reportGroup);

    reportGrid->addWidget(new QLabel(tr("Reported User:")), 0, 0);
    reportedUserLabel = new QLabel(reportedUser);
    reportedUserLabel->setStyleSheet("font-weight: bold;");
    reportGrid->addWidget(reportedUserLabel, 0, 1);

    reportGrid->addWidget(new QLabel(tr("Game ID:")), 1, 0);
    if (gameId >= 0) {
        gameIdLabel = new QLabel(QString::number(gameId));
        gameIdLabel->setStyleSheet("font-weight: bold;");
        reportGrid->addWidget(gameIdLabel, 1, 1);
    } else {
        gameIdEdit = new QLineEdit;
        gameIdEdit->setPlaceholderText(tr("(Optional) Enter game ID if available"));
        gameIdEdit->setToolTip(tr("If the report is related to a specific game, enter its ID."));
        reportGrid->addWidget(gameIdEdit, 1, 1);
        gameIdLabel = nullptr;
    }

    auto *categoryGroup = new QGroupBox(tr("Category"));
    auto *categoryGrid = new QGridLayout(categoryGroup);

    categoryBox = new QComboBox;
    for (const QString &key : ReportCategories::keys()) {
        const QString label = categoryLabel(key);
        if (label.isEmpty()) {
            continue; // skip keys without a dialog label
        }
        categoryBox->addItem(label, key);
        categoryBox->setItemData(categoryBox->count() - 1, categoryToolTip(key), Qt::ToolTipRole);
    }

    categoryGrid->addWidget(new QLabel(tr("Category:")), 0, 0);
    categoryGrid->addWidget(categoryBox, 0, 1);

    auto *descGroup = new QGroupBox(tr("Description"));
    auto *descLayout = new QVBoxLayout(descGroup);

    descriptionEdit = new QTextEdit;
    descriptionEdit->setPlaceholderText(
        tr("Please describe what happened. Include dates, game details, or any evidence if available."));
    descriptionEdit->setFixedHeight(120);
    descLayout->addWidget(descriptionEdit);

    auto *chatGroup = new QGroupBox(tr("Chat Log Context"));
    auto *chatLayout = new QVBoxLayout(chatGroup);
    chatLogEdit = new QTextEdit;
    chatLogEdit->setReadOnly(true);
    QFont monoFont("monospace");
    monoFont.setStyleHint(QFont::Monospace);
    const int systemPointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    if (systemPointSize > 0) {
        monoFont.setPointSize(systemPointSize);
    }
    chatLogEdit->setFont(monoFont);
    if (!_autoChatLog.isEmpty()) {
        chatLogEdit->setPlainText(_autoChatLog);
    } else {
        chatLogEdit->setPlaceholderText(tr("No chat context available (not triggered from chat)."));
    }
    chatLogEdit->setFixedHeight(100);
    chatLayout->addWidget(chatLogEdit);

    auto *chatNote = new QLabel(
        tr("This chat log is captured from your local chat window and may not reflect the full conversation."));
    chatNote->setWordWrap(true);
    chatNote->setStyleSheet("color: palette(placeholderText);");
    chatLayout->addWidget(chatNote);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Submit Report"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgReportUser::actSubmit);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(infoLabel);
    layout->addWidget(reportGroup);
    layout->addWidget(categoryGroup);
    layout->addWidget(descGroup);
    layout->addWidget(chatGroup);
    layout->addWidget(buttonBox);
}

void DlgReportUser::actSubmit()
{
    const QString description = descriptionEdit->toPlainText().trimmed();
    if (description.isEmpty()) {
        QMessageBox::warning(this, tr("Missing description"), tr("Please describe what happened before submitting."));
        return;
    }

    QMessageBox::StandardButton reply =
        QMessageBox::question(this, tr("Confirm Report"),
                              tr("Submit report against %1 for %2?").arg(reportedUser, categoryBox->currentText()),
                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    buttonBox->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Submitting..."));

    Command_Report cmd;
    cmd.set_reported_user(reportedUser.toStdString());
    cmd.set_category(categoryBox->currentData().toString().toStdString());
    cmd.set_description(description.toStdString());

    if (gameId >= 0) {
        cmd.set_game_id(gameId);
    } else if (gameIdEdit && !gameIdEdit->text().trimmed().isEmpty()) {
        bool ok;
        int manualGameId = gameIdEdit->text().trimmed().toInt(&ok);
        if (ok && manualGameId > 0) {
            cmd.set_game_id(manualGameId);
        }
    }

    const QString chatLog = chatLogEdit->toPlainText().trimmed();
    if (!chatLog.isEmpty()) {
        cmd.set_chat_log(chatLog.toStdString());
    }

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgReportUser::reportResponse);
    client->sendCommand(pend);
}

void DlgReportUser::reportResponse(const Response &response)
{
    buttonBox->setEnabled(true);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Submit Report"));

    if (response.response_code() == Response::RespOk) {
        QMessageBox::information(this, tr("Report Submitted"),
                                 tr("Your report has been submitted and will be reviewed by a moderator. Thank you."));
        accept();
    } else if (response.response_code() == Response::RespTooManyRequests) {
        QMessageBox::warning(this, tr("Submission Failed"),
                             tr("You have reached the daily report limit. Please try again later."));
    } else if (response.response_code() == Response::RespNameNotFound) {
        QMessageBox::warning(
            this, tr("Submission Failed"),
            tr("The reported user could not be found. Guests (unregistered users) cannot be reported."));
    } else {
        QMessageBox::warning(this, tr("Submission Failed"), tr("Failed to submit report. Please try again."));
    }
}

QString DlgReportUser::categoryLabel(const QString &key)
{
    if (key == QLatin1String("cheating")) {
        return tr("Cheating / Unsporting behavior");
    }
    if (key == QLatin1String("bug_abuse")) {
        return tr("Bug Abuse");
    }
    if (key == QLatin1String("harassment")) {
        return tr("Harassment / Abuse");
    }
    if (key == QLatin1String("verbal_abuse")) {
        return tr("Verbal Abuse");
    }
    if (key == QLatin1String("hate_speech")) {
        return tr("Hate speech");
    }
    if (key == QLatin1String("spam")) {
        return tr("Spam");
    }
    if (key == QLatin1String("other")) {
        return tr("Other");
    }
    return QString();
}

QString DlgReportUser::categoryToolTip(const QString &key)
{
    if (key == QLatin1String("cheating")) {
        return tr("Using external tools, cheat programs, or exploiting game bugs");
    }
    if (key == QLatin1String("bug_abuse")) {
        return tr("Exploiting a bug or glitch to gain an unfair advantage");
    }
    if (key == QLatin1String("harassment")) {
        return tr("Threatening, bullying, or persistent unwanted contact");
    }
    if (key == QLatin1String("verbal_abuse")) {
        return tr("Abusive or offensive language directed at another player");
    }
    if (key == QLatin1String("hate_speech")) {
        return tr("Discriminatory language targeting race, gender, religion, etc.");
    }
    if (key == QLatin1String("spam")) {
        return tr("Repeated unwanted messages or advertisements");
    }
    if (key == QLatin1String("other")) {
        return tr("Any behavior not covered by the above categories");
    }
    return QString();
}
