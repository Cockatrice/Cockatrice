import React from "react";
import * as _ from "lodash";

import AppBar from "@material-ui/core/AppBar";
import Box from "@material-ui/core/Box";
import Paper from "@material-ui/core/Paper";
import Table from "@material-ui/core/Table";
import TableBody from "@material-ui/core/TableBody";
import TableCell from "@material-ui/core/TableCell";
import TableHead from "@material-ui/core/TableHead";
import TableRow from "@material-ui/core/TableRow";
import Tab from "@material-ui/core/Tab";
import Tabs from "@material-ui/core/Tabs";
import Typography from "@material-ui/core/Typography";

import "./LogResults.css";

const LogResults = (props) => {
  const { logs } = props;

  const hasRoomLogs = logs.room && logs.room.length;
  const hasGameLogs = logs.game && logs.game.length;
  const hasChatLogs = logs.chat && logs.chat.length;
  
  const [value, setValue] = React.useState(0);

  const handleChange = (event, newValue) => {
    setValue(newValue);
  };

  const headerCells = [
    {
      label: "Time"
    },
    {
      label: "Sender Name"
    },
    {
      label: "Sender IP"
    },
    {
      label: "Message"
    },
    {
      label: "Target ID"
    },
    {
      label: "Target Name"
    }
  ];
  
  return (
    <div>
      <AppBar position="static">
        <Tabs value={value} onChange={handleChange} aria-label="simple tabs example">
          <Tab label={"Rooms" + (hasRoomLogs ? ` [${logs.room.length}]` : "")} {...a11yProps(0)} />
          <Tab label={"Games" + (hasGameLogs ? ` [${logs.game.length}]` : "")} {...a11yProps(1)} />
          <Tab label={"Chats" + (hasChatLogs ? ` [${logs.chat.length}]` : "")} {...a11yProps(2)} />
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
  )
};

const a11yProps = index => {
  return {
    id: `simple-tab-${index}`,
    "aria-controls": `simple-tabpanel-${index}`,
  };
};

const TabPanel = ({ children, value, index, ...other }) => {
  return (
    <Typography
      component="div"
      role="tabpanel"
      hidden={value !== index}
      id={`simple-tabpanel-${index}`}
      aria-labelledby={`simple-tab-${index}`}
      {...other}
    >
      <Box>{children}</Box>
    </Typography>
  );
};

const Results = ({headerCells, logs}) => (
  <Paper className="log-results">
    <Table size="small">
      <TableHead>
        <TableRow>
          { _.map(headerCells, ({ label }) => (
            <TableCell key={label}>{label}</TableCell>
          ))}
        </TableRow>
      </TableHead>
      <TableBody>
        { _.map(logs, ({ time, senderName, senderIp, message, targetId, targetName }, index) => (
          <TableRow key={index}>
            <TableCell>{time}</TableCell>
            <TableCell>{senderName}</TableCell>
            <TableCell>{senderIp}</TableCell>
            <TableCell>{message}</TableCell>
            <TableCell>{targetId}</TableCell>
            <TableCell>{targetName}</TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  </Paper>
);

export default LogResults;