//sdk
const { Class } = require("sdk/core/heritage"),
    self = require("sdk/self"),
    panel = require("sdk/panel");

//addon
const Config = require("../config").getConfig();

const Panel = Class({
    panel     : null,
    initialize: function (options) {
        if (options === void(0)) {
            options = {};
        }
        this.panel = panel.Panel({
            width               : Config.panel.width,
            height              : Config.panel.height,
            contentURL          : self.data.url("templates/panel.html"),
            contentScriptFile   : [
                self.data.url("js/jquery.js"),
                self.data.url("ui/panel.js")
            ],
            contentScriptWhen   : "ready",
            contentStyleFile    : self.data.url("css/panel.css"),
            contentScriptOptions: options.settings ? options.settings : {}
        });
    }
});

exports.Panel = Panel;
