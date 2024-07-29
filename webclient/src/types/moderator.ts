export interface BanHistoryItem {
    adminId: string;
    adminName: string;
    banTime: string;
    banLength: string;
    banReason: string;
    visibleReason: string;
}

export interface WarnHistoryItem {
    userName: string;
    adminName: string;
    reason: string;
    timeOf: string;
}

export interface WarnListItem {
    warning: string;
    userName: string;
    userClientid: string;
}
