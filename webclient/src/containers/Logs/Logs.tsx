import { AuthGuard, ModGuard } from '@app/components';
import { SearchForm } from '@app/forms';

import LogResults from './LogResults';
import { useLogs } from './useLogs';

import './Logs.css';

const Logs = () => {
  const { logs, onSubmit } = useLogs();

  return (
    <div className="moderator-logs overflow-scroll">
      <AuthGuard />
      <ModGuard />

      <div className="moderator-logs__form">
        <SearchForm onSubmit={onSubmit} />
      </div>

      <div className="moderator-logs__results">
        <LogResults logs={logs} />
      </div>
    </div>
  );
};

export default Logs;
