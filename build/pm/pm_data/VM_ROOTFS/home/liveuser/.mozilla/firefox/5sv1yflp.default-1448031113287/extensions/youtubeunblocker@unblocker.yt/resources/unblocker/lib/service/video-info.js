//sdk
const { Cc, Ci } = require("chrome"),
    { Class }    = require("sdk/core/heritage");

//addon
const Config = require("../config").getConfig();

const VideoInfo = Class({
    URL              : Config.videoInfo.url,
    STATUS_OK        : "STATUS_OK",
    STATUS_BLOCKED   : "STATUS_BLOCKED",
    STATUS_CODE_ERROR: -1,
    TIMEOUT          : 2000,
    READY_STATE      : 4,
    READY_STATUS     : 200,

    get: function (videoId, callback) {
        // https://developer.mozilla.org/en-US/Add-ons/SDK/Release_notes
        // Request.anonymous available for FF>31

        // { Request } = require("sdk/request"),
        /*

         Request({
         url       : this.URL.replace("{videoId}", videoId),
         anonymous : true,
         onComplete: (response) => {
         const regex = new RegExp(Config.videoInfo.blockCheck, "i");
         let result = regex.test(response.text);

         if (result) {
         result = !/certain/.test(response.text);
         }

         if (result) {
         callback(null, this.STATUS_BLOCKED);
         } else {
         callback(null, this.STATUS_OK);
         }
         }
         }).get();

         */

        let request,
            callbacked = false;
        if (typeof XMLHttpRequest !== "undefined") {
            request = new XMLHttpRequest();
        }
        else {
            request = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance(Ci.nsIXMLHttpRequest);
        }

        request.open("GET", this.URL.replace("{videoId}", videoId), true);
        request.channel.loadFlags |= Ci.nsIRequest.LOAD_ANONYMOUS;

        request.timeout = this.TIMEOUT;
        request.ontimeout = () => {
            
            if (callbacked) {
                return;
            }
            callbacked = true;
            callback(null, this.STATUS_BLOCKED, this.STATUS_CODE_ERROR);
        };

        // handle error
        request.onerror = (event) => {
            
            if (callbacked) {
                return;
            }
            callback(event.error, null);
            callbacked = true;
        };

        // handle response
        request.onreadystatechange = () => {
            // ready?
            
            if (request.readyState === this.READY_STATE) {
                if (callbacked) {
                    return;
                }
                callbacked = true;
                
                if (request.status !== this.READY_STATUS) {
                    callback(null, this.STATUS_BLOCKED, this.STATUS_CODE_ERROR);
                }
                else {
                    
                    this.handleResponse(videoId, callback, request.responseText);
                }
            }
        };
        request.send(null);
    },

    formatResponse: function (str) {

        const pairs = str.split("&"),
            temp = {};

        for (let i = 0; i < pairs.length; i++) {
            const pair = pairs[i].split("=");
            temp[pair[0]] = pair[1];
        }
        return temp;

    },

    testResponse: function (test, object) {

        if (typeof object[test.field] === "undefined") {
            return typeof test.value === "undefined";
        }

        return !!new RegExp(test.value, "i").test(object[test.field]);

    },

    handleResponse: function (videoId, callback, reqResponse) {

        const responseObj = this.formatResponse(reqResponse);
        let result = false;

        for (let i in Config.videoInfo.blockCheck) {
            result = result || this.testResponse(Config.videoInfo.blockCheck[i], responseObj);
        }

        let resultNegative = false;
        for (let i in Config.videoInfo.blockCheckNegative) {
            resultNegative = resultNegative || this.testResponse(Config.videoInfo.blockCheckNegative[i], responseObj);
        }

        result = result && !resultNegative;

        for (let i in Config.videoInfo.blockCheckValid) {
            resultNegative = resultNegative || this.testResponse(Config.videoInfo.blockCheckValid[i], responseObj);
        }

        if (!resultNegative && !result) {
            result = true;
        }

        let errorCode = 0;

        if (responseObj && responseObj.errorcode) {
            errorCode = responseObj.errorcode;
        }

        if (result) {
            callback(null, this.STATUS_BLOCKED, errorCode);
        }
        else {
            callback(null, this.STATUS_OK);
        }

    }
});

exports.VideoInfo = new VideoInfo();
