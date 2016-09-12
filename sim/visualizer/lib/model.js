'use strict';

const EventEmitter = require('events');
const spawn = require('child_process').spawn;
const Object = require('./object.js');
const debug = require('debug')('vis:model');

class Model extends EventEmitter {
    constructor(argv) {
        super();
        this._objects = {};
        this._process = spawn(argv[0], argv.slice(1), {
            stdio: ['pipe', 'ipc', process.stderr]
        });
        this._process
            .on('exit', (code, signal) => this._processExit(code, signal))
            .on('message', (msg) => this._handleMsg(msg));
    }

    update(docs) {
        if (this._process != null) {
            let msg = JSON.stringify(docs);
            debug('update %s', msg);
            this._process.stdin.write(msg);
        }
        return this;
    }

    objects() {
        let objs = {}, id;
        for (id in this._objects) {
            objs[id] = this._objects[id].properties();
        }
        return objs;
    }

    _handleMsg(msg) {
        debug('message %j', msg);
        if (msg == null) {
            return;
        }
        let cmds = msg;
        if (!Array.isArray(cmds)) {
            cmds = [cmds];
        }
        cmds.forEach((cmd) => {
            if (typeof(cmd.action) == 'string') {
                let fn = this[`_action_${cmd.action}`];
                if (fn != null) {
                    fn.call(this, cmd);
                }
            }
        });
        this.emit('update', cmds);
    }

    _processExit(code, signal) {
        delete this._process;
        debug('process exited: %d, %j', code, signal);
    }

    _action_object(cmd) {
        if (cmd.object == null) {
            return false;
        }
        let obj = new Object(cmd.object);
        if (!obj.valid()) {
            return false;
        }
        this._objects[obj.id()] = obj;
        return true;
    }

    _action_remove(cmd) {
        let id = cmd.id;
        if (id == null) {
            return false;
        }
        delete this._objects[id];
        return true;
    }
}

module.exports = Model;
