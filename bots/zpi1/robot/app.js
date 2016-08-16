'use strict';

const net = require('net');
const debug = require('debug');
const five  = require('johnny-five');
const tbus  = require('tbus');
const logic = require('tbus-five');

const BrainConnector = require('./lib/brain-connector.js');

const NAME = 'zpi1';

const ID_LED = 0x01;
const ID_MOTOR_L = 0x10;
const ID_MOTOR_R = 0x11;
const ID_SERVO_P = 0x20;
const ID_SERVO_T = 0x21;

class App {
    constructor() {
        this.log = debug(NAME);
        this._board = new five.Board({
            port: '/dev/ttyAMA0',
            repl: false,
            debug: false
        });
        this._board
            .on('ready', () => this._start())
            .on('error', (err) => this._boardError(err));
    }

    _start() {
        this.log('assembling board');
        this._bus = new tbus.Bus();
        this._bus
            .plug(
                new tbus.LEDDev(
                    new logic.LED(
                        new five.Led(13)
                    ),
                    { id: ID_LED }
                )
            )
            .plug(
                new tbus.MotorDev(
                    new logic.Motor(
                        new five.Motor(five.Motor.SHIELD_CONFIGS.POLOLU_DRV8835_SHIELD.M1)
                    ),
                    { id: ID_MOTOR_L }
                )
            )
            .plug(
                new tbus.MotorDev(
                    new logic.Motor(
                        new five.Motor(five.Motor.SHIELD_CONFIGS.POLOLU_DRV8835_SHIELD.M2)
                    ),
                    { id: ID_MOTOR_R }
                )
            )
            .plug(
                new tbus.ServoDev(
                    new logic.Servo(
                        new five.Servo({controller: 'PCA9685', pin: 0})
                    ),
                    { id: ID_SERVO_P }
                )
            )
            .plug(
                new tbus.ServoDev(
                    new logic.Servo(
                        new five.Servo({controller: 'PCA9685', pin: 1})
                    ),
                    { id: ID_SERVO_T }
                )
            );
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
            if (err != null) {
                console.error(err);
            }
        });
    }

    _boardError(err) {
        console.error(err.message);
        process.exit(1);
    }
}

new App();
