import type { ServerInfo_Ban } from 'generated/proto/serverinfo_ban_pb';
import type { ServerInfo_Warning } from 'generated/proto/serverinfo_warning_pb';
import type { Response_WarnList } from 'generated/proto/response_warn_list_pb';

export type BanHistoryItem = ServerInfo_Ban;
export type WarnHistoryItem = ServerInfo_Warning;
export type WarnListItem = Response_WarnList;
