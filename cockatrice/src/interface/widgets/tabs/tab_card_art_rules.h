#ifndef COCKATRICE_DLG_CARD_ART_RULES_H
#define COCKATRICE_DLG_CARD_ART_RULES_H

#include "tab.h"

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <qnamespace.h>
#include <qtmetamacros.h>
#include <vector>

class AbstractClient;
class CardCompleterProxyModel;
class CardDatabaseDisplayModel;
class CardDatabaseModel;
class CardSearchModel;
class QComboBox;
class QCompleter;
class QLineEdit;
class QObject;
class QPushButton;
class QTableView;
class Response;
class TabSupervisor;

class CardArtRulesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct Entry
    {
        QString cardName;
        QString cardProviderId;
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
    const Entry *entryAt(int row) const;

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
    void populateProviderCombo(const QString &cardName);
    QCompleter *searchCompleter;
    CardDatabaseModel *cardDbModel;
    CardDatabaseDisplayModel *cardDbDisplayModel;
    CardSearchModel *cardSearchModel;
    CardCompleterProxyModel *cardProxyModel;
    QComboBox *providerComboBox;
    QComboBox *modeBox;
    QLineEdit *reasonEdit;

    QPushButton *addBtn;
    QPushButton *removeBtn;
    QPushButton *refreshBtn;

    QTableView *table;
    CardArtRulesModel *tableModel;
};

#endif // COCKATRICE_DLG_CARD_ART_RULES_H
