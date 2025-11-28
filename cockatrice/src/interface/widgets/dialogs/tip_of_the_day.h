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
    [[nodiscard]] QString getTitle() const
    {
        return title;
    }
    [[nodiscard]] QString getContent() const
    {
        return content;
    }
    [[nodiscard]] QString getImagePath() const
    {
        return imagePath;
    }
    [[nodiscard]] QDate getDate() const
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
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<TipOfTheDay> *tipList;
};

#endif
