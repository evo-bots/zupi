'use strict';

const EventEmitter = require('events');
const dgram = require('dgram');

const PORT = 6565;
const CAST_INTERVAL = 5000;

class BrainConnector extends EventEmitter {
    constructor(name) {
        this.name = name;
        this._sock = dgram.createSocket('udp4');
        this._sock.setBroadcast(true);
    }

    connect(done) {
        this._sock.bind(PORT, (err) => {
            done(err);
            if (err == null) {
                this._sock.on('message', (msg, rinfo) => this._onMessage(msg, rinfo));
                setImmediate(() => this._startBroadcast());
            }
        });
    }

    stop() {
        this._sock.close();
    }

    _startBroadcast() {
        this._castBuf = new Buffer(this.name)
        this._timer = setInterval(() => this._broadcast(), CAST_INTERVAL);
    }

    _broadcast() {
        this._sock.send(this._castBuf, 0, this._castBuf.length, PORT, '255.255.255.255');
    }

    _onMessage(msg, rinfo) {
        this.emit('offer', {
            name: msg.toString(),
            host: rinfo.address,
            port: PORT
        });
    }
}

module.exports = BrainConnector;
