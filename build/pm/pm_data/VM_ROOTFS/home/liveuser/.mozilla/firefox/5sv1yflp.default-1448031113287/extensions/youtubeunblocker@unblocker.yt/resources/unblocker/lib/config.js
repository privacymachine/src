// sdk
const self = require("sdk/self"),
    deepestMerge = require("deepest-merge"),
    fileIO = require("sdk/io/file"),
    Path = require('sdk/fs/path');

const {Cu, Ci, Cc} = require("chrome");
Cu.import("resource://gre/modules/AddonManager.jsm");
Cu.import("resource://gre/modules/FileUtils.jsm");
Cu.import("resource://gre/modules/NetUtil.jsm");

// addon


// default config object
let Config = {
    logLevel           : "info",
    Api                : {
        url: "http://api.unblocker.yt/"
    },
    "panel"            : {
        "width" : 480,
        "height": 350
    },
    "preferences"      : {
        "social_count": 0
    },
    "videoInfo"        : {
        "proxiesToTry"      : 8,
        "networkFallback"   : true,
        "url"               : "http://www.youtube.com/get_video_info?asv=3&hl=en_US&video_id={videoId}",
        "blockCheck"        : [
            {
                "field": "status",
                "value": "^fail$"
            },
            {
                "field": "errorcode",
                "value": "^150$"
            }
        ],
        "blockCheckNegative": [
            {
                "field": "reason",
                "value": "Playback\\+on\\+other\\+websites\\+has\\+been\\+disabled\\+by\\+the\\+video\\+owner\\."
            },
            {
                "field": "reason",
                "value": "It\\+is\\+restricted\\+from\\+playback\\+on\\+certain\\+sites\\."
            },
            {
                "field": "errorcode",
                "value": "^100"
            },
            {
                "field": "errorcode",
                "value": "^2"
            }
        ],
        "blockCheckValid"   : [
            {
                "field": "title",
                "value": "^.*$"
            },
            {
                "field": "author",
                "value": "^.*$"
            },
            {
                "field": "view_count",
                "value": "^.*$"
            },
            {
                "field": "length_seconds",
                "value": "^.*$"
            },
            {
                "field": "video_verticals",
                "value": "^.*$"
            }
        ]
    },
    "videoRestrictions": {
        "url"             : "http://gdata.youtube.com/feeds/api/videos/{videoId}?v=2&alt=jsonc",
        "data"            : "restrictions",
        "dataType"        : "type",
        "dataRelationship": "relationship"
    },
    "social"           : {
        "maxShowLarge": 5
    },
    "observer"         : {
        "ajax": {
            "uri"        : {
                "hosts": ["youtube.com"],
                "paths": ["spf=navigate", "spf=load"]
            },
            "headers"    : {
                "X-SPF-Response-Type": ".*"
            },
            "identifiers": {
                "\\/results"               : null,
                "watch.*[\\?&]v=([^\\?&]+)": 1
            }
        }
    },
    "pagemod"          : {
        "facebook": {
            "include"          : "https?://[\\s\\S]*.youtube.com\\/embed.*",
            "activeRegexp"     : "\\/\\/(www\\.)?youtube\\.com\\/(embed\\/|v\\/|apiplayer)([\\?&]video_id=)?([^\\?\\/&]+)",
            "inactiveRegexp"   : "blocked=true",
            "activeRegexpGroup": 4
        },
        "websites": {
            "include"          : "^https?://((?!youtube.com)[\\s\\S])*$",
            "exclude"          : ".*\\.xml((\\?|#)[\\s\\S]*)?$",
            "activeRegexp"     : "\\/\\/(www\\.)?youtube\\.com\\/(embed\\/|v\\/|apiplayer)([\\?&]video_id=)?([^\\?\\/&]+)",
            "activeRegexpGroup": 4
        },
        "video"   : {
            "include"          : "https?://.*.youtube.com\\/watch.*([\\?&]v=.*)",
            "activeRegexp"     : "^.*[\\?&]v=(.*?)([#&].*){0,}$",
            "activeRegexpGroup": 1,
            "options"          : {
                "htmlSelectors": {
                    "player" : ["#player"],
                    "content": ["#watch7-content"]
                }
            }
        },
        "search"  : {
            "include"          : "https?://.*\\.youtube.com\\/results.*[\\?&](search_query|q)=.*",
            "activeRegexp"     : "^.*[\\?&](search_query|q)=(.*?)([#&].*){0,}$",
            "activeRegexpGroup": 2,
            "options"          : {
                "htmlSelectors": {
                    "filter": ["#gh-activityfeed", ".branded-page-v2-body", ".search-header"]
                }
            }
        },
        "embed"   : {
            "include"          : "https?://[\\s\\S]*.youtube.com\\/embed\\/[\\S\\s]*blocked=true[\\S\\s]*",
            "activeRegexp"     : "\\/\\/(www\\.)?youtube\\.com\\/(embed\\/|v\\/|apiplayer)([\\?&]video_id=){0,1}([^#\\?\\/&]+)",
            "activeRegexpGroup": 4,
            "options"          : {
                "htmlSelectors": {
                    "wrapper": ["body"]
                }
            }
        }
    },
    "RequestFilter"    : {
        "rulesetLanguage"    : {
            "pages"  : ["youtube", "website"],
            "host"   : {
                "^(.*\\.)youtube\\.com$"    : true,
                "^(.*\\.)googlevideo\\.com$": true
            },
            "path"   : {
                "^\\/get_video_info": true
            },
            "headers": {
                "Accept-Language": "en-US,en"
            }
        },
        "rulesetRefererEmbed": {
            "pages"  : ["website"],
            "host"   : {
                "youtube\\.com"    : true,
                "googlevideo\\.com": true
            },
            "path"   : {
                "^\\/embed\\/"      : true,
                "^\\/v\\/"          : true,
                "^\\/apiplayer\\/"  : true,
                "^\\/get_video_info": true
            },
            "headers": {
                "Referer": "https://www.youtube.com"
            }
        }
    },
    "ProxyFilter"      : {
        "rulesetWatchEmbedGetVideoInfo": {
            "resolveMethod": "resolveByVideoId",
            "resolveParams": ["path.id"],
            "host"         : {
                "^(www\\.)youtube\\.com$": true
            },
            "path"         : {
                "^\\/watch.*[\\?&]v=([^\\?\\/&#]*)([\\S\\s]*){0,}$"                      : {
                    "1": "id"
                },
                "^\\/embed\\/([^\\?\\/&#]*)"                                             : {
                    "1": "id"
                },
                "^\\/get_video_info\\?([\\S\\s]*)(video_id=([^\\?\\/&#]+))($|&[\\S\\s]*)": {
                    "3": "id"
                }
            }
        },
        "rulesetSearch"                : {
            "resolveMethod": "resolveBySearchquery",
            "resolveParams": ["query.search"],
            "host"         : {
                "^(.*\\.)youtube\\.com$": true
            },
            "path"         : {
                "^\\/results": true
            },
            "query"        : {
                "search_query=([^&#]+)": {
                    "1": "search"
                }
            }
        },
        "rulesetVideoplayback"         : {
            "resolveMethod": "resolveByProxyIp",
            "resolveParams": ["query.ip"],
            "host"         : {
                "^(.*\\.)googlevideo\\.com$": true
            },
            "path"         : {
                "^\\/videoplayback": true
            },
            "query"        : {
                "&ip=(.*?)&"      : {
                    "1": "ip"
                },
                "\\/ip\\/(.*?)\\/": {
                    "1": "ip"
                }
            }
        }
    },
    "proxyStatus"      : {
        "doCheck"    : true,
        "url"        : "http://s3-eu-west-1.amazonaws.com/youtube-unblocker/prox_check.txt?check=__IP__",
        "matchString": "Y"
    }
};

