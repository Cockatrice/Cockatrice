#ifndef ALT_TABLE_DELEGATE_H
#define ALT_TABLE_DELEGATE_H

#include <QStyledItemDelegate>
#include <qtmetamacros.h>

/**
 * Badge + icon painting for the User Alts table. The match-evidence column
 * renders as a palette-tinted pill per match type (email / client id / IP),
 * with the style's warning icon once an IP is shared by more than the VPN
 * threshold. Warn/ban counts gain the style's warning/critical icons when
 * nonzero. The active column renders as a palette-tinted pill (Highlight
 * when active, PlaceholderText when inactive, from Qt::UserRole). Selected
 * rows and unknown content fall back to default painting.
 */
class AltTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AltTableDelegate(int viaColumn,
                              int warnsColumn,
                              int bansColumn,
                              int sharedColumn,
                              int activeColumn,
                              QObject *parent = nullptr);
    void setVpnThreshold(int threshold);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int viaColumn;
    int warnsColumn;
    int bansColumn;
    int sharedColumn;
    int activeColumn;
    int vpnThreshold = 10;
};

#endif // ALT_TABLE_DELEGATE_H
