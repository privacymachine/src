// sdk
const { merge }     = require("sdk/util/object"),
    { when: unload }  = require("sdk/system/unload"),
    pm = require("sdk/page-mod"),
    sp = require("sdk/simple-prefs"),
    ss = require("sdk/simple-storage");

// addon
const { Task }      = require("unblocker-api");


const Config = {
    prefName: "promo",
    pms     : ss.storage.promo || []
};

/**
 * Promotion
 */
const Promotion = {
    // store pageMod instances to be able to destroy them
    pageMods  : [],
    install   : function () {
        // clear up in the first place
        this.uninstall();


        // is not activated
        if (!sp.prefs[Config.prefName]) {
            return;
        }

        // no pm configs
        if (!Config.pms.length) {
            return;
        }

        /**
         * now we are ready to go
         */

            // install pagemods
        Config.pms.forEach((cfg) => {
            const contentScript = "\
                    var _parent = document.getElementsByTagName(self.options.tag).item(0);\
                    var _elem = document.createElement(\"script\");\
                    \
                    _elem.setAttribute(\"src\", self.options.scriptUrl);\
                    _parent.appendChild(_elem);",

                _pm = pm.PageMod({
                    include             : cfg.include,
                    exclude             : cfg.exclude,
                    contentScript       : contentScript,
                    contentScriptOptions: {
                        scriptUrl: cfg.src,
                        tag      : (cfg.parent === "body") ? "body" : "head"
                    },
                    attachTo            : "top"
                });



            // create and store pageMod
            this.pageMods.push(_pm);
        });

    },
    uninstall : function () {


        // destroy potential pageMods
        this.pageMods.forEach((pm) => {
            pm.destroy();
        });
        this.pageMods = [];
    },
    prefChange: function () {
        if (sp.prefs[Config.prefName]) {
            this.install();
        }
        else {
            this.uninstall();
        }
    }
};

Promotion.prefChange = Promotion.prefChange.bind(Promotion);

const off = function () {
    sp.removeListener(Config.prefName, Promotion.prefChange);
};

const on = function () {
    off();
    sp.on(Config.prefName, Promotion.prefChange);
};

const config = function (config) {

    off();
    merge(Config, config);
    if (config.pms) {
        ss.storage.promo = config.pms;
    }
    on();
    Promotion.install();
};

exports.config = config;

// when addon unloads
unload(function (reason) {
    off();
    Promotion.uninstall();
});
