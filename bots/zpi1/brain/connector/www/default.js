(function (app) {
    'use strict';

    app.Model = {
        brains: []
    };

    var queryTimer;

    function selectBrain(name) {
        $.ajax('connect', {
            method: 'PUT',
            contentType: 'application/json',
            data: JSON.stringify({allowed: [name]})
        }).always(query);
    }

    function updateStatus(status) {
        if (!status) {
            return;
        }

        app.Model.name = status.name;
        var brains = [], names = {};
        if (Array.isArray(status.brains)) {
            brains = status.brains.map(function(b, index) {
                names[b] = index;
                return {name: b};
            });
        }
        if (Array.isArray(status.allowed)) {
            status.allowed.forEach(function(a) {
                var index = names[a];
                if (index != null) {
                    brains[index].allowed = true;
                }
            });
        }
        app.Model.brains = brains;
        if (status.current) {
            var index = names[status.current.name];
            if (index != null) {
                brains[index].connected = status.current;
            }
            app.Model.current = current;
        }

        window.dispatchEvent(new Event('status'));
    }

    function query() {
        $.get('connect', updateStatus).always(function() {
            if (queryTimer) {
                clearTimeout(queryTimer);
            }
            queryTimer = setTimeout(query, 1000);
        });
    }

    // bootstrap Angular2
    document.addEventListener('DOMContentLoaded', function() {
        var components = [
            ng.core.Component({
                selector: '#brains',
                templateUrl: "templates/brains.html"
            })
            .Class({
                constructor: function() {
                    this.M = app.Model;
                    window.addEventListener('status', function() {});
                },

                onClick: function(name) {
                    selectBrain(name);
                }
            }),
            ng.core.Component({
                selector: '#botname',
                template: "{{M.name}}"
            })
            .Class({
                constructor: function() {
                    this.M = app.Model;
                    window.addEventListener('status', function() {});
                }
            })
        ];
        ng.platformBrowserDynamic
          .platformBrowserDynamic()
          .bootstrapModule(ng.core.NgModule({
              imports: [ng.platformBrowser.BrowserModule],
              declarations: components,
              bootstrap: components
          }).Class({
              constructor: function() {}
          }));
    });

    // start status polling
    $(document).ready(function() {
        query();
    });
})(window.app || (window.app = {}));
