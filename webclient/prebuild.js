const fse = require('fs-extra');
const util = require('util');
const exec = util.promisify(require('child_process').exec);

const protoFilesDir = './public/pb';
const serverPropsFile = './src/server-props.json';
const masterProtoFile = './src/proto-files.json';

const sharedFiles = [
  ['../common/pb', protoFilesDir],
  ['../cockatrice/resources/countries', './src/images/countries'],
];


(async () => {
  // make sure these files finish copying before master file is created
  await copySharedFiles();

  createMasterProtoFile();
  createServerProps();
})();

async function copySharedFiles() {
  try {
    return await Promise.all(sharedFiles.map(([src, dest]) => fse.copy(src, dest, { overwrite: true })));
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}

function createMasterProtoFile() {
  try {
    fse.readdir(protoFilesDir, (err, files) => {
      if (err) throw err;

      fse.outputFile(masterProtoFile, JSON.stringify(files.filter(file => /\.proto$/.test(file))));
    });
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}

async function createServerProps() {
  try {
    fse.outputFile(serverPropsFile, JSON.stringify({
      REACT_APP_VERSION: await getCommitHash(),
    }));
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}

async function getCommitHash() {
  return (await exec('git rev-parse HEAD')).stdout.trim();
}
