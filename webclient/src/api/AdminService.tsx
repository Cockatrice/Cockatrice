import { AdminCommands } from 'websocket';

export default class AdminService {
    static reloadConfig(): void {
        AdminCommands.reloadConfig();
    }

    static shutdownServer(reason: string, minutes: number): void {
        AdminCommands.shutdownServer(reason, minutes);
    }

    static updateServerMessage(): void {
        AdminCommands.updateServerMessage();
    }
}