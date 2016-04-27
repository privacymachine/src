// sdk
const { EventTarget } = require("sdk/event/target"),
    { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object");

const AbstractManager = Class({
    extends             : EventTarget,
    modelClass          : null,
    items               : {},
    initialize          : function initialize(options) {
        EventTarget.prototype.initialize.call(this, options);
        merge(this, options);
    },
    findOrCreateItemById: function (itemId) {

        let item = this.findItemById(itemId);

        if (!item) {
            item = new this.modelClass(itemId);
            this.items[itemId] = item;
        }
        return item;
    },
    findItemById        : function (itemId) {

        if (itemId in this.items) {
            return this.items[itemId];
        }

        return null;
    },
    purge               : function () {
        this.items = {};
    }
});

exports.AbstractManager = AbstractManager;
