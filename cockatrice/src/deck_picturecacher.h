#ifndef DECK_PICTURECACHER_H
#define DECK_PICTURECACHER_H

#include <QObject>

class InnerDecklistNode;
class QProgressDialog;
class DeckList;
class QWidget;

class Deck_PictureCacher : public QObject {
	Q_OBJECT
private:
	static void cacheHelper(InnerDecklistNode *item, QProgressDialog *progress);
public:
	static void cachePictures(DeckList *deck, QWidget *parent);
};

#endif
