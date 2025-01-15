#ifndef TAB_EDHREC_H
#define TAB_EDHREC_H

#include "../../../../game/cards/card_database.h"
#include "../../../ui/widgets/general/layout_containers/flow_widget.h"
#include "../../tab.h"

#include <QHBoxLayout>
#include <QNetworkAccessManager>

class TabEdhRec : public Tab
{
    Q_OBJECT
public:
    explicit TabEdhRec(TabSupervisor *_tabSupervisor);

    void retranslateUi() override;
    QString getTabText() const override
    {
        auto cardName = cardToQuery.isNull() ? QString() : cardToQuery->getName();
        return tr("EDHREC: ") + cardName;
    }

    QNetworkAccessManager *networkManager;

public slots:
    void processApiJson(QNetworkReply *reply);
    void prettyPrintJson(const QJsonValue &value, int indentLevel);
    void setCard(CardInfoPtr _cardToQuery);

private:
    QWidget *container;
    FlowWidget *flowWidget;
    QHBoxLayout *layout;
    CardInfoPtr cardToQuery;
};

#endif // TAB_EDHREC_H
