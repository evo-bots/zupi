'use strict';

const net = require('net');
const EventEmitter = require('events');
const tbus = require('tbus');
const ID = require('./devices.js').ID;
const RtspCameraDev = require('../gen/zupi/camera_tbusdev.js').RtspCameraDev;

class Robot extends EventEmitter {
    constructor(builder) {
        super();
        this._builder = builder;
    }

    start(cb) {
        this._builder.build((err, logics) => {
            if (err == null) {
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
            }
            cb.call(this);
        });
        return this;
    }

    reset() {
        this._logics.reset();
        return this;
    }

    connect(host, port) {
        if (!port) {
            port = 6565;
        }
        this._port = new tbus.RemoteBusPort(this._busDev,
            tbus.SocketConnector(() => net.connect(port, host)));
        this._port
            .on('connected', () => {
                if (this._logics.indicator) {
                    this._logics.indicator.on(true);
                }
                // HACK: wait tbus to expose connection
                this._logics.connected(this._port._connection);
                this.emit('connected');
            })
            .on('error', (err) => {
                this.reset();
                this.emit('error', err);
            })
            .on('disconnected', () => {
                this.reset();
                this.emit('disconnected');
            });
        this.reset();
        this._port.connect();
        if (this._logics.indicator) {
            this._logics.indicator.flash(300);
        }
        return this;
    }
}

module.exports = Robot;
