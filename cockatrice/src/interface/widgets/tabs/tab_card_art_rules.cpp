#include "tab_card_art_rules.h"

#include "libcockatrice/card/database/card_database_manager.h"

#include <QCompleter>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
#include <libcockatrice/protocol/pb/response_card_art_rule_entry.pb.h>
#include <libcockatrice/protocol/pending_command.h>

CardArtRulesModel::CardArtRulesModel(AbstractClient *client, QObject *parent)
    : QAbstractTableModel(parent), client(client)
{
}

int CardArtRulesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(entries.size());
}

int CardArtRulesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant CardArtRulesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &e = entries.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return e.cardName;
            case 1:
                return e.mode;
            case 2:
                return e.reason;
        }
    }

    return {};
}

QVariant CardArtRulesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case 0:
            return tr("Card");
        case 1:
            return tr("Mode");
        case 2:
            return tr("Reason");
        default:
            return {};
    }
}

void CardArtRulesModel::refresh()
{
    Command_ListCardArtRules cmd;

    PendingCommand *pend = client->prepareModeratorCommand(cmd);

    connect(pend, &PendingCommand::finished, this, &CardArtRulesModel::onRefreshFinished);

    client->sendCommand(pend);
}

void CardArtRulesModel::clear()
{
    beginResetModel();
    entries.clear();
    endResetModel();
}

QString CardArtRulesModel::cardAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(entries.size())) {
        return {};
    }

    return entries[row].cardName;
}

void CardArtRulesModel::onRefreshFinished(const Response &r)
{
    if (r.response_code() != Response::RespOk) {
        return;
    }

    const auto &resp = r.GetExtension(Response_ListCardArtRules::ext);

    beginResetModel();
    entries.clear();

    for (const auto &e : resp.entries()) {
        entries.push_back({QString::fromStdString(e.card_name()), QString::fromStdString(e.mode()),
                           QString::fromStdString(e.reason())});
    }

    endResetModel();
}

TabCardArtRules::TabCardArtRules(TabSupervisor *parent, AbstractClient *_client) : Tab(parent), client(_client)
{
    setupUi();
    refresh();
}

void TabCardArtRules::setupUi()
{
    auto *central = new QWidget(this);

    initSearchBar();

    modeBox = new QComboBox;
    reasonEdit = new QLineEdit;

    addBtn = new QPushButton;
    removeBtn = new QPushButton;
    refreshBtn = new QPushButton;

    modeBox->addItems({"ALLOW", "DENY"});

    tableModel = new CardArtRulesModel(client, this);

    table = new QTableView;
    table->setModel(tableModel);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *form = new QFormLayout;
    form->addRow(tr("Card:"), searchEdit);
    form->addRow(tr("Mode:"), modeBox);
    form->addRow(tr("Reason:"), reasonEdit);

    auto *buttons = new QHBoxLayout;
    buttons->addWidget(addBtn);
    buttons->addWidget(removeBtn);
    buttons->addWidget(refreshBtn);

    auto *layout = new QVBoxLayout;
    layout->addLayout(form);
    layout->addLayout(buttons);
    layout->addWidget(table);

    central->setLayout(layout);
    setCentralWidget(central);

    connect(addBtn, &QPushButton::clicked, this, &TabCardArtRules::addRule);

    connect(removeBtn, &QPushButton::clicked, this, &TabCardArtRules::removeSelected);

    connect(refreshBtn, &QPushButton::clicked, this, &TabCardArtRules::refresh);

    retranslateUi();
}

void TabCardArtRules::initSearchBar()
{
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(tr("Type a card name..."));

    cardDbModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
    cardDbDisplayModel = new CardDatabaseDisplayModel(this);
    cardDbDisplayModel->setSourceModel(cardDbModel);
    cardSearchModel = new CardSearchModel(cardDbDisplayModel, this);

    cardProxyModel = new CardCompleterProxyModel(this);
    cardProxyModel->setSourceModel(cardSearchModel);
    cardProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    searchCompleter = new QCompleter(cardProxyModel, this);
    searchCompleter->setCompletionRole(Qt::DisplayRole);
    searchCompleter->setCompletionMode(QCompleter::PopupCompletion);
    searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    searchCompleter->setFilterMode(Qt::MatchContains);
    searchCompleter->setMaxVisibleItems(15);
    searchEdit->setCompleter(searchCompleter);

    connect(searchEdit, &QLineEdit::textEdited, cardSearchModel, &CardSearchModel::updateSearchResults);
    connect(searchEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        const QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        cardProxyModel->setFilterRegularExpression(
            QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        if (!text.isEmpty()) {
            searchCompleter->complete();
        }
    });
    connect(searchCompleter, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated), this,
            [this](const QString &name) { searchEdit->setText(name); });
}

void TabCardArtRules::retranslateUi()
{
    addBtn->setText(tr("Add rule"));
    removeBtn->setText(tr("Remove rule"));
    refreshBtn->setText(tr("Refresh"));
}

void TabCardArtRules::refresh()
{
    tableModel->refresh();
}

void TabCardArtRules::addRule()
{
    Command_AddCardArtRule cmd;
    cmd.set_card_name(searchEdit->text().toStdString());
    cmd.set_mode(modeBox->currentText().toStdString());
    cmd.set_reason(reasonEdit->text().toStdString());

    client->sendCommand(client->prepareModeratorCommand(cmd));

    refresh();
}

void TabCardArtRules::removeSelected()
{
    QModelIndex idx = table->currentIndex();
    if (!idx.isValid()) {
        return;
    }

    Command_RemoveCardArtRule cmd;
    cmd.set_card_name(tableModel->cardAt(idx.row()).toStdString());

    client->sendCommand(client->prepareModeratorCommand(cmd));

    refresh();
}