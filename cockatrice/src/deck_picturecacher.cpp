#include <QProgressDialog>
#include "deck_picturecacher.h"
#include "decklist.h"
#include "carddatabase.h"
#include "main.h"

void Deck_PictureCacher::cacheHelper(InnerDecklistNode *item, QProgressDialog *progress)
{
	for (int i = 0; i < item->size(); i++) {
		DecklistCardNode *node = dynamic_cast<DecklistCardNode *>(item->at(i));
		if (node) {
			db->getCard(node->getName())->loadPixmap();
			progress->setValue(progress->value() + 1);
		} else
			cacheHelper(dynamic_cast<InnerDecklistNode *>(item->at(i)), progress);
	}
}

void Deck_PictureCacher::cachePictures(DeckList *deck, QWidget *parent)
{
	int totalCards = deck->getRoot()->recursiveCount();

	QProgressDialog progress(tr("Caching card pictures..."), QString(), 0, totalCards, parent);
	progress.setMinimumDuration(1000);
	progress.setWindowModality(Qt::WindowModal);

	cacheHelper(deck->getRoot(), &progress);
}
 