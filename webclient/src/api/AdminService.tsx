import { AdminCommands } from 'websocket';

export default class AdminService {
    static updateServerMessage(): void {
        AdminCommands.updateServerMessage();
    }
}