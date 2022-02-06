const fse = require('fs-extra');
const util = require('util');
const exec = util.promisify(require('child_process').exec);

createEnvFile();

async function createEnvFile() {
  try {
    writeFile({
      REACT_APP_VERSION: await getCommitHash(),
    });
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}

async function getCommitHash() {
  return (await exec('git rev-parse HEAD')).stdout;
}

function writeFile(props) {
  const file = Object.keys(props).reduce((str, prop) => {
    return str + `${prop}=${props[prop]}\n`;
  }, '');

  fse.outputFile('.env', file);
}
