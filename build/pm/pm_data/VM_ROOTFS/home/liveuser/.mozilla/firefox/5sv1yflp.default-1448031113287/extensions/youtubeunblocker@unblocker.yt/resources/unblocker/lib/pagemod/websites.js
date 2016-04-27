// sdk
const { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object"),
    self = require("sdk/self");

//addon
const { PageModAbstract } = require("./abstract"),
    { VideoInfo } = require("../service/video-info"),
    { VideoManager } = require("../manager/video"),
    Config = require("../config").getConfig();

const PageModWebsites = Class({
    extends      : PageModAbstract,
    initialize   : function initialize(options) {
        PageModAbstract.prototype.initialize.call(this, options);
        merge(this, options);

        this.setup();
        this.pageModOptions = {
            include          : new RegExp(Config.pagemod.websites.include),
            exclude          : new RegExp(Config.pagemod.websites.exclude),
            attachTo         : ["existing", "top"],
            attachWhen       : "ready",
            onAttach         : this.installWorker.bind(this),
            contentScriptFile: [
                self.data.url("js/jquery.js"),
                self.data.url("pagemod/websites.js")
            ]
        };
    },
    installWorker: function (worker) {
        worker.port.on("check_status", function (url) {
            const videoId = url.match(new RegExp(Config.pagemod.websites.activeRegexp, "i"));
            if (videoId && videoId[Config.pagemod.websites.activeRegexpGroup]) {
                const video = VideoManager.findItemById(videoId[Config.pagemod.websites.activeRegexpGroup]);
                if (video && video.status !== null) {
                    worker.port.emit(
                        "check_status",
                        {
                            "url"    : url,
                            "status" : video.status,
                            "videoId": videoId[Config.pagemod.websites.activeRegexpGroup]
                        }
                    );
                }
                else {
                    VideoInfo.get(videoId[Config.pagemod.websites.activeRegexpGroup], function (error, status) {
                        worker.port.emit(
                            "check_status",
                            {
                                "url"    : url,
                                "status" : status === VideoInfo.STATUS_OK ? "free" : "blocked",
                                "videoId": videoId[Config.pagemod.websites.activeRegexpGroup]
                            }
                        );
                    });
                }
            }
        });
    }
});

exports.PageModWebsites = new PageModWebsites();
