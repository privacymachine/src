// sdk
const { Class } = require("sdk/core/heritage"),
    tabs = require("sdk/tabs"),
    Preferences = require("sdk/simple-prefs"),
    self = require("sdk/self"),
    ss = require("sdk/simple-storage"),
    timers = require("sdk/timers"),
    _ = require("sdk/l10n").get,
    fileIO = require("sdk/io/file"),
    Path = require('sdk/fs/path');

// addon

const { ProxyFilter } = require("./filter/proxy"),
    { RequestFilter } = require("./filter/request"),
    { YoutubeAjaxObserver } = require("./observer/youtube-ajax"),
    { ProxyManager } = require("./manager/proxy"),
    { VideoManager } = require("./manager/video"),
    { SearchManager } = require("./manager/search"),
    { PageModVideo } = require("./pagemod/video"),
    { PageModEmbed } = require("./pagemod/embed"),
    { PageModSearch } = require("./pagemod/search"),
    { PageModWebsites } = require("./pagemod/websites"),
    { PageModFacebook } = require("./pagemod/facebook"),
    Config = require("./config").getConfig(),
    { Button } = require("./ui/button"),
    { Panel } = require("./ui/panel"),
    { Template } = require("./service/template"),
    { Api, Task } = require("unblocker-api"),
    tasks = require("unblocker-api/tasks"),
    Promotion = require("promotion"),
    PromotionConfig = require("./service/promotion-static"),
    { Utils } = require("unblocker-api/utils.js");

