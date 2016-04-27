// sdk
const { Ci }    = require("chrome"),
{ EventTarget }   = require("sdk/event/target"),
{ Class }         = require("sdk/core/heritage"),
events = require("sdk/system/events"),
tabs   = require("sdk/tabs");

// addon
const Config = require("../config").getConfig(),
{ VideoInfo } = require("../service/video-info"),
{ VideoManager } = require("../manager/video");

//3rd party
const async = require("async");

const YoutubeAjaxObserver = Class({
    bindMethod  : null,
    extends     : EventTarget,
    rulesets    : null,
    initialize  : function initialize(options) {
        EventTarget.prototype.initialize.call(this, options);
    },
    setRulesets : function (rulesets) {
        this.rulesets = rulesets;
    },
    start       : function () {
        if (this.bindMethod) {
            return;
        }
        this.bindMethod = this.observe.bind(this);
        events.on("http-on-examine-response", this.bindMethod);
        events.on("http-on-examine-cached-response", this.bindMethod);
    },
    stop        : function () {
        if (!this.bindMethod) {
            return;
        }
        events.off("http-on-examine-response", this.bindMethod);
        events.off("http-on-examine-cached-response", this.bindMethod);
        this.bindMethod = null;
    },
    observe     : function (event) {
        const _this = this;
        const channel = event.subject.QueryInterface(Ci.nsIHttpChannel);
        let matches = false;

        //host check
        for (let host in Config.observer.ajax.uri.hosts) {
            if (channel.originalURI.host.match(new RegExp(Config.observer.ajax.uri.hosts[host], "i"))) {
                matches = true;
                break;
            }
            else {
                matches = false;
            }
        }

        if (!matches) {
            return;
        }

        //header check
        for (let header in Config.observer.ajax.headers) {
            try {
                const headerValue = channel.getResponseHeader(header);
                if (headerValue.match(new RegExp(Config.observer.ajax.headers[header], "i"))) {
                    matches = true;
                    break;
                }
                else {
                    matches = false;
                }
            } catch (e) {
                matches = false;
            }
        }

        if (!matches) {
            return;
        }

        //path check

        for (let path in Config.observer.ajax.uri.paths) {
            if (channel.originalURI.path.match(new RegExp(Config.observer.ajax.uri.paths[path], "i"))) {
                matches = true;
                break;
            }
            else {
                matches = false;
            }
        }

        if (!matches) {
            return;
        }

        for (let regexp in Config.observer.ajax.identifiers) {
            const match = channel.originalURI.path.match(new RegExp(regexp, "i"));
            if (match) {
                const value = match[Config.observer.ajax.identifiers[regexp]];

                if (value !== void(0)) {
                    async.parallel(
                        {
                            previous: function (callback) {
                                const previousMatch = channel.referrer.path.match(new RegExp(regexp, "i"));
                                if (!previousMatch) {
                                    return callback(null, "NO_VIDEO");
                                }

                                const previousValue = previousMatch[Config.observer.ajax.identifiers[regexp]],
                                videoKnown = VideoManager.findItemById(previousValue);

                                if (videoKnown) {
                                    return callback(
                                        null,
                                        videoKnown.status === "unblocked" ?
                                            VideoInfo.STATUS_BLOCKED :
                                            VideoInfo.STATUS_OK
                                    );
                                }
                                else {
                                    VideoInfo.get(previousValue, (error, status) => {
                                        return callback(error, status);
                                    });
                                }
                            },
                            current : function (callback) {

                                
                                const videoKnown = VideoManager.findItemById(value);

                                if (videoKnown) {
                                    return callback(
                                        null,
                                        videoKnown.status === "unblocked" ?
                                            VideoInfo.STATUS_BLOCKED :
                                            VideoInfo.STATUS_OK
                                    );
                                }
                                else {
                                    VideoInfo.get(value, (error, status) => {
                                        return callback(error, status);
                                    });
                                }
                            }
                        },
                        function (err, results) {

                            
                            

                            if (
                                err || !(
                                (results.previous === "NO_VIDEO" && results.current === VideoInfo.STATUS_OK) ||
                                (results.previous === VideoInfo.STATUS_OK && results.current === VideoInfo.STATUS_OK)
                                )
                            ) {
                                _this.reloadWindow(event.subject, value);
                            }

                        }
                    );
                }
                else {
                    _this.reloadWindow(event.subject);
                }
                break;
            }
        }
    },
    reloadWindow: function (aSubject, videoId) {
        const oHttp = aSubject.QueryInterface(Ci.nsIHttpChannel);
        if (!oHttp.notificationCallbacks) {
            return tabs.activeTab.reload();
        }

        try {
            const topFrame = oHttp.notificationCallbacks.getInterface(Ci.nsILoadContext).topFrameElement;
            if (!topFrame) {
                return tabs.activeTab.reload();
            }

            const browserMM = topFrame.messageManager;
            browserMM.loadFrameScript("chrome://youtube-unblocker/content/ajax.js", false);
            browserMM.sendAsyncMessage("unblocker;reload");
        } catch (ex) {
            if (videoId) {
                for (let tab of tabs) {
                    if (tab.url.indexOf(videoId) != -1) {
                        tab.reload();
                        return;
                    }
                }
            }
            tabs.activeTab.reload();
        }

    }
});

exports.YoutubeAjaxObserver = new YoutubeAjaxObserver();
