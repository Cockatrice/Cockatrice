/**
 * @file tab_edhrec_main.h
 * @ingroup Tabs
 * @brief TODO: Document this.
 */

#ifndef TAB_EDHREC_MAIN_H
#define TAB_EDHREC_MAIN_H

#include "../../tab.h"

#include <libcockatrice/card/card_info.h>

class CardSizeWidget;
class QNetworkAccessManager;
class QNetworkReply;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLineEdit;
class SettingsButtonWidget;
class EdhrecCommanderApiResponseDisplayWidget;
class TabEdhRecMain : public Tab
{
    Q_OBJECT
public:
    explicit TabEdhRecMain(TabSupervisor *_tabSupervisor);

    void retranslateUi() override;
    void doSearch();
    [[nodiscard]] QString getTabText() const override
    {
        auto cardName = cardToQuery.isNull() ? QString() : cardToQuery->getName();
        return tr("EDHRec: ") + cardName;
    }

    CardSizeWidget *getCardSizeSlider() const
    {
        return cardSizeSlider;
    }

    QNetworkAccessManager *networkManager;

public slots:
    void processApiJson(QNetworkReply *reply);
    void processCommanderResponse(QJsonObject reply, QString responseUrl = "");
    void processTopCardsResponse(QJsonObject reply);
    void processTopTagsResponse(QJsonObject reply);
    void processTopCommandersResponse(QJsonObject reply);
    void processAverageDeckResponse(QJsonObject reply);
    void prettyPrintJson(const QJsonValue &value, int indentLevel);
    void setCard(CardInfoPtr _cardToQuery, bool isCommander = false);
    void actNavigatePage(QString url);
    void getTopCards();
    void getTopCommanders();
    void getTopTags();

private:
    QWidget *container;
    QWidget *navigationContainer;
    QWidget *currentPageDisplay;
    QVBoxLayout *mainLayout;
    QHBoxLayout *navigationLayout;
    QVBoxLayout *currentPageLayout;
    QPushButton *cardsPushButton;
    QPushButton *topCommandersPushButton;
    QPushButton *tagsPushButton;
    QLineEdit *searchBar;
    QPushButton *searchPushButton;
    SettingsButtonWidget *settingsButton;
    CardSizeWidget *cardSizeSlider;
    CardInfoPtr cardToQuery;
    EdhrecCommanderApiResponseDisplayWidget *displayWidget;
};

#endif // TAB_EDHREC_MAIN_H
