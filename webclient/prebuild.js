const fse = require('fs-extra');
const path = require('path');
const util = require('util');
const exec = util.promisify(require('child_process').exec);

const ROOT_DIR = './src';
const PUBLIC_DIR = './public';

const protoFilesDir = `${PUBLIC_DIR}/pb`;
const i18nDefaultFile = `${ROOT_DIR}/i18n-default.json`;
const serverPropsFile = `${ROOT_DIR}/server-props.json`;
const masterProtoFile = `${ROOT_DIR}/proto-files.json`;

const sharedFiles = [
  ['../libcockatrice_protocol/libcockatrice/protocol/pb', protoFilesDir],
  ['../cockatrice/resources/countries', `${ROOT_DIR}/images/countries`],
];

const i18nFileRegex = /\.i18n\.json$/;

const i18nOnly = process.argv.indexOf('-i18nOnly') > -1;

(async () => {
  if (i18nOnly) {
    await createI18NDefault();
    return;
  }

  // make sure these files finish copying before master file is created
  await copySharedFiles();

  await createMasterProtoFile();
  await createServerProps();
  await createI18NDefault();
})();

async function copySharedFiles() {
  try {
    return await Promise.all(sharedFiles.map(([src, dest]) => fse.copy(src, dest, { overwrite: true })));
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}

async function createMasterProtoFile() {
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

async function createI18NDefault() {
  try {
    const files = getAllFiles(ROOT_DIR, i18nFileRegex);
    const allJson = await Promise.all(files.map(file => fse.readJson(file)));

    const rollup = allJson.reduce((acc, json) => {
      const newKeys = Object.keys(json);

      newKeys.forEach(key => {
        if (acc[key]) {
          throw new Error(`i18n key collision: ${key}\n${JSON.stringify(json)}`);
        }

        acc[key] = json[key];
      });

      return acc;
    }, {});

    fse.outputFile(i18nDefaultFile, JSON.stringify(rollup, null, 2));
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}

async function getCommitHash() {
  return (await exec('git rev-parse HEAD')).stdout.trim();
}

function getAllFiles(dirPath, regex = /./, allFiles = []) {
  return fse.readdirSync(dirPath).reduce((files, file) => {
    const filePath = dirPath + "/" + file;

    if (fse.statSync(filePath).isDirectory()) {
      files.concat(getAllFiles(filePath, regex, files));
    } else if (regex.test(file)) {
      files.push(path.join(__dirname, filePath));
    }

    return files;
  }, allFiles);
}
