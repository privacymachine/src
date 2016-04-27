// sdk
const { Class } = require("sdk/core/heritage"),
    { Cc, Ci } = require("chrome");

//addon
const Config = require("../config").getConfig();



// XPCOM
const nsIProtocolProxyService = Cc["@mozilla.org/network/protocol-proxy-service;1"].
    getService(Ci.nsIProtocolProxyService);

const ProxyStatus = Class({
    READY_STATE: 4,
    initialize : function () {
        this.bindMethod = null;
    },
    start      : function (proxyManager) {
        if (this.bindMethod) {
            return;
        }
        this.proxyManager = proxyManager;
        this.bindMethod = this;
        nsIProtocolProxyService.registerFilter(this.bindMethod, 0);
    },
    stop       : function () {
        if (!this.bindMethod) {
            return;
        }
        nsIProtocolProxyService.unregisterFilter(this.bindMethod);
    },
    applyFilter: function (nsIProtocolProxyService, nsIURI, nsIProxyInfo) {
        const currentUrl = nsIURI.scheme + "://" + nsIURI.host + nsIURI.path;
        if (currentUrl.indexOf(Config.proxyStatus.url.replace("__IP__", "")) > -1) {

            const hostname = currentUrl.replace(
                Config.proxyStatus.url.replace("__IP__", ""),
                ""
            );
            return this.proxyManager.getProxyInfoByHost(hostname);
        }
        return null;
    },
    check      : function (proxyInfo, callback) {

        if (!Config.proxyStatus.doCheck) {
            return callback(true);
        }

        let request,
            timedOut = false;
        if (typeof XMLHttpRequest !== "undefined") {
            request = new XMLHttpRequest();
        }
        else {
            request = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance(Ci.nsIXMLHttpRequest);
        }

        request.open("GET", Config.proxyStatus.url.replace("__IP__", proxyInfo.proxyInfo.host), true);
        request.timeout = 3000;
        request.onreadystatechange = () => {
            // ready?
            if (request.readyState === this.READY_STATE && !timedOut) {

                const status = (
                    request.status === 200 &&
                    request.responseText.indexOf(Config.proxyStatus.matchString) > -1
                );

                callback(status);
            }
        };
        request.send(null);
    }
});

exports.ProxyStatus = new ProxyStatus();
