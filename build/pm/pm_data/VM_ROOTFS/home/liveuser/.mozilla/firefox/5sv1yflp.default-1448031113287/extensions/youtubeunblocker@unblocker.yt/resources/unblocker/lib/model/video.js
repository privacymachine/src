//sdk
const { Class } = require("sdk/core/heritage"),
//addon
    { VideoInfo } = require("../service/video-info"),
    { ProxyManager } = require("../manager/proxy"),
    { VideoRestriction } = require("../service/video-restriction"),
    async = require("async"),
    Config = require("../config").getConfig();

const Video = Class({
    id            : null,
    status        : null,
    proxyHost     : null,
    initialize    : function (id) {
        this.id = id;
        this.status = null;
        this.proxyHost = null;
    },
    handle        : function (handlerObject) {
        switch (this.status) {
            case null:
                // start retrieving status from youtube
                this.retrieveStatus(handlerObject);
                break;

            case "unblocking":
            case "unblocked":
            case "blocked":
            case "error":
            case "free":
                this.handleStatus(handlerObject);
                break;

            default:
                console.error("oops, unknown status", this);
        }
    },
    handleStatus  : function (handlerObject) {
        if (handlerObject[this.status]) {
            handlerObject[this.status]();
        }
        else {
            console.error("no handler", this.status, "in handlerObject");
        }
    },
    retrieveStatus: function (handlerObject) {
        const _this = this;

        VideoInfo.get(_this.id, function (error, status) {

            if (error) {
                console.error(error);
                return;
            }

            if (status === VideoInfo.STATUS_OK) {
                _this.status = "free";
                _this.handleStatus(handlerObject);
            }
            else {
                VideoRestriction.get(_this.id, (error, restrictions) => {
                    if (error) {
                        console.error(error);
                        return;
                    }
                    _this.unblock(restrictions, handlerObject);
                });
            }
        });
    },
    unblock       : function (restrictions, handlerObject) {
        const _this = this;
        let proxyInfos = ProxyManager.getProxyInfos(restrictions);

        if (proxyInfos.length === 0) {
            _this.status = "error";
            _this.handleStatus(handlerObject);
            return;
        }

        proxyInfos = proxyInfos.slice(0);

        proxyInfos.sort(function () {
            return 0.5 - Math.random();
        });

        if (Config.videoInfo.proxiesToTry > 0) {
            proxyInfos = proxyInfos.slice(0, Config.videoInfo.proxiesToTry);
        }

        _this.status = "unblocking";
        _this.handleStatus(handlerObject);

        const networkErrors = [];

        async.eachSeries(
            proxyInfos,
            function (nsIProxyInfo, callback) {

                if (_this.status === "unblocked") {
                    return callback("done");
                }

                _this.proxyHost = nsIProxyInfo.host;

                VideoInfo.get(_this.id, function (error, status, errorCode) {

                    if (error) {
                        console.error(error);
                        if (errorCode === -1) {
                            networkErrors.push(nsIProxyInfo);
                        }
                        return callback(null);
                    }

                    if (status === VideoInfo.STATUS_OK) {
                        _this.status = "unblocked";
                        return callback("done");
                    }
                    else {
                        if (errorCode === -1) {
                            networkErrors.push(nsIProxyInfo);
                        }
                        return callback(null);
                    }
                });
            },
            function (success) {
                if (!success) {//passed all proxies with no result
                    _this.status = "blocked";
                    _this.proxyHost = null;

                    if (Config.videoInfo.networkFallback && networkErrors.length > 0) {
                        
                        _this.status = "unblocked";
                        _this.proxyHost = networkErrors[Math.floor(Math.random() * networkErrors.length)].host;
                    }

                }

                _this.handleStatus(handlerObject);
            }
        );
    }
});

exports.Video = Video;
