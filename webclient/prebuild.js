const fse = require('fs-extra');
const util = require('util');
const exec = util.promisify(require('child_process').exec);

const sharedFiles = [
  ['../common/pb', './public/pb'],
  ['../cockatrice/resources/countries', './src/images/countries'],
];

const serverPropsFile = './src/server-props.json';

copySharedFiles();
createServerProps();

function copySharedFiles() {
  runCommand(() => {
    sharedFiles.forEach(([src, dest]) => {
      fse.copy(src, dest, { overwrite: true });
    });
  });
}

function createServerProps() {
  runCommand(async () => {
    fse.outputFile(serverPropsFile, JSON.stringify({
      REACT_APP_VERSION: await getCommitHash(),
    }));
  });
}

async function getCommitHash() {
  return (await exec('git rev-parse HEAD')).stdout.trim();
}

function runCommand(command) {
  try {
    command();
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}