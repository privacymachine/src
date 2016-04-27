// sdk
const { Request } = require("sdk/request"),
    { Class } = require("sdk/core/heritage");

//addon
const Config = require("../config").getConfig();

const VideoRestriction = Class({

    get: function (videoId, callback) {
        Request({
            url       : Config.videoRestrictions.url.replace("{videoId}", videoId),
            onComplete: (response) => {

                if (!response.json.data) {
                    return callback(null, {});
                }

                const restrictions = response.json.data[Config.videoRestrictions.data];
                let callbacked = false;

                if (restrictions) {
                    restrictions.forEach(function (restriction) {
                        const type = restriction[Config.videoRestrictions.dataType],
                            relation = restriction[Config.videoRestrictions.dataRelationship];

                        if (type === "country" && (relation === "deny" || relation === "allow")) {
                            callbacked = true;
                            return callback(null, {
                                "countries": restriction.countries.toLowerCase().split(" "),
                                "relation" : relation
                            });
                        }

                    });
                }
                if (!callbacked) {
                    callback(null, {});
                }
            }
        }).get();
    }

});

exports.VideoRestriction = new VideoRestriction();
