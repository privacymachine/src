// sdk
const { Cc, Ci }    = require("chrome"),
    { EventTarget }   = require("sdk/event/target"),
    { Class }         = require("sdk/core/heritage"),

    async = require("async"),

    { ProxyStatus } = require("../service/proxy-status");

// addon


// XPCOM
const nsIProtocolProxyService = Cc["@mozilla.org/network/protocol-proxy-service;1"]
    .getService(Ci.nsIProtocolProxyService);

const ProxyManager = Class({
        extends           : EventTarget,
        proxyInfos        : [],
        initialize        : function initialize(options) {
            EventTarget.prototype.initialize.call(this, options);
        },
        setProxies        : function (proxies) {
            const _this = this;

            _this.proxyInfos = [];

            ProxyStatus.start(this);

            async.each(
                proxies,
                function (proxy, callback) {
                    const nsIProxyInfo = {
                        proxyInfo: nsIProtocolProxyService.newProxyInfo("http", proxy.host, proxy.port, 0, 0, null),
                        country  : proxy.country,
                        auth     : proxy.auth ? proxy.auth : null,
                        hostV6   : proxy.hostV6,
                        fallback : proxy.fallback
                    };
                    _this.proxyInfos.push(nsIProxyInfo);
                    ProxyStatus.check(nsIProxyInfo, function (status) {
                        if (!status) {
                            _this.proxyInfos.splice(_this.proxyInfos.indexOf(nsIProxyInfo), 1);
                        }

                        callback();
                    });
                },
                function (err, result) {
                    ProxyStatus.stop();
                }
            );
        },
        getProxyInfos     : function (restrictions, noFallback) {
            if (!restrictions || !restrictions.countries) {
                restrictions = {
                    countries: [],
                    relation : "deny"
                };
            }

            let validProxies = [];



            for (let i = 0; i < this.proxyInfos.length; i++) {
                if (restrictions.relation === "deny") {
                    if (
                        restrictions.countries.indexOf(this.proxyInfos[i].country) === -1 ||
                            (this.proxyInfos[i].fallback && !noFallback)
                        ) {
                        validProxies.push(this.proxyInfos[i].proxyInfo);
                    }
                }
                else if (restrictions.relation === "allow") {
                    if (
                        restrictions.countries.indexOf(this.proxyInfos[i].country) > -1 ||
                            (this.proxyInfos[i].fallback && !noFallback)
                        ) {
                        validProxies.push(this.proxyInfos[i].proxyInfo);
                    }
                }
                else {
                    validProxies.push(this.proxyInfos[i].proxyInfo);
                }
            }



            return validProxies;
        },
        getProxyInfoByHost: function (proxyHost, getWrapped) {
            proxyHost = decodeURIComponent(proxyHost);

            for (let i = 0; i < this.proxyInfos.length; i++) {

                if (this.proxyInfos[i].proxyInfo.host === proxyHost || this.proxyInfos[i].hostV6 === proxyHost) {
                    if (getWrapped) {
                        return this.proxyInfos[i];
                    }
                    return this.proxyInfos[i].proxyInfo;
                }
            }
            return null;
        }
    })
    ;

exports.ProxyManager = new ProxyManager();
