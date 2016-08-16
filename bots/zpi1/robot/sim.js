'use strict';

const net = require('net');
const debug = require('debug');
const tbus = require('tbus');

const BrainConnector = require('./lib/brain-connector.js');

const NAME = 'zpi1sim';

const ID_LED = 0x01;
const ID_MOTOR_L = 0x10;
const ID_MOTOR_R = 0x11;
const ID_SERVO_P = 0x20;
const ID_SERVO_T = 0x21;

class SimDevBase {
    constructor(name) {
        this.log = debug(name);
    }

    setDevice(dev) {
        this.device = dev;
        return this;
    }
}

class LED extends SimDevBase {
    constructor() {
        super("led");
    }

    setPowerState(params, done) {
        this.on = params.getOn();
        this.log("on %j", this.on);
        done();
    }
}

class Motor extends SimDevBase {
    constructor(suffix) {
        super("motor" + suffix);
    }

    start(params, done) {
        this.direction = params.getDirection() ? 'rev' : 'fwd';
        this.speed = params.getSpeed();
        this.log("start: dir %s speed %j", this.direction, this.speed);
        done();
    }

    stop(done) {
        this.speed = 0;
        this.log("stop");
        done();
    }

    brake(params, done) {
        this.brakeOn = params.getOn();
        this.log("brake %j", this.brakeOn);
        done();
    }
}

class Servo extends SimDevBase {
    constructor(suffix) {
        super("servo" + suffix);
    }

    setPosition(params, done) {
        this.angle = params.getAngle();
        this.log("angle %j", this.angle);
        done();
    }

    stop(done) {
        this.log("stop");
        done();
    }
}

class App {
    constructor() {
        this.log = debug(NAME);
        this._bus = new tbus.Bus();
        this._bus
            .plug(new tbus.LEDDev(new LED(), { id: ID_LED }))
            .plug(new tbus.MotorDev(new Motor("L"), { id: ID_MOTOR_L }))
            .plug(new tbus.MotorDev(new Motor("R"), { id: ID_MOTOR_R }))
            .plug(new tbus.ServoDev(new Servo("P"), { id: ID_SERVO_P }))
            .plug(new tbus.ServoDev(new Servo("T"), { id: ID_SERVO_T }));
        this._busDev = new tbus.BusDev(this._bus);
        this._port = new tbus.RemoteBusPort(this._busDev,
            tbus.SocketConnector(() => net.connect(this._offer.port, this._offer.host)));
        this._brainConn = new BrainConnector(NAME);
        this._port
            .on('connected', () => this.log("connected"))
            .on('error', (err) => {
                this.log("connect error %s", err.message);
                this._brainConn.connect((err) => {
                    if (err) {
                        console.error(err);
                    }
                });
            });
        this._brainConn.on('offer', (offer) => {
            this.log("offer %s:%d", offer.host, offer.port);
            this._offer = offer;
            this._port.connect();
            this._brainConn.stop();
        });
        this._brainConn.connect((err) => {
            if (err) {
                console.error(err);
            }
        });
    }
}

new App();
