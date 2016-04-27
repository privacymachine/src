// sdk
const { Class } = require("sdk/core/heritage"),
    { merge } = require("sdk/util/object");

//addon
const { AbstractManager } = require("./abstract"),
    { Search } = require("../model/search");

const SearchManager = Class({
    extends   : AbstractManager,
    initialize: function initialize(options) {
        AbstractManager.prototype.initialize.call(this, options);
        merge(this, options);
        this.modelClass = Search;
    }
});

exports.SearchManager = new SearchManager();
