#ifndef COCKATRICE_DLG_CARD_ART_RULES_H
#define COCKATRICE_DLG_CARD_ART_RULES_H

#include "card/card_search_model.h"
#include "tab_supervisor.h"

#include <QAbstractTableModel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>

class AbstractClient;

class CardArtRulesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct Entry
    {
        QString cardName;
        QString mode;
        QString reason;
    };

    explicit CardArtRulesModel(AbstractClient *client, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void refresh();
    void clear();

    QString cardAt(int row) const;

private slots:
    void onRefreshFinished(const Response &r);

private:
    AbstractClient *client;
    std::vector<Entry> entries;
};

class TabCardArtRules : public Tab
{
    Q_OBJECT

public:
    TabCardArtRules(TabSupervisor *parent, AbstractClient *client);

    QString getTabText() const override
    {
        return tr("Card Art Rules");
    }
    void retranslateUi() override;

private:
    void setupUi();

private slots:
    void addRule();
    void removeSelected();
    void refresh();

private:
    AbstractClient *client;

    QLineEdit *searchEdit;
    void initSearchBar();
    QCompleter *searchCompleter;
    CardDatabaseModel *cardDbModel;
    CardDatabaseDisplayModel *cardDbDisplayModel;
    CardSearchModel *cardSearchModel;
    CardCompleterProxyModel *cardProxyModel;
    QComboBox *modeBox;
    QLineEdit *reasonEdit;

    QPushButton *addBtn;
    QPushButton *removeBtn;
    QPushButton *refreshBtn;

    QTableView *table;
    CardArtRulesModel *tableModel;
};

#endif // COCKATRICE_DLG_CARD_ART_RULES_H