// export config object
module.exports = {
    getConfig           : function () {
        return Config;
    },
    mergeConfig         : function (next) {
        try {
            AddonManager.getAddonByID(self.id, (addon) => {
                addon.getDataDirectory((directory) => {
                    if (directory) {
                        try {
                            let dataConfig = fileIO.read(Path.join(directory, "data" + Path.sep + "config.json"), "r");
                            if (!dataConfig.match(/^\s*\{[\s\S]*\}\s*$/igm)) {
                                dataConfig = "{}";
                            }

                            dataConfig = JSON.parse(dataConfig);
                            Config = deepestMerge(Config, dataConfig);
                        } catch (e) {

                        }

                        return this.mergePermanentValues(next);
                    }
                })
            });
        } catch (error) {

            return this.mergePermanentValues(next);
        }
    },
    mergePermanentValues: function (next) {
        //Permanent values -- DO NOT REMOVE THIS COMMENT!
        Config = deepestMerge(Config, {"campaignId":1,"Api":{"urlBackup":"http://api.unblocker.yt/"}});

        // set logLevel



        return next();
    },
    initConfig          : function (loadReason, next) {
        if (loadReason === "install" || loadReason === "upgrade") {
            AddonManager.getAddonByID(self.id, function (addon) {
                addon.getDataDirectory(function (directory) {
                    if (directory) {

                        const files = ["config.json", "proxies.json", "campaign_background.png"];

                        fileIO.mkpath(Path.join(directory, "data" + Path.sep));
                        var count = 0;
                        for (var file of files) {
                            (function (file) {
                                NetUtil.asyncFetch(
                                    self.data.url("./" + file),
                                    function (nsIInputStream) {
                                        const nsIFile = FileUtils.File(Path.join(directory, "data" + Path.sep + file));
                                        const ostream = Cc["@mozilla.org/network/file-output-stream;1"].
                                            createInstance(Ci.nsIFileOutputStream);
                                        ostream.init(nsIFile, -1, -1, Ci.nsIFileOutputStream.DEFER_OPEN);

                                        NetUtil.asyncCopy(
                                            nsIInputStream,
                                            ostream,
                                            function () {
                                                count++;
                                                if (count === files.length) {
                                                    next();
                                                }


                                            }
                                        );
                                    }
                                );
                            })(file);
                        }

                    }
                });
            });
        }
        else {
            return next();
        }
    }
};
