#ifndef ALT_TABLE_DELEGATE_H
#define ALT_TABLE_DELEGATE_H

#include <QStyledItemDelegate>
#include <qtmetamacros.h>

/**
 * Badge + icon painting for the User Alts table. The match-evidence column
 * renders as a palette-tinted pill per match type (email / client id / IP), and
 * an IP shared by more than one other account is dimmed. Warn/ban counts gain
 * the style's warning/critical icons when nonzero. Selected rows and unknown
 * content fall back to default painting.
 */
class AltTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AltTableDelegate(int viaColumn,
                              int warnsColumn,
                              int bansColumn,
                              int sharedColumn,
                              QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int viaColumn;
    int warnsColumn;
    int bansColumn;
    int sharedColumn;
};

#endif // ALT_TABLE_DELEGATE_H
