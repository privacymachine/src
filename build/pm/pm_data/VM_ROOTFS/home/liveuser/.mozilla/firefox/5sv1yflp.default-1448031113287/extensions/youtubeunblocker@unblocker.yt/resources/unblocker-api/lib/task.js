// sdk
const { Class }     = require("sdk/core/heritage"),
    { merge }       = require("sdk/util/object"),
    request         = require("sdk/request").Request;

// addon
const async         = require("async");


const Task = Class({
    url       : null,
    headers   : null,
    content   : null,
    onComplete: null, // (response) => {}

    buildUrl    : null, // (callback, url) => {}
    buildHeaders: null, // (callback, headers) => {}
    buildContent: null, // (callback, content) => {}

    initialize: function (options) {
        merge(this, options);
    },

    // Api needs these
    path      : null,
    interval  : []
});

Task.createRequestObject = function () {

    let tasks = Array.prototype.slice.call(arguments, 0);
    const callback = tasks.pop();




    // build object being given to sdk/request
    let r = {};

    // for properties
    for (let task of tasks) {
        let copy = JSON.parse(JSON.stringify(task));
        for (let p in copy) {
            // only keep these
            if ([
                "url",
                "headers",
                "content"
            ].indexOf(p) < 0) {
                delete copy[p];
            }
        }
        merge(r, copy);
    }

    let tasksReverse = tasks.reverse();

    // onComplete method
    for (let task of tasksReverse) {
        if (task.onComplete) {
            r.onComplete = task.onComplete;
            break;
        }
    }

    if (!r.onComplete) {

    }

    const builders = {
        url    : null,
        headers: null,
        content: null
    };

    const buildBuilderFn = function (fn, scope) {
        return function (callback, value) {
            fn.call(scope, callback, value);
        };
    };

    for (let property in builders) {

        // buildMethod
        let buildMethod = "build" + property.charAt(0).toUpperCase() + property.slice(1);

        // store callback for the first buildMethod into builders
        for (let task of tasksReverse) {

            if (task[buildMethod]) {


                let fn = task[buildMethod],
                    scope = task;
                // store it
                builders[property] = buildBuilderFn(fn, scope);
                break;
            }
        }
    }

    const buildParallelFn = function (fn, property) {
        return function (callback) {
            fn(callback, r[property]);
        };
    };

    // build parallel object
    const parallel = {};
    for (let prop in builders) {

        let property = prop,
            fn = builders[prop];

        if (fn) {
            parallel[property] = buildParallelFn(fn, property);
        }
    }

    const parallelFinalCallback = function (err, result) {
        if (err) {

        }
        merge(r, result);
        callback(r);
    };

    // perform parallel actions
    async.parallel(parallel, parallelFinalCallback);
};

Task.createRequest = function () {


    let tasks = Array.prototype.slice.call(arguments, 0);
    const callback = tasks.pop();

    tasks.push(function (r) {

        // output of full request before requesting


        // Task will put this all into JSON
        r.content = JSON.stringify(r.content);
        r.contentType = "application/json";

        callback(request(r));
    });

    Task.createRequestObject.apply(null, tasks);
};

exports.Task = Task;
