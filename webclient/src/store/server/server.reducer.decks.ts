import { CaseReducer, PayloadAction } from '@reduxjs/toolkit';
import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { ServerState } from './server.interfaces';

function splitPath(path: string): string[] {
  return path ? path.split('/') : [];
}

function insertAtPath(
  folder: Data.ServerInfo_DeckStorage_Folder,
  pathSegments: string[],
  item: Data.ServerInfo_DeckStorage_TreeItem,
): Data.ServerInfo_DeckStorage_Folder {
  if (pathSegments.length === 0 || (pathSegments.length === 1 && pathSegments[0] === '')) {
    return create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [...folder.items, item] });
  }
  const [head, ...tail] = pathSegments;
  const match = folder.items.find(child => child.name === head && child.folder);
  if (match) {
    return create(Data.ServerInfo_DeckStorage_FolderSchema, {
      items: folder.items.map(child =>
        child === match
          ? { ...child, folder: insertAtPath(child.folder!, tail, item) }
          : child
      ),
    });
  }
  const created: Data.ServerInfo_DeckStorage_TreeItem = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
    id: 0, name: head, folder: insertAtPath(create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] }), tail, item)
  });
  return create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [...folder.items, created] });
}

function removeById(folder: Data.ServerInfo_DeckStorage_Folder, id: number): Data.ServerInfo_DeckStorage_Folder {
  return create(Data.ServerInfo_DeckStorage_FolderSchema, {
    items: folder.items
      .filter(item => item.id !== id)
      .map(item =>
        item.folder ? { ...item, folder: removeById(item.folder, id) } : item
      ),
  });
}

function removeByPath(folder: Data.ServerInfo_DeckStorage_Folder, pathSegments: string[]): Data.ServerInfo_DeckStorage_Folder {
  if (pathSegments.length === 0 || (pathSegments.length === 1 && pathSegments[0] === '')) {
    return folder;
  }
  const [head, ...tail] = pathSegments;
  if (tail.length === 0) {
    return create(Data.ServerInfo_DeckStorage_FolderSchema, {
      items: folder.items.filter(item => !(item.name === head && item.folder != null))
    });
  }
  return create(Data.ServerInfo_DeckStorage_FolderSchema, {
    items: folder.items.map(item =>
      item.name === head && item.folder
        ? { ...item, folder: removeByPath(item.folder, tail) }
        : item
    ),
  });
}

export const deckReducers = {
  backendDecks: ((state, action) => {
    state.backendDecks = action.payload.deckList;
  }) as CaseReducer<ServerState, PayloadAction<{ deckList: Data.Response_DeckList }>>,

  deckUpload: ((state, action) => {
    if (!state.backendDecks?.root) {
      return;
    }
    state.backendDecks = create(Data.Response_DeckListSchema, {
      root: insertAtPath(state.backendDecks.root, splitPath(action.payload.path), action.payload.treeItem),
    });
  }) as CaseReducer<ServerState, PayloadAction<{ path: string; treeItem: Data.ServerInfo_DeckStorage_TreeItem }>>,

  deckDelete: ((state, action) => {
    if (!state.backendDecks?.root) {
      return;
    }
    state.backendDecks = create(Data.Response_DeckListSchema, {
      root: removeById(state.backendDecks.root, action.payload.deckId),
    });
  }) as CaseReducer<ServerState, PayloadAction<{ deckId: number }>>,

  deckNewDir: ((state, action) => {
    if (!state.backendDecks?.root) {
      return;
    }
    const newFolder: Data.ServerInfo_DeckStorage_TreeItem = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: action.payload.dirName, folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    state.backendDecks = create(Data.Response_DeckListSchema, {
      root: insertAtPath(state.backendDecks.root, splitPath(action.payload.path), newFolder),
    });
  }) as CaseReducer<ServerState, PayloadAction<{ path: string; dirName: string }>>,

  deckDelDir: ((state, action) => {
    if (!state.backendDecks?.root) {
      return;
    }
    state.backendDecks = create(Data.Response_DeckListSchema, {
      root: removeByPath(state.backendDecks.root, splitPath(action.payload.path)),
    });
  }) as CaseReducer<ServerState, PayloadAction<{ path: string }>>,

  deckDownloaded: ((state, action) => {
    state.downloadedDeck = action.payload;
  }) as CaseReducer<ServerState, PayloadAction<{ deckId: number; deck: string }>>,
};