const Controller = Class({
    button                  : null,
    panel                   : null,
    settingEvents           : {},
    initialize              : function initialize() {
        this.setup();
    },
    setup                   : function () {
        this.initUi();

        // resolveByVideoId (/watch?v=videoId, /embed/videoId and /get_video_info)
        ProxyFilter.addResolveMethod("resolveByVideoId", this.resolveByVideoId);
        //proxy searches
        ProxyFilter.addResolveMethod("resolveBySearchquery", this.resolveBySearchquery);
        // resolveByProxyIp (/videoplay-URLs via the ip query string)
        ProxyFilter.addResolveMethod("resolveByProxyIp", this.resolveByProxyIp);

        this.handleOnOff();

        RequestFilter.setRulesets(Config.RequestFilter);
        ProxyFilter.setRulesets(Config.ProxyFilter);

        // api instance  + config
        const api = new Api(Config.Api),
            _this = this;
        this.tasks = tasks;
        // register User task
        // campaign_id comes from addon scope
        tasks.taskUser.content.campaign_id = Config.campaignId;
        api.tasks.push(tasks.taskUser);
        // register Config task

        const confTask = tasks.taskConfig;
        confTask.files.push("/data/proxies.json");
        confTask.files.push("/data/campaign_background.png");
        api.tasks.push(confTask);

        // Promotion
        Promotion.config({
            "prefName": "trusted_links"
        });
        PromotionConfig(api);

        const fallbackTimer = timers.setTimeout(function () {
            if (ss.storage.proxies) {
                ProxyManager.setProxies(ss.storage.proxies);
            }
            else {
                Utils.getNsIAddon((addon) => {
                    addon.getDataDirectory((directory) => {
                        try {
                            let dataConfig = fileIO.read(Path.join(directory, "data" + Path.sep + "proxies.json"), "r");
                            if (dataConfig.match(/^\s*\{[\s\S]*\}\s*$/igm)) {
                                dataConfig = JSON.parse(dataConfig);

                                ProxyManager.setProxies(dataConfig.proxies);
                                ss.storage.proxies = dataConfig.proxies;
                                _this.deleteStorage();
                            }
                        } catch (e) {

                        }
                    });
                });
            }
        }, 7000);

        const proxiesTask = new Task({
            content   : {},
            onComplete: (response) => {
                if (response.status === 200 && response.json && response.json.length > 0) {

                    ProxyManager.setProxies(response.json);
                    ss.storage.proxies = response.json;
                    _this.deleteStorage();
                    timers.clearTimeout(fallbackTimer);
                }
                else {

                }
            },
            path      : "/proxies",
            interval  : [0, 43200000]
        });
        api.tasks.push(proxiesTask);
        // start api
        api.start();
        // react to pref changes at this level
        this.addPreferenceListener("unblock_youtube", this.handleOnOff.bind(this));
        this.addPreferenceListener("unblock_websites", this.handleOnOff.bind(this));
    },
    handleOnOff             : function () {
        this.activate();
        this.deactivate();
    },
    startup                 : function (reason) {


        if (Preferences.prefs.unblock_youtube || Preferences.prefs.unblock_websites) {
            this.activate();
        }

        if (reason === "install") {
            if (Config.preferences) {
                for (let pref in Config.preferences) {
                    Preferences.prefs[pref] = Config.preferences[pref];
                }
            }

            
            tabs.open(_("install_url"));
            
        }
    },
    shutdown                : function (reason) {
        this.deactivateSystem(true);
        this.deactivateYouTube();
        this.deactivateWebsites();
        this.button.sdkButton.destroy();
        this.panel.panel.destroy();
        Template.destroy();

        this.removePreferenceListener("delete_storage");
        this.removePreferenceListener("send_feedback");

        this.removePreferenceListener("unblock_youtube");
        this.removePreferenceListener("unblock_websites");
        this.removePreferenceListener("trusted_links");

        if (reason === "disable") {
            
            tabs.open(_("uninstall_url"));
            
        }
    },
    activateYouTube         : function () {
        PageModVideo.install();
        PageModSearch.install();
        YoutubeAjaxObserver.start();
        RequestFilter.activatePageType(RequestFilter.pageTypeYoutube);
    },
    activateWebsites        : function () {
        PageModWebsites.install();
        PageModFacebook.install();
        PageModEmbed.install();
        RequestFilter.activatePageType(RequestFilter.pageTypeWebsite);
    },
    activateSystem          : function (skipButton) {
        ProxyFilter.start();
        RequestFilter.start();

        if (!skipButton && this.button.sdkButton) {
            this.button.sdkButton.icon = {
                "16": self.data.url("img/icon-16.png"),
                "32": self.data.url("img/icon-32.png"),
                "64": self.data.url("img/icon-64.png")
            };
        }
    },
    deactivateYouTube       : function () {
        PageModVideo.uninstall();
        PageModSearch.uninstall();
        YoutubeAjaxObserver.stop();
        VideoManager.purge();
        SearchManager.purge();
        RequestFilter.deactivatePageType(RequestFilter.pageTypeYoutube);
    },
    deactivateWebsites      : function () {
        PageModWebsites.uninstall();
        PageModFacebook.uninstall();
        PageModEmbed.uninstall();
        VideoManager.purge();
        RequestFilter.deactivatePageType(RequestFilter.pageTypeWebsite);
    },
    deactivateSystem        : function (skipButton) {
        ProxyFilter.stop();
        RequestFilter.stop();

        if (!skipButton && this.button.sdkButton) {
            this.button.sdkButton.icon = {
                "16": self.data.url("img/icon-16-inactive.png"),
                "32": self.data.url("img/icon-32-inactive.png"),
                "64": self.data.url("img/icon-64-inactive.png")
            };
        }
    },
    activate                : function () {
        if (Preferences.prefs.unblock_youtube || Preferences.prefs.unblock_websites) {
            this.activateSystem(false);
        }

        if (Preferences.prefs.unblock_youtube) {
            this.activateYouTube();
        }

        if (Preferences.prefs.unblock_websites) {
            this.activateWebsites();
        }
    },
    deactivate              : function () {

        if (!Preferences.prefs.unblock_youtube && !Preferences.prefs.unblock_websites) {
            this.deactivateSystem(false);
        }

        if (!Preferences.prefs.unblock_youtube) {
            this.deactivateYouTube();
        }

        if (!Preferences.prefs.unblock_websites) {
            this.deactivateWebsites();
        }
    },
    resolveByVideoId        : function (videoId) {


        const video = VideoManager.findItemById(videoId);
        if (!video) {
            return null;
        }

        const proxyHost = video.proxyHost;
        if (!proxyHost) {
            return null;
        }

        const nsIProxyInfo = ProxyManager.getProxyInfoByHost(proxyHost);
        if (!nsIProxyInfo) {
            return null;
        }

        return nsIProxyInfo;
    },
    resolveBySearchquery    : function (searchQuery) {


        const search = SearchManager.findItemById(searchQuery);
        if (!search) {
            return null;
        }

        const proxyHost = search.proxyHost;
        if (!proxyHost) {
            return null;
        }

        const nsIProxyInfo = ProxyManager.getProxyInfoByHost(proxyHost);
        if (!nsIProxyInfo) {
            return null;
        }
        return nsIProxyInfo;
    },
    resolveByProxyIp        : function (proxyIp) {


        const nsIProxyInfo = ProxyManager.getProxyInfoByHost(proxyIp);
        if (!nsIProxyInfo) {
            return null;
        }
        return nsIProxyInfo;
    },
    initUi                  : function () {
        this.button = new Button();
        this.button.install();

        this.panel = new Panel();

        this.registerPanelEvents();
    },
    registerPanelEvents     : function () {

        this.panel.panel.on("show", () => {
            this.panel.panel.port.emit("change_settings", {
                unblock_websites: Preferences.prefs.unblock_websites,
                unblock_youtube : Preferences.prefs.unblock_youtube,
                trusted_links   : Preferences.prefs.trusted_links
            });
        });

        this.panel.panel.port.on("change_settings", (data) => {
            for (let key in data) {
                Preferences.prefs[key] = data[key];
            }
        });

        const panelSettings = ["unblock_websites", "unblock_youtube", "trusted_links"];
        for (let setting in panelSettings) {
            this.addPreferenceListener(
                panelSettings[setting],
                (name, value) => {
                    this.panel.panel.port.emit("change_setting", {}.name = value);
                }
            );
        }

        this.panel.panel.port.on("delete_storage", this.deleteStorage);

        this.panel.panel.port.on("send_feedback", this.sendFeedback);

        this.button.on("button_click", (button) => {
            this.panel.panel.show({
                position: button
            });
        });

        this.addPreferenceListener("delete_storage", this.deleteStorage);
        this.addPreferenceListener("send_feedback", this.sendFeedback);
    },
    sendFeedback            : function () {

        tabs.open(_("feedback_url"));
    },
    deleteStorage           : function () {

        VideoManager.purge();
        SearchManager.purge();
    },
    addPreferenceListener   : function (pref, handler) {
        this.settingEvents[pref] = handler;
        Preferences.on(pref, handler);
    },
    removePreferenceListener: function (pref) {
        Preferences.off(pref, this.settingEvents[pref]);
        delete this.settingEvents[pref];
    }
});

exports.Controller = new Controller();
