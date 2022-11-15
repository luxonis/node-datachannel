const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

const prebuildsDir = path.join(__dirname, 'prebuilds');
if (!fs.existsSync(prebuildsDir)) {
  console.log(`creating`, prebuildsDir);
  fs.mkdirSync(prebuildsDir);
}

const arch = childProcess.execSync('uname -m', { encoding: 'utf-8' }).trim();
console.log(`detected architecture`, arch);

fs.renameSync(
  path.join(__dirname, 'build/Release/node_datachannel.node'),
  path.join(__dirname, `prebuilds/node_datachannel.${arch}.node`),
);
console.log(`prebuild relocated`);
