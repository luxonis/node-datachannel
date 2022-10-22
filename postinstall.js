/* eslint-disable @typescript-eslint/no-var-requires */
const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

if (fs.existsSync(path.resolve('./prebuilds'))) {
  const arch = childProcess.execSync('uname -m').toString('utf-8').trim();
  if (fs.existsSync(path.resolve('./node_datachannel.node'))) {
    fs.unlinkSync(path.resolve('./node_datachannel.node'));
  }
  fs.linkSync(path.resolve(`./prebuilds/node_datachannel.${arch}.node`), path.resolve('./node_datachannel.node'));
}
