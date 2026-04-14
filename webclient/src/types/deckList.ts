import type {
  ServerInfo_DeckStorage_File, ServerInfo_DeckStorage_Folder, ServerInfo_DeckStorage_TreeItem
} from 'generated/proto/serverinfo_deckstorage_pb';
import type { Response_DeckList } from 'generated/proto/response_deck_list_pb';

export type DeckList = Response_DeckList;
export type DeckStorageFolder = ServerInfo_DeckStorage_Folder;
export type DeckStorageFile = ServerInfo_DeckStorage_File;
export type DeckStorageTreeItem = ServerInfo_DeckStorage_TreeItem;
