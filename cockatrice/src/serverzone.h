#ifndef SERVERZONE_H
#define SERVERZONE_H

#include <QString>

class ServerZone {
private:
	QString name;
	bool isPublic;
	bool hasCoords;
	int cardCount;
public:
	ServerZone(const QString &_name, bool _isPublic, bool _hasCoords, int _cardCount)
		: name(_name), isPublic(_isPublic), hasCoords(_hasCoords), cardCount(_cardCount) { }
	QString getName() const { return name; }
	bool getPublic() const { return isPublic; }
	bool getHasCoords() const { return hasCoords; }
	int getCardCount() const { return cardCount; }
};

#endif
