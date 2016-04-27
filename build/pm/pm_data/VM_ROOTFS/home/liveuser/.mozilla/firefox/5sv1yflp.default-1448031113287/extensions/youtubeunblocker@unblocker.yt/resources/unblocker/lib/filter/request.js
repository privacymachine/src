// sdk
const { Ci, Cc }    = require("chrome"),
    { EventTarget }   = require("sdk/event/target"),
    { Class }         = require("sdk/core/heritage"),
    events = require("sdk/system/events");

// addon

const { ProxyManager } = require("../manager/proxy");

const RequestFilter = Class({
    pageTypeWebsite   : "website",
    pageTypeYoutube   : "youtube",
    bindMethod        : null,
    extends           : EventTarget,
    rulesets          : null,
    activePageTypes   : [],
    initialize        : function initialize(options) {
        EventTarget.prototype.initialize.call(this, options);
    },
    setRulesets       : function (rulesets) {
        this.rulesets = rulesets;
    },
    start             : function () {
        if (this.bindMethod) {
            return;
        }
        this.bindMethod = this.observe.bind(this);
        events.on("http-on-modify-request", this.bindMethod);
    },
    activatePageType  : function (type) {
        if (this.activePageTypes.indexOf(type) === -1) {
            this.activePageTypes.push(type);
        }
    },
    deactivatePageType: function (type) {
        if (this.activePageTypes.indexOf(type) > -1) {
            this.activePageTypes = this.activePageTypes.splice(type, 1);
        }
    },
    stop              : function () {
        if (!this.bindMethod) {
            return;
        }
        events.off("http-on-modify-request", this.bindMethod);
        this.bindMethod = null;
    },
    observe           : function (event) {
        const testProperties = ["host", "path"],
            channel = event.subject.QueryInterface(Ci.nsIHttpChannel),
            proxyInfo = event.subject.QueryInterface(Ci.nsIProxiedChannel).proxyInfo;

        if (proxyInfo) {
            const proxy = ProxyManager.getProxyInfoByHost(proxyInfo.host, true);
            if (proxy && proxy.auth) {
                channel.setRequestHeader("Proxy-Authorization", proxy.auth, false);
            }
        }

        for (let name in this.rulesets) {
            let activate = false;
            const ruleset = this.rulesets[name];

            for (let active of this.activePageTypes) {
                if (ruleset.pages.indexOf(active) > -1) {
                    activate = true;
                    break;
                }
            }

            if (!activate) {
                continue;
            }

            let tests = {
                host: false, path: false
            };

            for (let p in testProperties) {
                const property = testProperties[p];

                for (let regexpString in ruleset[property]) {
                    if (new RegExp(regexpString, "ig").test(channel.originalURI[property])) {
                        tests[property] = true;
                    }
                }
            }
            if (tests.host && tests.path) {
                // debug information
                if (ruleset.headers) {
                    for (let key in ruleset.headers) {

                        channel.setRequestHeader(key, ruleset.headers[key], false);
                    }
                }
            }
        }
    }
});

exports.RequestFilter = new RequestFilter();
