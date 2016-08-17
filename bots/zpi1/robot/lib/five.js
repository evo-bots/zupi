'use strict';

const five = require('tbus-five');

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
        let logics = {
            led: new five.Led(13),
            motors: {
                left: new five.Motor(five.Motor.SHIELD_CONFIGS.POLOLU_DRV8835_SHIELD.M1),
                right: new five.Motor(five.Motor.SHIELD_CONFIGS.POLOLU_DRV8835_SHIELD.M2)
            },
            servos: {
                pan: new five.Servo({controller: 'PCA9685', pin: 0}),
                tilt: new five.Servo({controller: 'PCA9685', pin: 1})
            },
            indicator: {}
        };

        logics.indicator.flash = (delay) => {
            logics.led.blink(delay);
        };

        logics.indicator.on = (on) => {
            logics.led.stop();
            (on ? logics.led.on() : logics.led.off());
        };

        done(null, logics);
    }
}

module.exports = Builder;
