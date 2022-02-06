const fse = require('fs-extra');

copySharedFiles();

function copySharedFiles() {
  try {
    fse.copy('../common/pb', './public/pb', { overwrite: true });
    fse.copy('../cockatrice/resources/countries', './src/images/countries', { overwrite: true });
  } catch (e) {
    console.error(e);
    process.exitCode = 1;
  }
}
