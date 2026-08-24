#ifndef DLG_TOURNAMENT_SETTINGS_H
#define DLG_TOURNAMENT_SETTINGS_H

#include <QDialog>

class QDialogButtonBox;
class QSpinBox;

struct DlgTournamentSettingsResult
{
    int gamesPerMatch = 1;
};

class DlgTournamentSettings : public QDialog
{
    Q_OBJECT
public:
    explicit DlgTournamentSettings(QWidget *parent = nullptr);
    DlgTournamentSettingsResult getResult() const;
    void setCurrentGamesPerMatch(int n);

private slots:
    void actOK();

private:
    QSpinBox *gamesPerMatchSpin;
    QDialogButtonBox *buttonBox;
};

#endif
