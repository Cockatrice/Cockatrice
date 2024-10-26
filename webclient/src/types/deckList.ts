export interface DeckList {
    root: DeckStorageFolder;
}

export interface DeckStorageFolder {
    items: DeckStorageTreeItem[];
}

export interface DeckStorageFile {
    creationTime: number;
}

export interface DeckStorageTreeItem {
    id: number;
    name: string;
    file: DeckStorageFile;
    folder: DeckStorageFolder;
}
