(function (exports) {
    'use strict';

    vis.defineCanvasObject('dot', {
        measure: function () {
            var origin = this.outer.origin();
            if (origin == null) {
                return null;
            }
            var radius = this.properties.radius;
            if (typeof(radius) != 'number') {
                return null;
            }
            return {
                x: origin.x - radius,
                y: origin.y + radius,
                w: radius * 2,
                h: radius * 2
            };
        },

        paint: function (rc, canvas) {
            var ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, rc.w, rc.h);
            var x0 = rc.w >> 1;
            var y0 = rc.h >> 1;
            var r = Math.floor(Math.min(rc.w, rc.h) / 2);
            ctx.fillStyle = $(canvas).css('color');
            ctx.beginPath();
            ctx.arc(x0, y0, r, 0, 2 * Math.PI);
            ctx.fill();
            //ctx.stroke();
        }
    });

    vis.defineCanvasObject('corner', {
        measure: function () {
            var rc = this.outer.rect();
            if (rc != null && rc.w > 0 && rc.h > 0) {
                switch (this.properties.loc) {
                case 'lt':
                    break;
                case 'rt':
                    rc.x -= rc.w;
                    break;
                case 'lb':
                    rc.y += rc.h;
                    break;
                case 'rb':
                    rc.y += rc.h;
                    rc.x -= rc.w;
                }
            }
            return rc;
        },

        paint: function (viewRc, canvas) {
            var w = viewRc.w, h = viewRc.h;
            var ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, w, h);
            ctx.strokeStyle = $(canvas).css('color');
            ctx.lineWidth = 1;
            ctx.beginPath();
            switch (this.properties.loc) {
            case 'lt':
                ctx.moveTo(0, h);
                ctx.lineTo(0, 0);
                ctx.lineTo(w, 0);
                break;
            case 'rt':
                ctx.moveTo(0, 0);
                ctx.lineTo(w - 1, 0);
                ctx.lineTo(w - 1, h);
                break;
            case 'lb':
                ctx.moveTo(0, 0);
                ctx.lineTo(0, h - 1);
                ctx.lineTo(w, h - 1);
            case 'rb':
                ctx.moveTo(0, h - 1);
                ctx.lineTo(w - 1, h - 1);
                ctx.lineTo(w - 1, 0);
            }
            ctx.stroke();
        }
    });

    vis.defineCanvasObject('camera', {
        measure: function () {
            var origin = this.outer.origin();
            if (origin == null) {
                return null;
            }
            var radius = this.properties.radius;
            if (typeof(radius) != 'number') {
                return null;
            }
            return {
                x: origin.x - radius,
                y: origin.y + radius,
                w: radius * 2,
                h: radius * 2
            };
        },

        paint: function (rc, canvas) {
            var angle = this.properties.angle;
            if (angle == null) {
                angle = 0;
            }
            var x0 = rc.w >> 1;
            var y0 = rc.h >> 1;
            var r = Math.min(Math.min(rc.w / 2, rc.h / 2), Math.sqrt(rc.w * rc.w + rc.h * rc.h) / 2);
            var t = (45 + angle) * Math.PI / 180;
            var x1 = Math.ceil(r * Math.cos(t));
            var y1 = Math.ceil(r * Math.sin(t));
            var t = (-45 + angle) * Math.PI / 180;
            var x2 = Math.ceil(r * Math.cos(t));
            var y2 = Math.ceil(r * Math.sin(t));

            var ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, rc.w, rc.h);
            ctx.beginPath();
            ctx.moveTo(x0, y0);
            ctx.lineTo(x0 + x1, y0 - y1);
            ctx.lineTo(x0 + x2, y0 - y2);
            ctx.closePath();
            ctx.stroke();
            ctx.setLineDash([1, 1]);
            ctx.beginPath();
            ctx.moveTo(0, y0);
            ctx.lineTo(rc.w, y0);
            ctx.moveTo(x0, 0);
            ctx.lineTo(x0, rc.h);
            ctx.stroke();
        }
    });
})(window);
