'use strict';

const five = require('johnny-five');
const tbus = require('tbus-five');
const RtspCamera = require('./camera.js');

class Builder {
    build(done) {
        this._board = new five.Board({
            port: process.env.SERIAL_PORT || '/dev/ttyAMA0',
            repl: false,
            debug: false
        });
        this._board
            .on('ready', () => this._start(done))
            .on('error', done);
    }

    _start(done) {
        this._led = new five.Led(13);
        let logics = {
            led: new tbus.LED(this._led),
            motors: {
                left: new tbus.Motor(new five.Motor(five.Motor.SHIELD_CONFIGS.POLOLU_DRV8835_SHIELD.M1)),
                right: new tbus.Motor(new five.Motor(five.Motor.SHIELD_CONFIGS.POLOLU_DRV8835_SHIELD.M2))
            },
            servos: {
                pan: new tbus.Servo(new five.Servo({controller: 'PCA9685', pin: 0, offset: -25, center: true})),
                tilt: new tbus.Servo(new five.Servo({controller: 'PCA9685', pin: 1, offset: -25, center: true}))
            },
            camera: new RtspCamera(),
            indicator: {}
        };

        logics.indicator.flash = (delay) => {
            this._led.blink(delay);
        };

        logics.indicator.on = (on) => {
            this._led.stop();
            (on ? this._led.on() : this._led.off());
        };

        logics.reset = () => {
            logics.camera.off(()=>{});
            logics.servos.pan.fiveDev.center();
            logics.servos.tilt.fiveDev.to(160);
            logics.motors.left.fiveDev.stop();
            logics.motors.right.fiveDev.stop();
            logics.led.fiveDev.stop();
        };

        logics.connected = (socket) => {
            logics.camera.setLocalAddress(socket.localAddress);
        };

        done(null, logics);
    }
}

module.exports = Builder;
