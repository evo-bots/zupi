const spawn = require('child_process').spawn;
const Logic = require('./logic.js');
const CameraState = require('../gen/zupi/camera_pb.js').CameraState;
const RstpEndpoint = require('../gen/zupi/camera_pb.js').RstpEndpoint;

class RstpCamera extends Logic {
    constructor() {
        super('camera');
    }

    getState(done) {
        let state = new CameraState();
        if (this.rstp) {
            state.setMode(CameraState.Video);
            let endpoint = new RstpEndpoint();
            endpoint.setHost(this.rstp.host);
            endpoint.setPort(this.rstp.port);
            state.setRstp(endpoint);
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
        this[method].call(this, (err) {
            if (err == null) {
                this.GetState(done);
            } else {
                done(err);
            }
        });
    }

    on(done) {
        if (this.rstp == null) {
            this.rstp = {
                host: '0.0.0.0',
                port: 8554,
                server: spawn('v4l2rstpserver',
                    ['-W', '320', '-H', '240', '-F', '30'],
                    { stdio: 'inherit' })
            }
            this.rstp.server
                .on('exit', (reason) => this._rstpExit(null, reason))
                .on('error', (err) => this._rstpExit(err));
        }
        done();
    }

    off(done) {
        if (this.rstp) {
            this.rstp.exitCallback = () => done();
            this.rstp.kill('SIGTERM');
        }
    }

    _rstpExit(err, reason) {
        if (err) {
            this.log('rstp server error %v', err)
        } else {
            this.log('rstp server exit %v', reason)
        }
        let cb = this.rstp ? this.rstp.exitCallback : null;
        delete this.rstp;
        if (cb) {
            cb();
        }
    }
}

module.exports = RstpCamera;
