#ifndef COCKATRICE_URL_PARSER_H
#define COCKATRICE_URL_PARSER_H
#include <QObject>
#include <qt5/QtCore/qurlquery.h>

class MainWindow;
class IntentUrlParser : public QObject
{
    Q_OBJECT

public:
    IntentUrlParser(QObject *parent, MainWindow *mainWindow);
    void handle(const QString &urlStr);
    void handleJoinGame(const QUrlQuery &query);

    void parse(QString url);

private:
    MainWindow *mainWindow;
};

#endif // COCKATRICE_URL_PARSER_H
