// sdk
const { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object"),
    self = require("sdk/self");

//addon

const { SearchManager }= require("../manager/search"),
    { PageModAbstract } = require("./abstract"),
    { ProxyManager } = require("../manager/proxy"),
    Config = require("../config").getConfig();

const PageModSearch = Class({
    extends      : PageModAbstract,
    initialize   : function initialize(options) {
        PageModAbstract.prototype.initialize.call(this, options);
        merge(this, options);

        this.setup();

        this.pageModOptions = {
            include          : new RegExp(Config.pagemod.search.include),
            attachTo         : ["existing", "top"],
            attachWhen       : "start",
            onAttach         : this.installWorker.bind(this),
            contentScriptFile: [
                self.data.url("js/jquery.js"),
                self.data.url("pagemod/search.js")
            ],
            contentStyleFile : [
                self.data.url("css/search.css"),
                self.data.url("css/social.css")
            ],
            contentScriptOptions: Config.pagemod.search.options
        };
    },
    installWorker: function (worker) {
        const identifierMatch = worker.tab.url.match(
            new RegExp(Config.pagemod.search.activeRegexp)
        );

        if (!identifierMatch || !identifierMatch[Config.pagemod.search.activeRegexpGroup]) {
            return;
        }

        const identifier = identifierMatch[Config.pagemod.search.activeRegexpGroup];


        PageModAbstract.prototype.installWorker.call(this, worker);
        worker.port.emit("identifier", identifier);

        const countries = [];
        ProxyManager.proxyInfos.forEach((proxyInfo) => {
            if (countries.indexOf(proxyInfo.country) < 0) {
                countries.push(proxyInfo.country);
            }
        });



        worker.port.on("status", function (identifier) {
            const search = SearchManager.findOrCreateItemById(identifier);

            worker.port.emit(
                "status",
                {
                    "countries": countries,
                    "country"  : search.country,
                    "status"   : search.status
                }
            );
        });
        worker.port.on("start", function (countryCode) {



            const search = SearchManager.findOrCreateItemById(identifier);

            if (countryCode) {
                search.country = countryCode;
            }

            search.status = "unblock";

            search.handle({
                "unblocked" : function () {

                    worker.port.emit("unblocked");
                },
                "unblocking": function () {

                    worker.port.emit("unblocking");
                },
                "disabled"  : function () {

                    worker.port.emit("disabled");
                },
                "error"     : function () {

                    worker.port.emit("error");
                }
            });
        });

        worker.port.on("stop", function () {
            const search = SearchManager.findOrCreateItemById(identifier);
            search.status = "disable";
            search.handle({});
            worker.port.emit("disabled");
        });
    }
});

exports.PageModSearch = new PageModSearch();
