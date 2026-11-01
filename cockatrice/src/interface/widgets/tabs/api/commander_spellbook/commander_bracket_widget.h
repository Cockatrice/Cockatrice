#ifndef COCKATRICE_COMMANDER_BRACKET_WIDGET_H
#define COCKATRICE_COMMANDER_BRACKET_WIDGET_H

#include "commander_bracket_service.h"
#include "commander_spellbook_bracket_explainer.h"

#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <QtGlobal>
#include <qtmetamacros.h>

class QLabel;
class QToolButton;
class QTimer;
struct CommanderBracketEstimate;
class DeckList;
class QObject;
template <class T> class QSharedPointer;

class CommanderBracketWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommanderBracketWidget(QWidget *parent = nullptr);
    ~CommanderBracketWidget() override;

    void setDeck(const QSharedPointer<DeckList> &_deck);
    void retranslateUi();

private slots:
    void requestBracketEstimate();
    void onEstimateBracketFinished(quint64 id, QObject *requester, const CommanderBracketEstimate &result);
    void onEstimateBracketError(quint64 id, QObject *requester, const QString &error);
    void maybeAutoEstimateBracket();

private:
    bool promptCommanderSpellbookIntegration();
    void updateBracketVisibility(bool visible);
    void cancelEstimate();
    void displayEstimate();

    QSharedPointer<DeckList> deck;
    bool prompting = false;
    quint64 requestId = 0;
    QTimer *autoEstimateTimer;
    bool hasEstimate = false;
    CommanderBracketEstimate lastEstimate;

    QLabel *bracketLabel;
    QLabel *bracketValueLabel;
    QToolButton *bracketInfoButton;
    QToolButton *bracketRefreshButton;

    BracketExplanation lastBracketExplanation;
};

#endif // COCKATRICE_COMMANDER_BRACKET_WIDGET_H
