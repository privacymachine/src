// sdk
const { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object"),
    self = require("sdk/self");

//addon

const { VideoManager } = require("../manager/video"),
    { PageModAbstract } = require("./abstract"),
    Config = require("../config").getConfig();

const PageModEmbed = Class({
    extends      : PageModAbstract,
    initialize   : function initialize(options) {
        PageModAbstract.prototype.initialize.call(this, options);
        merge(this, options);

        this.setup();

        this.pageModOptions = {
            include             : new RegExp(Config.pagemod.embed.include),
            attachTo            : ["frame"],
            attachWhen          : "start",
            onAttach            : this.installWorker.bind(this),
            contentScriptFile   : [
                self.data.url("js/jquery.js"),
                self.data.url("pagemod/embed.js")
            ],
            contentStyleFile    : [
                self.data.url("css/watch.css"),
                self.data.url("css/social.css")
            ],
            contentScriptOptions: Config.pagemod.embed.options
        };

    },
    installWorker: function (worker) {
        const identifier = worker.url.match(new RegExp(Config.pagemod.embed.activeRegexp, "i"));
        if (identifier && identifier[Config.pagemod.embed.activeRegexpGroup]) {

            PageModAbstract.prototype.installWorker.call(this, worker);
            worker.port.emit("identifier", identifier[Config.pagemod.embed.activeRegexpGroup]);
            worker.port.on("status", function (identifier) {
                const video = VideoManager.findOrCreateItemById(identifier);
                worker.port.emit("status", video.status);
            });
            worker.port.on("start", function () {
                const video = VideoManager.findOrCreateItemById(identifier[Config.pagemod.embed.activeRegexpGroup]);
                video.handle({
                    "unblocking": function () {

                        worker.port.emit("unblocking", null);
                    },
                    "blocked"   : function () {

                        worker.port.emit("blocked", null);
                    },
                    "error"     : function () {

                        worker.port.emit("error", null);
                    },
                    "free"      : function () {

                        worker.port.emit("free", null);
                    },
                    "unblocked" : function () {

                        worker.port.emit("unblocked", null);
                    }
                });
            });
        }
    }
});

exports.PageModEmbed = new PageModEmbed();
