// eslint-disable-next-line @typescript-eslint/no-var-requires
const nodeDataChannel = require('../node_datachannel.node');
module.exports = nodeDataChannel;

module.exports.DataChannelStream = require('./datachannel-stream');
