// sdk
const { Class }    = require("sdk/core/heritage"),
    PageWorker = require("sdk/page-worker").Page,
    self = require("sdk/self");

const Template = Class({
    workers    : [],
    getTemplate: function (file, callback) {
        // we load the l10n'ed html from a pageWorker
        const pageWorker = PageWorker({
            contentScriptFile: self.data.url("pageworker/template.js"),
            contentURL       : self.data.url("templates/" + file)
        });

        //initialize translate
        pageWorker.port.emit('translate', file);
        //get translated data
        pageWorker.port.on("html_translated", function (file, template) {
            callback(file, template);
        });
        this.workers.push(pageWorker);
    },
    destroy    : function () {
        for (let worker in this.workers) {
            this.workers[worker].destroy();
        }
    }
});

exports.Template = new Template();
