/*
Response_DeckDownload::Response_DeckDownload(int _cmdId, ResponseCode _responseCode, DeckList *_deck)
	: ProtocolResponse(_cmdId, _responseCode, "deck_download")
{
	if (!_deck)
		_deck = new DeckList;
	insertItem(_deck);
}

DeckList *Response_DeckDownload::getDeck() const
{
	return static_cast<DeckList *>(itemMap.value("cockatrice_deck"));
}

Response_DeckUpload::Response_DeckUpload(int _cmdId, ResponseCode _responseCode, DeckList_File *_file)
	: ProtocolResponse(_cmdId, _responseCode, "deck_upload")
{
	if (!_file)
		_file = new DeckList_File;
	insertItem(_file);
}
*/
/*
GameEventContainer::GameEventContainer(const QList<GameEvent *> &_eventList, int _gameId, GameEventContext *_context)
	: ProtocolItem("container", "game_event")
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	
	context = _context;
	if (_context)
		itemList.append(_context);

	eventList = _eventList;
	for (int i = 0; i < _eventList.size(); ++i)
		itemList.append(_eventList[i]);
}

void GameEventContainer::extractData()
{
	for (int i = 0; i < itemList.size(); ++i) {
		GameEvent *_event = dynamic_cast<GameEvent *>(itemList[i]);
		GameEventContext *_context = dynamic_cast<GameEventContext *>(itemList[i]);
		if (_event)
			eventList.append(_event);
		else if (_context)
			context = _context;
	}
}

void GameEventContainer::setContext(GameEventContext *_context)
{
	for (int i = 0; i < itemList.size(); ++i) {
		GameEventContext *temp = qobject_cast<GameEventContext *>(itemList[i]);
		if (temp) {
			delete temp;
			itemList.removeAt(i);
			break;
		}
	}
	itemList.append(_context);
	context = _context;
}

void GameEventContainer::addGameEvent(GameEvent *event)
{
	appendItem(event);
	eventList.append(event);
}

GameEventContainer *GameEventContainer::makeNew(GameEvent *event, int _gameId)
{
	return new GameEventContainer(QList<GameEvent *>() << event, _gameId);
}
*/
