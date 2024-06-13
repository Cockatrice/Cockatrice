import { AdminCommands } from 'websocket';

export default class AdminService {
    static shutdownServer(reason: string, minutes: number): void {
        AdminCommands.shutdownServer(reason, minutes);
    }

    static updateServerMessage(): void {
        AdminCommands.updateServerMessage();
    }
}