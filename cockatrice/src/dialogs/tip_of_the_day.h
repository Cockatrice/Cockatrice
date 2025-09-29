/**
 * @file tip_of_the_day.h
 * @ingroup Dialogs
 * @brief TODO: Document this.
 */

#ifndef TIP_OF_DAY_H
#define TIP_OF_DAY_H

#include <QAbstractListModel>
#include <QDate>

class TipOfTheDay
{
public:
    explicit TipOfTheDay(QString _title, QString _content, QString _imagePath, QDate _date);
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
    QDate getDate() const
    {
        return date;
    }

private:
    QString title, content, imagePath;
    QDate date;
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
        DateColumn,
    };

    explicit TipsOfTheDay(QString xmlPath, QObject *parent = nullptr);
    ~TipsOfTheDay() override;
    TipOfTheDay getTip(int tipId);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<TipOfTheDay> *tipList;
};

#endif
