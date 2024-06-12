import { ServerDispatch } from 'store';
import { Log } from 'types';

import NormalizeService from '../utils/NormalizeService';

export class ModeratorPersistence {
    static banFromServer(userName: any) {
        console.log(userName);
    }

    static banHistory(banHistory: any) {
        console.log(banHistory);
    }

    static viewLogs(logs: Log[]) {
        ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
    }
}