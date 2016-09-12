'use strict';

class Object {
    constructor(props) {
        this._properties = props;
        this._id = props.id;
        this._type = props.type;
    }

    id() {
        return this._id;
    }

    type() {
        return this._type;
    }

    present() {
        return this._properties.length > 1;
    }

    valid() {
        return this._id != null && this._type != null;
    }

    properties() {
        return this._properties;
    }
}

module.exports = Object;
