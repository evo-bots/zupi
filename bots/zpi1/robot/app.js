'use strict';

const nomnom = require('nomnom');
const log = require('debug')('zpi1.robot');
const Robot = require('./lib/robot.js');
const Builder = require('./lib/five.js');

var robot = new Robot(new Builder());

function quit(code) {
    robot.reset();
    setTimeout(() => process.exit(code), 1000);
}

robot
    .on('connected', () => log('connected'))
    .on('error', (err) => {
        console.error(err);
        quit(1);
    })
    .on('disconnected', () => {
        log('disconnected');
        quit(0);
    });

function runRobot(fn) {
    return (opts) => robot.start((err) => {
        if (err) {
            console.error(err);
            process.exit(1);
        } else {
            fn(opts);
        }
    });
}

nomnom.script('zpi1-robot');
nomnom.command('reset')
    .callback(runRobot(() => robot.reset()))
    .help('Reset Robot');
nomnom.command('connect')
    .option('host', {
        position: 1,
        metavar: 'HOST',
        type: 'string',
        help: 'Host of brain',
        required: true
    })
    .option('port', {
        abbr: 'p',
        metavar: 'PORT',
        default: 6565,
        help: 'Port of brain'
    })
    .callback(runRobot((opts) => robot.connect(opts.host, opts.port)))
    .help('Connect Brain');

nomnom.parse();
