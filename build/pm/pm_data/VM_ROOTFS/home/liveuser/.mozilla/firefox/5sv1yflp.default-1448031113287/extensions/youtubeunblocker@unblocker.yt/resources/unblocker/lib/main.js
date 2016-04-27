const Config = require("./config.js");

var Controller;
exports.main = function (options) {

    Config.initConfig(options.loadReason, () => {
        Config.mergeConfig(() => {
            // start up Controller
            Controller = require("./controller").Controller;
            Controller.startup(options.loadReason);
        });
    });
};

exports.onUnload = function (reason) {

    if (!Controller) {
        return;
    }
    // shut down Controller
    Controller.shutdown(reason);
};
