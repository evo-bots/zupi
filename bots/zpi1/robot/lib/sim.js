'use strict';

const debug = require('debug')

class SimDevBase {
    constructor(name) {
        this.log = debug('zpi1.robot.dev.'+name);
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

class Builder {
    constructor() {
        this.log = debug('builder');
    }

    build(done) {
        done(null, {
            led: new LED(),
            motors: {
                left: new Motor("L"),
                right: new Motor("R")
            },
            servos: {
                pan: new Servo("P"),
                tilt: new Servo("T")
            },
            indicator: {
                flash: (delay) => this.log("indicator flashing %j", delay),
                on: (on) => this.log("indicator %s", on ? 'on' : 'off')
            }
        });
    }
}

module.exports = Builder;
