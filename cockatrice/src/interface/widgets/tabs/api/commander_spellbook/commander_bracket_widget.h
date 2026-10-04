#ifndef COCKATRICE_COMMANDER_BRACKET_WIDGET_H
#define COCKATRICE_COMMANDER_BRACKET_WIDGET_H

#include "commander_spellbook_bracket_explainer.h"

#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <qtmetamacros.h>
#include <qtypes.h>

class QLabel;
class QToolButton;
struct CommanderBracketEstimate;
class DeckList;
class QObject;
template <class T> class QSharedPointer;

class CommanderBracketWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CommanderBracketWidget(QWidget *parent = nullptr);

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

    QSharedPointer<DeckList> deck;
    bool prompting = false;
    quint64 requestId = 0;

    QLabel *bracketLabel;
    QLabel *bracketValueLabel;
    QToolButton *bracketInfoButton;
    QToolButton *bracketRefreshButton;

    BracketExplanation lastBracketExplanation;
};

#endif // COCKATRICE_COMMANDER_BRACKET_WIDGET_H
