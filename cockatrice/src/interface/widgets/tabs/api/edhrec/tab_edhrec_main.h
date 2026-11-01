/**
 * @file tab_edhrec_main.h
 * @ingroup Tabs
 */
//! \todo Document this file.

#ifndef TAB_EDHREC_MAIN_H
#define TAB_EDHREC_MAIN_H

#include "../../tab.h"
#include "libcockatrice/card/card_info.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <qtmetamacros.h>

class CardSizeWidget;
class EdhrecCommanderApiResponseDisplayWidget;
class QHBoxLayout;
class QLineEdit;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QVBoxLayout;
class QWidget;
class SettingsButtonWidget;
class TabSupervisor;

class TabEdhRecMain : public Tab
{
    Q_OBJECT
public:
    explicit TabEdhRecMain(TabSupervisor *_tabSupervisor);
    ~TabEdhRecMain() override;

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
    enum class RequestKind
    {
        TopCards,
        TopCommanders,
        TopTags,
        Details,
        AverageDeck
    };

    void startRequest(QUrl url, RequestKind kind);
    void cancelRequest();
    void showRequestError(const QString &error, bool canRetry = true);
    void clearPage();
    void clearCardSelection();

    QPointer<QNetworkReply> activeReply;
    QUrl requestUrl;
    RequestKind requestKind = RequestKind::TopCards;
    QLabel *requestStatusLabel;
    QPushButton *retryButton;
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
};

#endif // TAB_EDHREC_MAIN_H
