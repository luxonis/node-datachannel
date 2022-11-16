const fs = require('fs');
const path = require('path');
const glob = require('glob');

const allPrebuildsPath = path.join(__dirname, 'all-prebuilds');
const matches = glob.sync(`${allPrebuildsPath}/**/*.node`);

fs.mkdirSync('prebuilds');
matches.forEach((file) => {
  fs.renameSync(file, path.join(__dirname, 'prebuilds', path.basename(file)));
});
