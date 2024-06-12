import { ServerDispatch } from 'store';
import { Log } from 'types';

import NormalizeService from '../utils/NormalizeService';

export class ModeratorPersistence {
    static banFromServer(userName: string) {
        // TODO
    }

    static viewLogs(logs: Log[]) {
        ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
    }
}