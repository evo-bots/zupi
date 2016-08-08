'use strict';

const net = require('net');

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

var board = new five.Board({
    port: '/dev/ttyAMA0'
});

board.on('ready', () => {
    let bus = new tbus.Bus();

    bus
        .plug(
            new tbus.LEDDev(
                new logic.LED(
                    new five.Led(13),
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

    let bc = new BrainConnector(NAME);
    bc
        .on('offer', (offer) => {
            let sock = net.connect(offer.port, offer.host);
            sock
                .on('connect', () => {
                    bc.stop();
                    sock.pipe(bus.hostStream());
                    bus.hostStream().pipe(sock);
                })
                .on('error', (err) => {
                    console.error(err);
                });
        })
        .connect((err) => {
            if (err != nil) {
                console.error(err);
            }
        });
});
