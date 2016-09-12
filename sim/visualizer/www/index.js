(function(exports) {
    'use strict';

    function unknownObject(props) {
        return Object.create({
            render: function (elem) {
                elem.innerHTML = '<span class="glyphicon glyphicon-question-sign"></span><span>&nbsp;' + props.id + '</span>';
                elem.classList.add('object-unknown');
            }
        });
    }

    function mapAs(dst, src, type) {
        if (src == null) {
            return null;
        }
        for (var key in src) {
            var val = src[key];
            if (typeof(val) == type) {
                dst[key] = val;
            }
        }
        return dst;
    }

    var ObjectElem = Class({
        constructor: function (world, props) {
            this._world = world;
            this._id = props.id;
            this._type = props.type;
        },

        id: function () {
            return this._id;
        },

        type: function () {
            return this._type;
        },

        properties: function () {
            return this._impl.properties;
        },

        rect: function () {
            return mapAs({x: 0, y: 0, w: 0, h: 0}, this.properties().rect, 'number');
        },

        origin: function () {
            return mapAs({x: 0, y: 0}, this.properties().origin, 'number');
        },

        build: function (impl) {
            if (this._impl && typeof(this._impl.destroy) == 'function') {
                this._impl.destroy();
            }
            this._impl = impl;
            var elem = document.createElement('div');
            elem.setAttribute("id", "object-" + this.id());
            elem.setAttribute("alt", this.id());
            elem.classList.add('object');
            elem.classList.add('object-type-' + this.type());
            if (this._elem) {
                this._elem.parentNode.replaceChild(elem, this._elem);
            } else {
                var worldElem = this._world.worldElem();
                worldElem.appendChild(elem);
            }
            this._elem = elem;
            this._impl.render(elem, this);
        },

        destroy: function () {
            if (this._impl) {
                if (typeof(this._impl.destroy) == 'function') {
                    this._impl.destroy();
                }
                delete this._impl;
            }
            if (this._elem) {
                this._elem.parentNode.removeChild(this._elem);
                delete this._elem;
            }
        },

        measure: function () {
            if (this._impl && typeof(this._impl.measure) == 'function') {
                return this._impl.measure();
            } else {
                var rc = this.rect();
                if (rc != null) {
                    var origOff = this.origin();
                    if (origOff != null) {
                        rc.x -= origOff.x;
                        rc.y += origOff.y;
                    }
                }
                return rc;
            }
        },

        place: function (viewRc, rc) {
            if (this._elem) {
                this._elem.style.left = viewRc.x + 'px';
                this._elem.style.top = viewRc.y + 'px';
                this._elem.style.width = viewRc.w + 'px';
                this._elem.style.height = viewRc.h + 'px';
                if (typeof(this._impl.place) == 'function') {
                    this._impl.place(viewRc, rc);
                }
            }
        }
    });

    function gridExtent(orig, min, max, fn) {
        var i, v;
        i = 1;
        for (v = orig - 10; v >= min; v -= 10) {
            fn(v, i % 10 == 0);
            i ++;
        }
        i = 1
        for (v = orig + 10; v <= max; v += 10) {
            fn(v, i % 10 == 0);
            i ++;
        }
    }

    var World = Class({
        constructor: function () {
            this._factories = {};
            this._objects = {};
        },

        connect: function (elem) {
            this._elem = elem;
            this._canvas = document.createElement('canvas');
            this._canvas.classList.add('grids');
            this._elem.appendChild(this._canvas);
            this._socket = io.connect();
            this._socket
                .on('objects', this.reload.bind(this))
                .on('update', this.update.bind(this));
        },

        register: function (type, factory) {
            this._factories[type] = factory;
            return this;
        },

        registerClass: function (cls) {
            this._factories[cls.Type] = function (props, world) {
                return new cls(props, world);
            };
        },

        createObjectImpl: function (props) {
            if (props == null || typeof(props.type) != 'string') {
                return null;
            }
            var factory = this._factories[props.type];
            if (factory != null) {
                return factory(props, this);
            } else {
                return unknownObject(props);
            }
        },

        addObjectElem: function (props) {
            var impl = this.createObjectImpl(props);
            if (impl == null) {
                return null;
            }
            var obj = this._objects[props.id];
            if (obj == null) {
                obj = new ObjectElem(this, props);
                this._objects[props.id] = obj;
            }
            obj.build(impl);
            return obj;
        },

        worldElem: function () {
            return this._elem;
        },

        updateLayout: function () {
            this.resizeGrids();
            var bounds = {
                minX: null,
                minY: null,
                maxX: null,
                maxY: null
            };
            var rects = {};
            for (var id in this._objects) {
                var rc = this._objects[id].measure();
                if (rc == null || rc.w <= 0 || rc.h <= 0) {
                    continue;
                }
                if (bounds.minX == null || bounds.minX > rc.x) {
                    bounds.minX = rc.x;
                }
                if (bounds.minY == null || bounds.minY > rc.y - rc.h) {
                    bounds.minY = rc.y - rc.h;
                }
                if (bounds.maxX == null || bounds.maxX < rc.x + rc.w) {
                    bounds.maxX = rc.x + rc.w;
                }
                if (bounds.maxY == null || bounds.maxY < rc.y) {
                    bounds.maxY = rc.y;
                }
                rects[id] = rc;
            }
            if (Object.keys(rects).length == 0) {
                return;
            }

            var w = this._elem.clientWidth, h = this._elem.clientHeight;
            var bw = bounds.maxX - bounds.minX, bh = bounds.maxY - bounds.minY;
            if (w == 0 || h == 0 || bw == 0 || bh == 0) {
                return;
            }

            var r = w / h, br = bw / bh, offx = 0, offy = 0;
            if (r > br) {
                var w1 = Math.ceil(h * br);
                offx = (w - w1) >> 1;
                w = w1;
            } else if (r < br) {
                var h1 = Math.ceil(w / br);
                offy = (h - h1) >> 1;
                h = h1;
            }

            for (var id in rects) {
                var rc = rects[id];
                var viewRc = {
                    x: Math.floor((rc.x - bounds.minX) * w / bw) + offx,
                    y: h - Math.floor((rc.y - bounds.minY) * h / bh) + offy,
                    w: Math.ceil(rc.w * w / bw),
                    h: Math.ceil(rc.h * h / bh)
                };
                this._objects[id].place(viewRc, rc);
            }

            this.updateGrids(offx - Math.floor(bounds.minX * w / bw),
                             offy + h + Math.floor(bounds.minY * h / bh),
                             offx, offy);
        },

        resizeGrids: function () {
            this._canvas.setAttribute('width', this._elem.clientWidth);
            this._canvas.setAttribute('height', this._elem.clientHeight);
        },

        updateGrids: function (origX, origY, offx, offy) {
            var w = this._canvas.clientWidth, h = this._canvas.clientHeight;
            var ctx = this._canvas.getContext('2d');
            ctx.clearRect(0, 0, w, h);
            ctx.strokeStyle = $(this._canvas).css('color');
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(origX, offy);
            ctx.lineTo(origX, h - offy);
            ctx.moveTo(offx, origY);
            ctx.lineTo(w - offx, origY);
            ctx.stroke();
            ctx.setLineDash([1, 1]);
            ctx.beginPath();
            gridExtent(origX, offx, w - offx, function (x, solid) {
                if (solid) {
                    ctx.moveTo(x, offy);
                    ctx.lineTo(x, h - offy);
                } else {
                    gridExtent(origY, offy, h - offy, function (y) {
                        ctx.moveTo(x, y);
                        ctx.lineTo(x+1, y+1);
                    });
                }
            });
            gridExtent(origY, offy, h - offy, function (y, solid) {
                if (solid) {
                    ctx.moveTo(offx, y);
                    ctx.lineTo(w - offx, y);
                }
            });
            ctx.stroke();
        },

        reload: function (objects) {
            for (var id in this._objects) {
                this._objects[id].destroy();
            }
            this._objects = {};
            for (var id in objects) {
                this.addObjectElem(objects[id]);
            }
            this.updateLayout();
        },

        update: function (cmds) {
            cmds.forEach(function (cmd) {
                if (typeof(cmd.action) == 'string') {
                    var fn = this['_update_' + cmd.action];
                    if (fn != null) {
                        fn.call(this, cmd);
                    }
                }
            }, this);
            this.updateLayout();
        },

        _update_object: function (cmd) {
            console.log('update-object: %j', cmd.object);
            this.addObjectElem(cmd.object);
        },

        _update_remove: function (cmd) {
            if (typeof(cmd.id) == 'string') {
                var obj = this._objects[cmd.id];
                if (obj != null) {
                    delete this._objects[cmd.id];
                    obj.destroy();
                }
            }
        }
    });

    var theWorld = new World();

    $(document).ready(function () {
        theWorld.connect(document.getElementById('world'));
        window.addEventListener('resize', theWorld.updateLayout.bind(theWorld));
        theWorld.updateLayout();
    });

    exports.world = theWorld;

    var ObjectBase = Class({
        constructor: function (props, world) {
            this.properties = props;
            this.world = world;
        },

        render: function (elem, outer) {
            this.outerElem = elem;
            this.outer = outer;
        }
    });

    var CanvasObject = Class(ObjectBase, {
        constructor: function () {
            ObjectBase.prototype.constructor.apply(this, arguments);
        },

        render: function () {
            ObjectBase.prototype.render.apply(this, arguments);
            this._canvas = document.createElement('canvas');
            this._canvas.classList.add('object');
            this._canvas.classList.add(this.properties.type);
            this.outerElem.appendChild(this._canvas);
        },

        place: function (viewRc) {
            this._canvas.setAttribute('width', viewRc.w);
            this._canvas.setAttribute('height', viewRc.h);
            this.paint(viewRc, this._canvas);
        },

        paint: function () {
            // to be overridden
        }
    });

    function defineObject (type, prototype, baseClass) {
        if (baseClass == null) {
            baseClass = ObjectBase;
        }
        var ctor = prototype.constructor;
        prototype.constructor = function () {
            baseClass.prototype.constructor.apply(this, arguments);
            if (ctor != null) {
                ctor.apply(this.arguments);
            }
        };
        var cls = Class(baseClass, prototype, { statics: { Type: type } });
        theWorld.registerClass(cls);
        return cls;
    }

    function defineCanvasObject (type, prototype) {
        return defineObject(type, prototype, CanvasObject);
    }

    exports.vis = {
        mapAs: mapAs,

        ObjectBase: ObjectBase,
        CanvasObject: CanvasObject,

        defineObject: defineObject,
        defineCanvasObject: defineCanvasObject,
    };
})(window);
