import { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';

import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Paper from '@mui/material/Paper';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';
import Tab from '@mui/material/Tab';
import Tabs from '@mui/material/Tabs';

import type { Data } from '@app/types';
import type { ServerStateLogs } from '@app/store';

import { useLogResults } from './useLogResults';

import './LogResults.css';

interface LogResultsProps {
  logs: ServerStateLogs;
}

interface HeaderCell {
  label: string;
}

interface ResultsProps {
  headerCells: HeaderCell[];
  logs: Data.ServerInfo_ChatMessage[];
}

interface TabPanelProps {
  children?: ReactNode;
  value: number;
  index: number;
}

const a11yProps = (index: number): { id: string; 'aria-controls': string } => ({
  id: `logs-tab-${index}`,
  'aria-controls': `logs-tabpanel-${index}`,
});

const TabPanel = ({ children, value, index }: TabPanelProps) => (
  <div
    role="tabpanel"
    hidden={value !== index}
    id={`logs-tabpanel-${index}`}
    aria-labelledby={`logs-tab-${index}`}
  >
    <Box>{children}</Box>
  </div>
);

const Results = ({ headerCells, logs }: ResultsProps) => (
  <Paper className="log-results">
    <Table size="small">
      <TableHead>
        <TableRow>
          {headerCells.map(({ label }) => (
            <TableCell key={label}>{label}</TableCell>
          ))}
        </TableRow>
      </TableHead>
      <TableBody>
        {logs.map((log, index) => (
          <TableRow key={`${log.time}-${log.senderIp}-${index}`}>
            <TableCell>{log.time}</TableCell>
            <TableCell>{log.senderName}</TableCell>
            <TableCell>{log.senderIp}</TableCell>
            <TableCell>{log.message}</TableCell>
            <TableCell>{log.targetId}</TableCell>
            <TableCell>{log.targetName}</TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  </Paper>
);

const LogResults = ({ logs }: LogResultsProps) => {
  const { t } = useTranslation();
  const { value, handleChange } = useLogResults();

  const headerCells: HeaderCell[] = [
    { label: t('Logs.column.time') },
    { label: t('Logs.column.senderName') },
    { label: t('Logs.column.senderIp') },
    { label: t('Logs.column.message') },
    { label: t('Logs.column.targetId') },
    { label: t('Logs.column.targetName') },
  ];

  const roomCount = logs.room?.length ?? 0;
  const gameCount = logs.game?.length ?? 0;
  const chatCount = logs.chat?.length ?? 0;

  return (
    <div>
      <AppBar position="static">
        <Tabs value={value} onChange={handleChange} aria-label={t('Logs.title', { defaultValue: 'Log Results' })}>
          <Tab label={`${t('Logs.tab.rooms')}${roomCount > 0 ? ` [${roomCount}]` : ''}`} {...a11yProps(0)} />
          <Tab label={`${t('Logs.tab.games')}${gameCount > 0 ? ` [${gameCount}]` : ''}`} {...a11yProps(1)} />
          <Tab label={`${t('Logs.tab.chats')}${chatCount > 0 ? ` [${chatCount}]` : ''}`} {...a11yProps(2)} />
        </Tabs>
      </AppBar>
      <TabPanel value={value} index={0}>
        <Results logs={logs.room} headerCells={headerCells} />
      </TabPanel>
      <TabPanel value={value} index={1}>
        <Results logs={logs.game} headerCells={headerCells} />
      </TabPanel>
      <TabPanel value={value} index={2}>
        <Results logs={logs.chat} headerCells={headerCells} />
      </TabPanel>
    </div>
  );
};

export default LogResults;
