'use strict';

const spawn = require('child_process').spawn;
const Logic = require('./logic.js');
const CameraState = require('../gen/zupi/camera_pb.js').CameraState;
const RtspEndpoint = require('../gen/zupi/camera_pb.js').RtspEndpoint;

class RtspCamera extends Logic {
    constructor() {
        super('camera');
        this._localAddr = '0.0.0.0';
    }

    getState(done) {
        let state = new CameraState();
        if (this.rtsp) {
            state.setMode(CameraState.Video);
            let endpoint = new RtspEndpoint();
            endpoint.setHost(this.rtsp.host);
            endpoint.setPort(this.rtsp.port);
            state.setRtsp(endpoint);
        } else {
            state.setMode(CameraState.OFF);
        }
        done(null, state);
    }

    setState(state, done) {
        let method = '';
        switch (state.getMode()) {
        case CameraState.Mode.OFF:
            method = 'off';
            break;
        case CameraState.Mode.VIDEO:
            method = 'on';
            break;
        default:
            done(new Error("invalid mode"));
            return;
        }
        this[method].call(this, (err) => {
            if (err == null) {
                this.getState(done);
            } else {
                done(err);
            }
        });
    }

    on(done) {
        if (this.rtsp == null) {
            this.rtsp = {
                host: this._localAddr,
                port: 8554,
                server: spawn('v4l2rtspserver',
                    ['-W', '320', '-H', '240', '-F', '30'],
                    { stdio: 'inherit' })
            }
            this.rtsp.server
                .on('exit', (reason) => this._rtspExit(null, reason))
                .on('error', (err) => this._rtspExit(err));
        }
        done();
    }

    off(done) {
        if (this.rtsp) {
            this.rtsp.exitCallback = () => done();
            this.rtsp.server.kill('SIGTERM');
        }
    }

    setLocalAddress(address) {
        this._localAddr = address;
    }

    _rtspExit(err, reason) {
        if (err) {
            this.log('rtsp server error %j', err)
        } else {
            this.log('rtsp server exit %j', reason)
        }
        let cb = this.rtsp ? this.rtsp.exitCallback : null;
        delete this.rtsp;
        if (cb) {
            cb();
        }
    }
}

module.exports = RtspCamera;
