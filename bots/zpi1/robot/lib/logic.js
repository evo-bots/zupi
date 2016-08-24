'use strict';

const debug = require('debug');

class Logic {
    constructor(name) {
        this.log = debug('zpi1.robot.logic.' + name);
    },

    setDevice(dev) {
        this.device = dev;
    }
}

module.exports = Logic;
