#ifndef COCKATRICE_URL_PARSER_H
#define COCKATRICE_URL_PARSER_H
#include <QObject>
#include <QUrlQuery>

class MainWindow;
struct ContextJoinGame;
class IntentUrlParser : public QObject
{
    Q_OBJECT

public:
    IntentUrlParser(QObject *parent, MainWindow *mainWindow);
    void handle(const QString &urlStr);
    void handleJoinGame(const QUrlQuery &query);

private:
    QString generateJoinGameMessage(const ContextJoinGame &context, const QString &gameDescription);

    MainWindow *mainWindow;
};

#endif // COCKATRICE_URL_PARSER_H
