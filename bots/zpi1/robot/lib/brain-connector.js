'use strict';

const EventEmitter = require('events');
const dgram = require('dgram');

const PORT = 6565;
const CAST_INTERVAL = 5000;

class BrainConnector extends EventEmitter {
    constructor(name) {
        super();
        this.name = name;
        this._castBuf = new Buffer(this.name)
    }

    connect(done) {
        this.stop();
        this._sock = dgram.createSocket('udp4');
        this._sock.bind((err) => {
            if (err == null) {
                this._sock.setBroadcast(true);
                this._sock.on('message', (msg, rinfo) => this._onMessage(msg, rinfo));
                setImmediate(() => this._startBroadcast());
            }
            done(err);
        });
        return this;
    }

    stop() {
        if (this._timer) {
            clearInterval(this._timer);
            delete this._timer;
        }
        if (this._sock) {
            this._sock.close();
            delete this._sock;
        }
        return this;
    }

    _startBroadcast() {
        if (this._sock) {
            this._timer = setInterval(() => this._broadcast(), CAST_INTERVAL);
        }
    }

    _broadcast() {
        if (this._sock) {
            this._sock.send(this._castBuf, 0, this._castBuf.length, PORT, '255.255.255.255');
        }
    }

    _onMessage(msg, rinfo) {
        this.emit('offer', {
            name: msg.toString(),
            host: rinfo.address,
            port: rinfo.port
        });
    }
}

module.exports = BrainConnector;
