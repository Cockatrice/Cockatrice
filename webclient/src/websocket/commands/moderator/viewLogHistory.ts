import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';
import { LogFilters } from 'types';

export function viewLogHistory(filters: LogFilters): void {
  BackendService.sendModeratorCommand('Command_ViewLogHistory', filters, {
    responseName: 'Response_ViewLogHistory',
    onSuccess: (response) => {
      ModeratorPersistence.viewLogs(response.logMessage);
    },
  });
}
