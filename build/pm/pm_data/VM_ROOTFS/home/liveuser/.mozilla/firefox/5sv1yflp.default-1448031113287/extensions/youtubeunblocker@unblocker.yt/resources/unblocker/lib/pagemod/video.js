// sdk
const { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object"),
    self = require("sdk/self"),
    Path = require('sdk/fs/path'),
    base64 = require("sdk/base64");

//addon

const { VideoManager } = require("../manager/video"),
    { PageModAbstract } = require("./abstract"),
    Config = require("../config").getConfig();

//XP
const {Cc, Ci, Cu} = require("chrome");
Cu.import("resource://gre/modules/AddonManager.jsm");
Cu.import("resource://gre/modules/FileUtils.jsm");

const PageModVideo = Class({
        extends         : PageModAbstract,
        initialize      : function initialize(options) {
            PageModAbstract.prototype.initialize.call(this, options);
            merge(this, options);

            this.setup();

            this.pageModOptions = {
                include   : new RegExp(Config.pagemod.video.include),
                attachTo  : ["existing", "top"],
                attachWhen: "start",
                onAttach  : this.installWorker.bind(this),

                contentScriptFile: [
                    self.data.url("js/jquery.js"),
                    self.data.url("pagemod/video.js")
                ],
                contentStyleFile : [
                    self.data.url("css/watch.css"),
                    self.data.url("css/social.css")
                ],

                contentScriptOptions: Config.pagemod.video.options
            };
        },
        install         : function () {
            this.getCampaignImage((data) => {
                this.pageModOptions.contentScriptOptions.campaignImage = data;
                PageModAbstract.prototype.install.call(this);
            });

        },
        getCampaignImage: function (next) {
            AddonManager.getAddonByID(self.id, function (addon) {
                addon.getDataDirectory(function (directory) {
                    try {
                        if (directory) {

                            var bStream = Cc["@mozilla.org/binaryinputstream;1"].
                                createInstance(Ci.nsIBinaryInputStream);
                            var fileInStream = Cc["@mozilla.org/network/file-input-stream;1"].
                                createInstance(Ci.nsIFileInputStream);

                            const nsIFile = FileUtils.File(Path.join(directory, "data" + Path.sep + "campaign_background.png"));
                            fileInStream.init(nsIFile, -1, -1, 0);
                            bStream.setInputStream(fileInStream);

                            var bytes = [];
                            while (bStream.available() != 0) {
                                bytes = bytes.concat(bStream.readByteArray(bStream.available()));
                            }
                            bStream.close();

                            next("data:image/png;base64," + base64.encode(String.fromCharCode.apply(null, bytes)));
                        }
                    } catch (e) {
                        next("");
                    }
                });
            });
        }
        ,
        installWorker   : function (worker) {
            const identifierMatch = worker.tab.url.match(
                new RegExp(Config.pagemod.video.activeRegexp)
            );

            if (!identifierMatch || !identifierMatch[Config.pagemod.video.activeRegexpGroup]) {
                return;
            }

            const identifier = identifierMatch[Config.pagemod.video.activeRegexpGroup];

            PageModAbstract.prototype.installWorker.call(this, worker);

            worker.port.emit("identifier", identifier);
            worker.port.on("start", function () {
                const video = VideoManager.findOrCreateItemById(identifier);
                video.handle({
                    "unblocking": function () {

                        worker.port.emit("unblocking", null);
                    },
                    "unblocked" : function () {

                        worker.port.emit("unblocked", null);
                    },
                    "blocked"   : function () {

                        worker.port.emit("blocked", null);
                    },
                    "error"     : function () {

                        worker.port.emit("error", null);
                    },
                    "free"      : function () {

                        worker.port.emit("free", null);
                    }
                });
            });
        }
    })
    ;

exports.PageModVideo = new PageModVideo();
