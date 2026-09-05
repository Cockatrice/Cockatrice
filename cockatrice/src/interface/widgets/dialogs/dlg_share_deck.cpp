#include "dlg_share_deck.h"

#include "../cards/additional_info/deck_color_identity.h"
#include "../deck_share/deck_share_utils.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimeZone>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_share_create.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_share_create.pb.h>
#include <libcockatrice/protocol/pending_command.h>

DlgShareDeck::DlgShareDeck(AbstractClient *_client, const QSharedPointer<DeckList> &_deck, QWidget *_parent)
    : QDialog(_parent), client(_client), deck(_deck)
{
    setWindowTitle(tr("Share deck"));

    auto *layout = new QVBoxLayout(this);

    nameEdit = new QLineEdit(this);
    nameEdit->setText(tr("Shared deck"));

    auto *form = new QFormLayout;
    form->addRow(tr("Share name:"), nameEdit);
    layout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Create share link"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgShareDeck::actShare);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgShareDeck::reject);
    this->buttonBox = buttonBox;
    layout->addWidget(buttonBox);
}

void DlgShareDeck::actShare()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    Command_DeckShareCreate cmd;
    cmd.set_name(nameEdit->text().trimmed().toStdString());
    if (cmd.name().empty()) {
        cmd.set_name(tr("Shared deck").toStdString());
    }

    DeckShareItem *item = cmd.add_items();
    item->set_deck_list(deck->writeToString_Native().toStdString());
    item->set_color_identity(getDeckColorIdentity(*deck, CardDatabaseManager::query()).toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DlgShareDeck::shareFinished);
    client->sendCommand(pend);
}

void DlgShareDeck::shareFinished(const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (response.response_code() != Response::RespOk) {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        QMessageBox::critical(this, tr("Share deck"),
                              tr("Failed to create the share link (server response code %1).")
                                  .arg(QString::number(static_cast<int>(response.response_code()))));
        return;
    }

    const Response_DeckShareCreate &resp = response.GetExtension(Response_DeckShareCreate::ext);
    const QString token = QString::fromStdString(resp.token());
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    const QDateTime expiry = QDateTime::fromSecsSinceEpoch(resp.expires_at(), QTimeZone::UTC);
#else
    const QDateTime expiry = QDateTime::fromSecsSinceEpoch(resp.expires_at(), Qt::UTC);
#endif

    const QString link = DeckShareUtils::buildShareLink(client, token);
    DeckShareUtils::copyShareLinkToClipboard(link);

    QMessageBox::information(this, tr("Share deck"),
                             tr("Share link created and copied to the clipboard:\n\n%1\n\n"
                                "The share expires on %2.")
                                 .arg(link, DeckShareUtils::formatShareExpiry(expiry)));
    accept();
}