'use strict';

const net = require('net');
const tbus = require('tbus');
const log = require('debug')('zpi1.robot.body');
const RtspCameraDev = require('../gen/zupi/camera_tbusdev.js').RtspCameraDev;
const BrainConnector = require('./brain-connector.js');
const ID = require('./devices.js').ID;
const NAME = require('./devices.js').NAME;

const IndicatorSlow = 1000;
const IndicatorFast = 300;

class Body {
    constructor(builder) {
        this._builder = builder;

        if (process.argv.length > 2) {
            let remote = process.argv[2].split(':');
            this._offer = { host: remote[0] };
            if (remote.length == 1) {
                this._offer.port = BrainConnector.PORT;
            } else {
                this._offer.port = parseInt(remote[1]);
                if (this._offer.port == NaN) {
                    console.error("invalid port " + remote[1]);
                    process.exit(1);
                }
            }
        } else {
            this._brainConn = new BrainConnector(NAME);
        }
    }

    run() {
        this._builder.build((err, logics) => {
            if (err) {
                this._startErr(err);
            } else {
                this._start(logics);
            }
        });
    }

    _start(logics) {
        this._logics = logics;
        this._bus = new tbus.Bus();
        this._bus
            .plug(new tbus.LEDDev(logics.led, { id: ID.LED }))
            .plug(new tbus.MotorDev(logics.motors.left, { id: ID.MOTOR_L }))
            .plug(new tbus.MotorDev(logics.motors.right, { id: ID.MOTOR_R }))
            .plug(new tbus.ServoDev(logics.servos.pan, { id: ID.SERVO_P }))
            .plug(new tbus.ServoDev(logics.servos.tilt, { id: ID.SERVO_T }))
            .plug(new RtspCameraDev(logics.camera, { id: ID.CAMERA }));
        this._busDev = new tbus.BusDev(this._bus);
        this._port = new tbus.RemoteBusPort(this._busDev,
            tbus.SocketConnector(() => net.connect(this._offer.port, this._offer.host)));
        this._port
            .on('connected', () => {
                log("connected");
                this._indicatorOn();
                // HACK: wait tbus to expose connection
                this._logics.connected(this._port._connection);
            })
            .on('error', (err) => {
                log("connect error %s", err.message);
                this._reset(err);
            })
            .on('disconnected', () => {
                this._reset();
            });
        if (this._brainConn) {
            this._brainConn.on('offer', (offer) => {
                log("offer %s:%d", offer.host, offer.port);
                this._offer = offer;
                this._port.connect();
                this._brainConn.stop();
                this._indicatorFlash(IndicatorFast);
            });
            this._reset();
        } else {
            log("connect %s:%d", this._offer.host, this._offer.port);
            this._logics.reset();
            this._port.connect();
            this._indicatorFlash(IndicatorFast);
        }
    }

    _startErr(err) {
        console.error(err);
        process.exit(1);
    }

    _connectBrain() {
        this._brainConn.connect((err) => {
            if (err) {
                console.error(err);
            }
        });
        this._indicatorFlash(IndicatorSlow);
    }

    _reset(err) {
        if (this._brainConn) {
            this._connectBrain();
            this._logics.reset();
        } else {
            log("terminate");
            process.exit(err ? 1 : 0);
        }
    }

    _indicatorFlash(delay) {
        if (this._logics.indicator) {
            this._logics.indicator.flash(delay);
        }
    }

    _indicatorOn() {
        if (this._logics.indicator) {
            this._logics.indicator.on(true);
        }
    }
}

module.exports = Body;
