(function (exports) {
    'use strict';

    vis.defineObject('reward', {
        createContent: function () {
            this._elem = document.createElement('span');
            this._elem.innerHTML = this.properties.reward;
            if (this.properties.level != null) {
                this._elem.classList.add('reward-' + this.properties.level);
            } else if (typeof(this.properties.reward) == 'number') {
                var g = Math.floor(255 * this.properties.reward);
                var r = Math.floor(255 * (1 - this.properties.reward));
                this._elem.style.color = "rgb(" + r + ", " + g + ", 0)";
            }
            return this._elem;
        }
    });

})(window);
