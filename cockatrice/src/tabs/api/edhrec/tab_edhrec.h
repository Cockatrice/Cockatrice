#ifndef TAB_EDHREC_H
#define TAB_EDHREC_H

#include "../../../card/card_info.h"
#include "../../../interface/widgets/general/layout_containers/flow_widget.h"
#include "../../tab.h"
#include "display/commander/edhrec_commander_api_response_display_widget.h"

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
    void setCard(CardInfoPtr _cardToQuery, bool isCommander = false);

private:
    CardInfoPtr cardToQuery;
    EdhrecCommanderApiResponseDisplayWidget *displayWidget;
};

#endif // TAB_EDHREC_H
