// sdk
const { Cc, Ci }    = require("chrome"),
    { EventTarget }   = require("sdk/event/target"),
    { Class }         = require("sdk/core/heritage"),
    { merge }         = require("sdk/util/object"),
    querystring = require("sdk/querystring");

// addon


// xpcom
let nsIProtocolProxyService;
if (Ci.nsIProtocolProxyService2) {
    nsIProtocolProxyService = Cc["@mozilla.org/network/protocol-proxy-service;1"].
        getService(Ci.nsIProtocolProxyService2);
}
else {
    nsIProtocolProxyService = Cc["@mozilla.org/network/protocol-proxy-service;1"].
        getService(Ci.nsIProtocolProxyService);
}

const ProxyFilter = Class({
    extends         : EventTarget,
    bindMethod      : null,
    rulesets        : null,
    resolveMethods  : {},
    initialize      : function initialize(options) {
        EventTarget.prototype.initialize.call(this, options);
    },
    setRulesets     : function (rulesets) {
        this.rulesets = rulesets;
    },
    addResolveMethod: function (name, fnResolve) {
        this.resolveMethods[name] = fnResolve;
    },
    start           : function () {

        if (this.bindMethod) {
            return;
        }
        this.bindMethod = this;

        //use the new channel logic
        if (Ci.nsIProtocolProxyService2 && nsIProtocolProxyService.registerChannelFilter) {
            nsIProtocolProxyService.registerChannelFilter(this.bindMethod, 0);
        }
        else {
            nsIProtocolProxyService.registerFilter(this.bindMethod, 0);
        }
    },
    stop            : function () {

        if (!this.bindMethod) {
            return;
        }

        if (Ci.nsIProtocolProxyService2 && nsIProtocolProxyService.unregisterChannelFilter) {
            nsIProtocolProxyService.unregisterChannelFilter(this.bindMethod);
        }
        else {
            nsIProtocolProxyService.unregisterFilter(this.bindMethod);
        }

        this.bindMethod = null;
    },
    applyFilter     : function (nsIProtocolProxyService, nsIURI, nsIProxyInfo) {
        if (nsIURI instanceof Ci.nsIChannel) {//we got a channel here
            nsIURI = nsIURI.URI;
        }

        const testProperties = ["host", "path"];

        for (let name in this.rulesets) {
            const ruleset = this.rulesets[name];
            let tests = {
                host: false,
                path: false
            };

            for (let p in testProperties) {
                const property = testProperties[p];

                for (let regexpString in ruleset[property]) {
                    if (new RegExp(regexpString, "i").test(nsIURI[property])) {
                        tests[property] = true;
                    }
                }
            }

            if (tests.host && tests.path) {
                const nsIProxyInfoResolved = this.detailedFilter(nsIURI, ruleset);

                return nsIProxyInfoResolved;
            }
        }

        return nsIProxyInfo;
    },
    detailedFilter  : function (nsIURI, ruleset) {

        try {
            const nsIURL = nsIURI.QueryInterface(Ci.nsIURL);
            const urlDO = {
                    host    : nsIURL.host,
                    path    : nsIURL.path,
                    filePath: nsIURL.filePath,
                    query   : nsIURL.query,
                    params  : querystring.parse(nsIURL.query)
                },
                keys = Object.keys(urlDO);



            let varsDO = {};

            for (let i = 0; i < keys.length; i++) {
                const key = keys[i],
                    rules = ruleset[keys[i]];
                if (rules) {
                    varsDO[key] = {};
                    const vars = this.rules(key, urlDO[key], rules);
                    if (vars) {
                        merge(varsDO[key], vars);
                    }
                    else {

                        return null;
                    }
                }
            }



            const fnResolve = this.resolveMethods[ruleset.resolveMethod];
            let fnArgs = [];

            ruleset.resolveParams.forEach(function (path) {
                path = path.split(".");
                let ref = varsDO;
                while (path.length) {
                    ref = ref[path.shift()];
                }
                fnArgs.push(ref);
            });



            const nsIProxyInfo = fnResolve.apply(this, fnArgs);

            return nsIProxyInfo;
        } catch (error) {
            console.error(error);
            return null;
        }
    },
    rules           : function (key, string, rules) {

        for (let regexString in rules) {
            const r = new RegExp(regexString, "i"),
                result = string.match(r);



            if (result) {
                let data = {};
                result.forEach((value, index) => {
                    if (rules[regexString][index]) {
                        data[rules[regexString][index]] = value;
                    }
                });
                return data;
            }
        }
        return false;
    }
});

exports.ProxyFilter = new ProxyFilter();
