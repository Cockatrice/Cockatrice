#ifndef TIP_OF_DAY_H
#define TIP_OF_DAY_H

#include <QAbstractListModel>

class TipOfTheDay : public QObject
{
    Q_OBJECT
public:
    TipOfTheDay(QString _title, QString _content, QString _imagePath);
    TipOfTheDay(const TipOfTheDay &other);
    ~TipOfTheDay();
    QString getTitle() const
    {
        return title;
    }
    QString getContent() const
    {
        return content;
    }
    QString getImagePath() const
    {
        return imagePath;
    }

private:
    QString title;
    QString content;
    QString imagePath;
};

class TipsOfTheDay : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Columns
    {
        TitleColumn,
        ContentColumn,
        ImagePathColumn,
    };
    TipsOfTheDay(QString xmlPath, QObject *parent = 0);
    ~TipsOfTheDay();
    void getTip(int tipId, QString &title, QString &content, QString &imagePath);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    QList<TipOfTheDay> *tipList;
};

#endif
