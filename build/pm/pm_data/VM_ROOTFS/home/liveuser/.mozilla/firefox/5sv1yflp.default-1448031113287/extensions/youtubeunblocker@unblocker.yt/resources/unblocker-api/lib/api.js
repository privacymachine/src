// sdk
const { Class }     = require("sdk/core/heritage"),
    { merge }       = require("sdk/util/object"),
    timers          = require("sdk/timers"),
    self            = require("sdk/self");

// addon
const async = require("async"),
    { Task }        = require("./task"),
    { Utils }       = require("./utils");



/**
 * Class Api
 */
const Api = Class({
    url      : null,
    urlBackup: null,
    waterfall: true,

    TRIES_MAX     : 3,
    TRIES_FALLBACK: 2,
    TRIES_DELAY   : 1000 * 60,

    /**
     * @type Object
     * each request would hold given values in 'headers'
     */
    headers: {
        "X-Api-Version"     : "0.1.2",
        "X-Api-User-Uuid"   : Utils.getUUID(),
        "X-Api-User-Version": self.version,
        "X-Api-User-Guid"   : self.id
    },

    /**
     * @type Object
     * each request would hold given values in 'content'
     */
    content: null,

    buildUrl    : function (callback, url) {

        if (!url) {
            url = this.url;
        }
        callback(null, url);
    },
    buildHeaders: function (callback, headers) {

        headers = headers || {};
        if (this.headers) {
            merge(headers, this.headers);
        }
        callback(null, headers);
    },
    buildContent: function (callback, content) {

        content = content || {};
        if (this.content) {
            merge(content, this.content);
        }
        callback(null, content);
    },
    onComplete  : function (response) {

    },

    /**
     * so far this class fits Task
     */
    tasks                 : [],
    delay                 : 0,
    runs                  : -1,

    // gets calculated on .start as least common divisor of task[].interval
    interval              : null,
    maxInterval           : null,
    inititalTasks         : null,

    initialize            : function (options) {

        // let Config not corrupt this instance
        for (let prop in options) {
            if ([
                "url",
                "urlBackup",
                "headers",
                "content",
                "delay",
                "TRIES_MAX",
                "TRIES_FALLBACK",
                "TRIES_DELAY",
                "waterfall"
            ].indexOf(prop) < 0) {
                delete options[prop];
            }
        }

        // try at least one time
        if (parseInt(options.TRIES_MAX) < 1) {
            delete options.TRIES_MAX;
        }
        // let at least one regular call go
        if (parseInt(options.TRIES_FALLBACK) < 1) {
            delete options.TRIES_FALLBACK;
        }
        // there has to be a delay
        if (!parseInt(options.TRIES_DELAY)) {
            delete options.TRIES_DELAY;
        }

        // experimental
        if (this.headers && options.headers) {
            merge(this.headers, options.headers);
            delete options.headers;

            for (let key in this.headers) {
                if (this.headers[key] === null) {
                    delete this.headers[key];
                }
            }
        }

        if (this.content && options.content) {
            merge(this.content, options.content);
            delete options.content;

            for (let key in this.content) {
                if (this.content[key] === null) {
                    delete this.content[key];
                }
            }
        }

        // instead of Task.prototype.initialize.apply(this, arguments);
        merge(this, options);

        this.tasks = [];


    },
    start                 : function () {

        const GCF = function (a, b) {
            if (b === 0) {
                return a;
            }
            else {
                return (GCF(b, a % b));
            }
        };

        const nGCF = function (numbers) {
            let result = numbers[0];
            for (let i = 1; i < numbers.length; i++) {
                result = GCF(result, numbers[i]);
            }
            return result;
        };

        let initialTasks = [],
            intervals = [];
        // calculate interval
        for (let task of this.tasks) {
            for (let modulo of task.interval) {


                if (modulo > this.maxInterval) {
                    this.maxInterval = modulo;
                }

                // tasks to be run initially
                if (modulo === 0) {
                    if (initialTasks.indexOf(task) < 0) {
                        initialTasks.push(task);
                    }
                }

                // regular
                else if (intervals.indexOf(modulo) < 0) {
                    intervals.push(modulo);
                }

            }
        }

        // remove 0s from task[].interval
        for (let task of initialTasks) {
            task.interval.splice(task.interval.indexOf(0), 1);
        }

        this.interval = nGCF(intervals);







        // store
        this.initialTasks = initialTasks;

        timers.setTimeout(() => {

            // run timer immediately after delay
            this.timer();

            // start intervals
            // if there is as least one beyond 0 (initialTask)
            if (intervals.length) {
                timers.setInterval(
                    () => {
                        this.timer();
                    },
                    this.interval
                );
            }

        }, this.delay);
    },
    timer                 : function () {
        this.runs++;
        const ms = this.runs * this.interval;



        let runTasks = [];

        // initial tasks
        if (this.runs === 0) {
            runTasks = this.initialTasks;
        }
        // regular tasks
        else {
            for (let task of this.tasks) {
                for (let modulo of task.interval) {
                    if (ms % modulo === 0) {

                        runTasks.push(task);
                        break;
                    }
                }
            }
        }

        // re-set
        if (ms % this.maxInterval === 0) {

            this.runs = 0;
        }

        if (runTasks.length) {


            // waterfall requests
            if (this.waterfall) {
                this.requestObjectFromTasks(
                    runTasks,
                    (requestObject) => {
                        this.handleRequestObject(requestObject);
                    }
                );
            }
            // single requests
            else {
                this.singleRequest(runTasks);
            }
        }
    },
    handleRequestObject   : function (requestObject) {


        let tries = 0,
            success = false;
        const _this = this;

        async.doUntil(
            function (callback) {

                // starts being 1
                tries++;

                Task.createRequest(
                    requestObject,
                    {
                        onComplete: function (response) {



                            // if there is a response
                            if (response.status > 0 && response.json) {
                                // store to let ansy.doUntil know its over
                                success = true;
                                // call original onComplete method
                                requestObject.onComplete(response);
                            }
                            // do nothing, let asny.doUntil move on
                            else {

                            }

                            // wait until next try
                            timers.setTimeout(callback, _this.TRIES_DELAY);
                        },
                        buildUrl  : function (callback, url) {
                            // use regular url
                            if (tries <= _this.TRIES_FALLBACK) {
                                callback(null, _this.url);
                            }
                            // use urlFallback
                            else {
                                callback(null, _this.urlBackup);
                            }
                        }
                    },
                    function (request) {
                        request.post();
                    }
                );
            },
            // test if to try the next one
            function () {
                // stop if previous response was successfull
                // stop if tries exeeds _this.TRIES_MAX
                let stop = (success === true || tries >= _this.TRIES_MAX);
                return stop;
            },
            function (err) {
                if (err) {

                }


                // let Tasks know failure anyway
                if (!success) {
                    requestObject.onComplete({
                        status    : 0,
                        headers   : null,
                        statusText: null,
                        json      : null,
                        text      : null
                    });
                }
            }
        );

    },
    requestObjectFromTasks: function (tasks, callback) {

        let paths = [];
        tasks.forEach((task) => {
                paths.push(task.path);
            }
        );


        const parallel = {},
            _this = this;

        tasks.forEach((task) => {
            parallel[task.path] = function (callback) {
                Task.createRequestObject(task, function (r) {

                    callback(null, r);
                });
            };
        });

        async.parallel(
            parallel,
            function (err, requests) {

                const waterfall = {

                    // build content based on tasks
                    buildContent: function (callback, content) {

                        // enhance content by that from each task
                        content = content || {};
                        tasks.forEach((task) => {
                                content[task.path] = requests[task.path].content;
                            }
                        );

                        // now let Api instance base method do its magic
                        _this.buildContent.call(_this, callback, content);
                    },
                    buildHeaders: function (callback, headers) {

                        headers = headers || {};
                        tasks.forEach((task) => {
                                merge(headers, requests[task.path].headers
                                );
                            }
                        );

                        // now let Api instance base method do its magic
                        _this.buildHeaders.call(_this, callback, headers);
                    },

                    // handle results based on tasks
                    onComplete  : function (response) {

                        // to Api's own onComplete
                        _this.onComplete.call(_this, response);

                        // for each task
                        tasks.forEach(
                            (task) => {

                                // prepare response proxy
                                const _res = {
                                    status    : response.status,
                                    statusText: null,
                                    headers   : null,
                                    json      : null,
                                    text      : null
                                };

                                try {
                                    _res.headers = response.headers;
                                    _res.statusText = response.statusText;
                                    // here, upshift into right "content"
                                    _res.json = response.json[task.path];
                                    _res.text = JSON.stringify(response.json[task.path]);
                                }
                                catch (e) {

                                }

                                try {
                                    task.onComplete(_res);
                                }
                                catch (e) {

                                }

                            }

                        );

                    }
                };

                // create waterfall request
                Task.createRequestObject(
                    _this,
                    waterfall,
                    // return request via callback
                    function (r) {
                        callback(r);
                    }
                );
            }
        );

    },

    singleRequest: function (tasks) {


        let tries = 0,
            success = false,
            workingURL = null;

        const _this = this,
            task = tasks.shift();

        // proceed with the remaining
        const onSuccess = () => {
            this.singleRequestRemaining(tasks, workingURL);
        };

        // nothing worked out, apply tasks with an "empty" response
        const onFailure = () => {
            tasks.forEach(
                (task) => {
                    task.onComplete({
                        status    : 0,
                        statusText: null,
                        headers   : null,
                        text      : null,
                        json      : null
                    });
                }
            );
        };

        async.doUntil(
            (callback) => {

                // starts being 1
                tries++;

                Task.createRequest(
                    this,
                    task,
                    {
                        onComplete: function (response) {



                            // if there is a response
                            if (response.status > 0 && response.json) {
                                // store to let ansy.doUntil know its over
                                success = true;

                                task.onComplete(response);



                                // handle the remaining tasks
                                onSuccess();
                            }
                            // do nothing, let asny.doUntil move on
                            else {

                            }

                            // wait until next try
                            timers.setTimeout(callback, _this.TRIES_DELAY);
                        },
                        buildUrl  : function (callback, url) {
                            // use regular url
                            if (tries <= _this.TRIES_FALLBACK) {
                                workingURL = _this.url;
                                callback(null, _this.url + task.path);
                            }
                            // use urlFallback
                            else {
                                workingURL = _this.urlBackup;
                                callback(null, _this.urlBackup + task.path);
                            }
                        }
                    },
                    function (request) {
                        request.post();
                    }
                );

            },
            // test if to try the next one
            function () {
                // stop if previous response was successfull
                // stop if tries exeeds _this.TRIES_MAX
                let stop = (success === true || tries >= _this.TRIES_MAX);
                // debug

                return stop;
            },
            function (err) {
                if (err) {

                }


                // let Tasks know failure anyway
                if (!success) {

                    // initial task
                    task.onComplete({
                        status    : 0,
                        statusTest: null,
                        headers   : null,
                        text      : null,
                        json      : null
                    });

                    // remaining tasks
                    onFailure();
                }
            }
        );

    },

    singleRequestRemaining: function (tasks, workingURL) {


        async.eachSeries(
            tasks,
            (task, callback) => {

                Task.createRequest(this, task, {
                    onComplete: function (response) {

                        task.onComplete(response);

                        // no error checking, just proceed
                        callback(null);
                    },
                    buildUrl  : function (callback, url) {
                        callback(null, workingURL + task.path);
                    }
                }, function (r) {
                    r.post();
                });

            },
            function (err) {
                if (err) {

                }
            }
        );

    }
});

exports.Api = Api;
