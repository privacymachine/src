// sdk
const { EventTarget } = require("sdk/event/target"),
    { Class }    = require("sdk/core/heritage"),
    { merge }    = require("sdk/util/object"),
    { PageMod }    = require("sdk/page-mod");

//addon

const { Template } = require("../service/template"),
    { Social } = require("../service/social");

const PageModAbstract = Class({
    extends        : EventTarget,
    pageMod        : null,
    workers        : [],
    initialize     : function initialize(options) {

        EventTarget.prototype.initialize.call(this, options);
        merge(this, options);
        this.workers = [];
        this.pagemod = null;

        this.setup();
    },
    setup          : function () {
    },
    install        : function () {


        if (this.pageMod) {

            return;
        }

        this.pageMod = PageMod(this.pageModOptions);

    },
    uninstall      : function () {


        // workers
        this.workers.forEach(function (worker) {
            try {
                //sending detach to content script to remove all items
                worker.port.emit("destroy");
                //when cleanup deatched is fired to destroy the worker
                worker.port.on("destroyed", () => {
                    worker.destroy();
                });
            } catch  (e) {
                //worker might already be gone, but make sure we uninstall properly
                worker.destroy();
            }
        });
        this.workers = [];

        if (this.pageMod) {
            try {
                this.pageMod.destroy();
                this.pageMod = null;
            } catch (error) {
                console.error("this.pageMod mus have been null", error);
            }
        }
    },
    installWorker  : function (worker) {


        // save the reference
        this.workers.push(worker);

        worker.on("detach", this.uninstallWorker.bind(this));
        Social.bindEventsToItem(worker.port);
        worker.port.on("template", function (templateName) {
            Template.getTemplate(templateName, function (name, content) {
                worker.port.emit("template", name, content);
            });
        });
    },
    uninstallWorker: function (worker) {


        const index = this.workers.indexOf(worker);
        if (index !== -1) {
            this.workers.splice(index, 1);
        }
    }
});

exports.PageModAbstract = PageModAbstract;
