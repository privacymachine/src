// sdk
const { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object"),
    self = require("sdk/self");

//addon
const { PageModAbstract } = require("./abstract"),
    { VideoInfo } = require("../service/video-info"),
    { VideoManager } = require("../manager/video"),
    Config = require("../config").getConfig();

const PageModFacebook = Class({
    extends      : PageModAbstract,
    initialize   : function initialize(options) {
        PageModAbstract.prototype.initialize.call(this, options);
        merge(this, options);

        this.setup();

        this.pageModOptions = {
            include          : new RegExp(Config.pagemod.facebook.include),
            attachTo         : ["frame", "existing", "top"],
            attachWhen       : "ready",
            onAttach         : this.installWorker.bind(this),
            contentScriptFile: [
                self.data.url("js/jquery.js"),
                self.data.url("pagemod/facebook.js")
            ]
        };
    },
    installWorker: function (worker) {
        worker.port.on("check_status", function (url) {

            const videoId = url.match(new RegExp(Config.pagemod.facebook.activeRegexp, "i"));
            if (videoId && videoId[Config.pagemod.facebook.activeRegexpGroup]) {
                if (Config.pagemod.facebook.inactiveRegexp) {
                    if (url.match(new RegExp(Config.pagemod.facebook.inactiveRegexp, "ig"))) {
                        return;
                    }
                }
                const video = VideoManager.findItemById(videoId[Config.pagemod.facebook.activeRegexpGroup]);
                if (video && video.status !== null) {
                    worker.port.emit(
                        "check_status",
                        {
                            "url"    : url,
                            "status" : video.status,
                            "videoId": videoId[Config.pagemod.facebook.activeRegexpGroup]
                        }
                    );
                }
                else {
                    VideoInfo.get(videoId[Config.pagemod.facebook.activeRegexpGroup], function (error, status) {
                        worker.port.emit(
                            "check_status",
                            {
                                "url"    : url,
                                "status" : status === VideoInfo.STATUS_OK ? "free" : "blocked",
                                "videoId": videoId[Config.pagemod.facebook.activeRegexpGroup]
                            }
                        );
                    });
                }
            }
        });
    }
});

exports.PageModFacebook = new PageModFacebook();
