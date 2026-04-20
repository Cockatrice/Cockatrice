import { useEffect, useState } from 'react';

import { LoadingState, useKnownHosts, useReduxEffect, useWebClient } from '@app/hooks';
import { getHostPort, HostDTO } from '@app/services';
import { ServerTypes } from '@app/store';
import { App } from '@app/types';

export enum TestConnection {
  TESTING = 'testing',
  FAILED = 'failed',
  SUCCESS = 'success',
}

export interface KnownHostsComponent {
  hosts: App.Host[];
  selectedHost: App.Host | undefined;
  testingConnection: TestConnection | null;
  dialogState: { open: boolean; edit: HostDTO | null };
  showCreateToast: boolean;
  showDeleteToast: boolean;
  showEditToast: boolean;
  setShowCreateToast: (v: boolean) => void;
  setShowDeleteToast: (v: boolean) => void;
  setShowEditToast: (v: boolean) => void;
  onPick: (host: HostDTO) => Promise<void>;
  openAddKnownHostDialog: () => void;
  openEditKnownHostDialog: (host: HostDTO) => void;
  closeKnownHostDialog: () => void;
  handleDialogRemove: (args: { id: number }) => Promise<void>;
  handleDialogSubmit: (args: {
    id?: number;
    name: string;
    host: string;
    port: string;
  }) => Promise<void>;
}

export interface UseKnownHostsComponentArgs {
  onChange: (value: HostDTO) => void;
}

export function useKnownHostsComponent({
  onChange,
}: UseKnownHostsComponentArgs): KnownHostsComponent {
  const webClient = useWebClient();
  const knownHosts = useKnownHosts();

  const [dialogState, setDialogState] = useState<{ open: boolean; edit: HostDTO | null }>({
    open: false,
    edit: null,
  });

  const [testingConnection, setTestingConnection] = useState<TestConnection | null>(null);

  const [showCreateToast, setShowCreateToast] = useState(false);
  const [showDeleteToast, setShowDeleteToast] = useState(false);
  const [showEditToast, setShowEditToast] = useState(false);

  const selectedHost =
    knownHosts.status === LoadingState.READY ? knownHosts.value?.selectedHost : undefined;
  const hosts = knownHosts.status === LoadingState.READY ? knownHosts.value?.hosts ?? [] : [];

  const testConnection = (host: HostDTO) => {
    setTestingConnection(TestConnection.TESTING);
    webClient.request.authentication.testConnection({ ...getHostPort(host) });
  };

  useEffect(() => {
    if (!selectedHost) {
      return;
    }
    onChange(selectedHost);
    testConnection(selectedHost);
  }, [selectedHost]);

  useReduxEffect(
    () => {
      setTestingConnection(TestConnection.SUCCESS);
    },
    ServerTypes.TEST_CONNECTION_SUCCESSFUL,
    [],
  );

  useReduxEffect(
    () => {
      setTestingConnection(TestConnection.FAILED);
    },
    ServerTypes.TEST_CONNECTION_FAILED,
    [],
  );

  const onPick = async (host: HostDTO) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }
    onChange(host);
    await knownHosts.select(host.id!);
    testConnection(host);
  };

  const openAddKnownHostDialog = () => {
    setDialogState((s) => ({ ...s, open: true, edit: null }));
  };

  const openEditKnownHostDialog = (host: HostDTO) => {
    setDialogState((s) => ({ ...s, open: true, edit: host }));
  };

  const closeKnownHostDialog = () => {
    setDialogState((s) => ({ ...s, open: false }));
  };

  const handleDialogRemove = async ({ id }: { id: number }) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }
    await knownHosts.remove(id);
    closeKnownHostDialog();
    setShowDeleteToast(true);
  };

  const handleDialogSubmit = async ({
    id,
    name,
    host,
    port,
  }: {
    id?: number;
    name: string;
    host: string;
    port: string;
  }) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }

    if (id) {
      await knownHosts.update(id, { name, host, port });
      setShowEditToast(true);
    } else {
      const newHost: App.Host = { name, host, port, editable: true };
      await knownHosts.add(newHost);
      setShowCreateToast(true);
    }

    closeKnownHostDialog();
  };

  return {
    hosts,
    selectedHost,
    testingConnection,
    dialogState,
    showCreateToast,
    showDeleteToast,
    showEditToast,
    setShowCreateToast,
    setShowDeleteToast,
    setShowEditToast,
    onPick,
    openAddKnownHostDialog,
    openEditKnownHostDialog,
    closeKnownHostDialog,
    handleDialogRemove,
    handleDialogSubmit,
  };
}
