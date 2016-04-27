
//addon
const { ProxyManager }  = require("../manager/proxy"),
//sdk
    { Class } = require("sdk/core/heritage");

const Search = Class({
    id        : null,
    status    : "disabled",
    proxyHost : null,
    initialize: function (id) {
        this.id = id;
        this.proxyHost = null;
    },
    handle    : function (handlerObject) {
        switch (this.status) {
            case "unblock":
                this.status = "unblocking";
                this.handleStatus(handlerObject);
                this.applyProxy(handlerObject);
                break;
            case "disable":
                this.proxyHost = null;
                this.status = "disabled";
                this.handleStatus(handlerObject);
                break;
            case "error":
            case "disabled":
            case "unblocked":
                this.handleStatus(handlerObject);
                break;

            default:
                console.error("oops, unknown status", this);
        }
    },

    handleStatus: function (handlerObject) {

        if (handlerObject[this.status]) {
            handlerObject[this.status]();
        }
        else {
            console.error("no handler", this.status, "in handlerObject");
        }
    },

    applyProxy: function (handlerObject) {



        let restrictions = false,
            randomProxy = false;

        if (this.country) {
            restrictions = {
                countries: [this.country],
                relation : "allow"
            };
        }

        const proxyInfos = ProxyManager.getProxyInfos(restrictions, true);

        if (proxyInfos.length) {
            randomProxy = proxyInfos[Math.floor(Math.random() * proxyInfos.length)];
        }



        if (randomProxy) {
            //just take the first proxy due to we cannot estimate if its unblocked or not
            this.proxyHost = randomProxy.host;
            this.status = "unblocked";
            this.handleStatus(handlerObject);
        }
        else {
            this.proxyHost = null;
            this.status = "error";
            this.handleStatus(handlerObject);
        }
    }
});

exports.Search = Search;
