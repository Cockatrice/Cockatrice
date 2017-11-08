#ifndef DLG_CREATETOKEN_H
#define DLG_CREATETOKEN_H

#include <QDialog>
#include <QModelIndex>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QRadioButton;
class DeckList;
class CardDatabaseModel;
class TokenDisplayModel;

class DlgCreateToken : public QDialog {
    Q_OBJECT
public:
    DlgCreateToken(const QStringList &_predefinedTokens, QWidget *parent = 0);
    QString getName() const;
    QString getColor() const;
    QString getPT() const;
    QString getAnnotation() const;
    bool getDestroy() const;
private slots:
    void tokenSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void updateSearch(const QString &search);
    void actChooseTokenFromAll(bool checked);
    void actChooseTokenFromDeck(bool checked);
    void actOk();
private:
    CardDatabaseModel *cardDatabaseModel;
    TokenDisplayModel *cardDatabaseDisplayModel;
    QStringList predefinedTokens;
    QLabel *nameLabel, *colorLabel, *ptLabel, *annotationLabel;
    QComboBox *colorEdit;
    QLineEdit *nameEdit, *ptEdit, *annotationEdit;
    QCheckBox *destroyCheckBox;
    QRadioButton *chooseTokenFromAllRadioButton, *chooseTokenFromDeckRadioButton;

    void updateSearchFieldWithoutUpdatingFilter(const QString &newValue) const;
};

#endif
