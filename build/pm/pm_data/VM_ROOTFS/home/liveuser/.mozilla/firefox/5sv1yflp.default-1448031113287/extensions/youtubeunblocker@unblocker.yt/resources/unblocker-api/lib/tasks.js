// sdk
const { merge }     = require("sdk/util/object"),
    self = require("sdk/self");

// addon

const async = require("async");

const { Utils }     = require("./utils"),
    { Task }        = require("./task");

/**
 * User Task
 */
const taskUser = new Task({
    content     : {},
    onComplete  : function (response) {

    },
    buildContent: function (callback, content) {
        content = content || {};
        Utils.getCurrentUser(function (user) {
            merge(content, user);
            callback(null, content);
        });
    },

    // Api specific
    path    : "/user",
    interval: [0]
});

exports.taskUser = taskUser;

// create Config task
const taskConfig = new Task({
    files       : ["/data/config.json"],
    content     : {},
    onComplete  : function (response) {


        if (response.json) {
            for (let file of response.json) {
                if (file.local && file.remote) {
                    Utils.updateConfigFile(file.local, file.remote);
                }
            }
        }

    },
    buildContent: function (callback, content) {
        content = content || {};

        var _this = this;

        async.parallel(
            {
                md5: function (callback) {
                    const md5 = {};
                    let count = 0;
                    _this.files.map(function (file) {
                        Utils.md5FromDataUrl(file, function (err, result) {
                            count++;
                            md5[file] = result;
                            if (count == _this.files.length) {
                                callback(null, md5);
                            }
                        });
                    });
                }

            },
            function (err, result) {
                if (err) {

                }
                merge(content, result);
                callback(null, content);
            }
        );
    },
    // Api specific
    path        : "/config",
    interval    : [0, 1000 * 60 * 60 * 12]
});

exports.taskConfig = taskConfig;
