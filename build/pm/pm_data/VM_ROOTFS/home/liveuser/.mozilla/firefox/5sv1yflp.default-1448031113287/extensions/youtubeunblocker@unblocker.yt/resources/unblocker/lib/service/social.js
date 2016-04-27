// sdk
const { Class }    = require("sdk/core/heritage"),
    Preferences = require("sdk/simple-prefs");

//addon
const Config = require("../config").getConfig();

const Social = Class({

    MAX_SHOW_LARGE: Config.social.maxShowLarge,
    PREF_NAME     : "social_count",

    SIZE_LARGE: "big",
    SIZE_SMALL: "small",

    CLOSE_EVENT  : "social_close",
    SHOW_EVENT   : "social_show",
    COUNT_REQUEST: "social_request",

    uniqueLargeDisplays: [],

    incrementCount  : function () {
        const count = this.getShowCount();
        if (count < this.MAX_SHOW_LARGE) {
            Preferences.prefs[this.PREF_NAME] += 1;
        }
    },
    closeHint       : function () {
        Preferences.prefs[this.PREF_NAME] = this.MAX_SHOW_LARGE;
    },
    getHintSize     : function (identifier) {
        const count = this.getShowCount();

        //already shown
        if (this.uniqueLargeDisplays.indexOf(identifier) > -1) {
            return this.SIZE_SMALL;
        }

        if (count < this.MAX_SHOW_LARGE) {
            this.uniqueLargeDisplays.push(identifier);
            return this.SIZE_LARGE;
        }
        return this.SIZE_SMALL;
    },
    getShowCount    : function () {
        return Preferences.prefs[this.PREF_NAME];
    },
    bindEventsToItem: function (item) {
        item.on(this.CLOSE_EVENT, () => {
            this.closeHint();
        });
        item.on(this.SHOW_EVENT, () => {
            this.incrementCount();
        });
        item.on(this.COUNT_REQUEST, (identifier) => {
            item.emit(this.COUNT_REQUEST, this.getHintSize(identifier));
        });
    }
});

exports.Social = new Social();
