'use strict';

const express = require('express');
const bodyParser = require('body-parser');
const morgan = require('morgan');
const socket = require('socket.io');
const debug = require('debug')('vis:app');
const Model = require('./lib/model.js');

var app = express();
app.use(express.static(__dirname + '/www'));
app.use(express.static(__dirname + '/node_modules'));
app.use(morgan('dev'));
app.use(bodyParser.json());

var model = new Model(process.argv.slice(2));

app.get('/model/objects', (req, res) => {
    res.json(model.objects());
});

app.post('/model/update', (req, res) => {
    model.update(req.body);
    res.status(204).end();
});

var port = process.env.PORT || 3333;
var server = app.listen(port, (err) => {
    if (err) {
        console.error(err);
    } else {
        debug('Listening on port ' + port);
    }
});

var io = socket(server);
model.on('update', (msg) => io.emit('update', msg));
io.on('connection', (conn) => {
    debug('client socket %j', conn.id);
    conn.emit('objects', model.objects());
    conn.on('update', (msg) => model.update(msg));
});
