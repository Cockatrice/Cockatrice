import { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';

import { useToast } from '@app/components';
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
  onPick: (id: number) => Promise<void>;
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

type ToastMode = 'created' | 'deleted' | 'edited';

export function useKnownHostsComponent({
  onChange,
}: UseKnownHostsComponentArgs): KnownHostsComponent {
  const webClient = useWebClient();
  const knownHosts = useKnownHosts();
  const { t } = useTranslation();

  const [toastMode, setToastMode] = useState<ToastMode>('created');
  const knownHostToast = useToast({
    key: 'known-hosts-action',
    children: t('KnownHosts.toast', { mode: toastMode }),
  });

  const [dialogState, setDialogState] = useState<{ open: boolean; edit: HostDTO | null }>({
    open: false,
    edit: null,
  });

  const [testingConnection, setTestingConnection] = useState<TestConnection | null>(null);
  // Tracks the host currently awaiting a testConnection response. If null when a
  // response arrives, the caller has moved on — ignore the stale reply.
  const pendingTestRef = useRef<HostDTO | null>(null);

  const selectedHost =
    knownHosts.status === LoadingState.READY ? knownHosts.value?.selectedHost : undefined;
  const hosts = knownHosts.status === LoadingState.READY ? knownHosts.value?.hosts ?? [] : [];

  const testConnection = (host: HostDTO) => {
    pendingTestRef.current = host;
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

  useReduxEffect(() => {
    if (!pendingTestRef.current) {
      return;
    }
    setTestingConnection(TestConnection.SUCCESS);
    pendingTestRef.current = null;
  }, ServerTypes.TEST_CONNECTION_SUCCESSFUL, []);

  useReduxEffect(() => {
    if (!pendingTestRef.current) {
      return;
    }
    setTestingConnection(TestConnection.FAILED);
    pendingTestRef.current = null;
  }, ServerTypes.TEST_CONNECTION_FAILED, []);

  const fireToast = (mode: ToastMode) => {
    setToastMode(mode);
    knownHostToast.openToast();
  };

  const onPick = async (id: number) => {
    if (knownHosts.status !== LoadingState.READY) {
      return;
    }
    const host = knownHosts.value?.hosts.find((h) => h.id === id);
    if (!host) {
      return;
    }
    onChange(host);
    await knownHosts.select(id);
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
    fireToast('deleted');
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
      fireToast('edited');
    } else {
      const newHost: App.Host = { name, host, port, editable: true };
      await knownHosts.add(newHost);
      fireToast('created');
    }

    closeKnownHostDialog();
  };

  return {
    hosts,
    selectedHost,
    testingConnection,
    dialogState,
    onPick,
    openAddKnownHostDialog,
    openEditKnownHostDialog,
    closeKnownHostDialog,
    handleDialogRemove,
    handleDialogSubmit,
  };
}
