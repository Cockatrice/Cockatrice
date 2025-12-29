#ifndef COCKATRICE_EDHREC_COMMANDER_API_RESPONSE_BRACKET_NAVIGATION_WIDGET_H
#define COCKATRICE_EDHREC_COMMANDER_API_RESPONSE_BRACKET_NAVIGATION_WIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QWidget>

class EdhrecCommanderApiResponseBracketNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderApiResponseBracketNavigationWidget(QWidget *parent, const QString &baseUrl);
    void retranslateUi();
    void applyOptionsFromUrl(const QString &url);
    QString getSelectedGameChanger() const
    {
        return selectedGameChanger;
    }

signals:
    void requestNavigation();

private:
    QGridLayout *layout;
    QLabel *gameChangerLabel;

    QStringList gameChangerOptions = {"", "exhibition", "core", "upgraded", "optimized", "cedh"};
    QString selectedGameChanger;

    QMap<QString, QPushButton *> gameChangerButtons;

    void updateOptionButtonSelection(QMap<QString, QPushButton *> &buttons, const QString &selectedKey);
};

#endif // COCKATRICE_EDHREC_COMMANDER_API_RESPONSE_BRACKET_NAVIGATION_WIDGET_H
