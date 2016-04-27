// sdk
const { Class } = require("sdk/core/heritage"),
    { merge } = require("sdk/util/object"),
    { AbstractManager } = require("./abstract");

const { Video } = require("../model/video");

const VideoManager = Class({
    extends   : AbstractManager,
    initialize: function initialize(options) {
        AbstractManager.prototype.initialize.call(this, options);
        merge(this, options);
        this.modelClass = Video;
    }
});

exports.VideoManager = new VideoManager();
