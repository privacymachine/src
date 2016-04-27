const { Cc, Ci, Cu, components }    = require("chrome"),
    { Class }                       = require("sdk/core/heritage"),
    self = require("sdk/self"),
    { Request }                     = require("sdk/request"),
    fileIO = require("sdk/io/file"),
    Path = require('sdk/fs/path');

const { AddonManager }              = Cu.import("resource://gre/modules/AddonManager.jsm"),
    { NetUtil }                     = Cu.import("resource://gre/modules/NetUtil.jsm"),
    { FileUtils } = Cu.import("resource://gre/modules/FileUtils.jsm");



// cache reference to this Addon's nsIAddon instance
let nsIAddon = null;

const Utils = Class({
    getNsIAddon       : function (callback) {
        if (!nsIAddon) {
            AddonManager.getAddonByID(self.id, (aAddon) => {

                nsIAddon = aAddon;
                callback(nsIAddon);
            });
        }
        else {
            callback(nsIAddon);
        }
    },
    getUUID           : function () {
        let path = this.getProfilePath();
        return this.md5(path);
    },
    getProfilePath    : function () {
        return Cc["@mozilla.org/file/directory_service;1"].getService(Ci.nsIProperties).get("ProfD", Ci.nsIFile).path;
    },
    getAppVersion     : function () {
        return Cc["@mozilla.org/xre/app-info;1"].getService(Ci.nsIXULAppInfo).version;
    },
    getOperatingSystem: function () {
        return Cc["@mozilla.org/xre/app-info;1"].getService(Ci.nsIXULRuntime).OS;
    },
    getCurrentUser    : function (callback) {
        this.getNsIAddon((nsIAddon) => {


            let user = {
                guid      : self.id,
                version   : self.version,
                uuid      : this.getUUID(),
                ua_version: this.getAppVersion(),
                os        : this.getOperatingSystem(),
                store     : (nsIAddon.updateURL === null)
            };

            callback(user);
        });
    },
    md5               : function (string) {
        const converter = Cc["@mozilla.org/intl/scriptableunicodeconverter"].
            createInstance(Ci.nsIScriptableUnicodeConverter);
        converter.charset = "UTF-8";

        const result = {};
        const data = converter.convertToByteArray(string, result);
        const ch = Cc["@mozilla.org/security/hash;1"].
            createInstance(Ci.nsICryptoHash);
        ch.init(ch.MD5);
        ch.update(data, data.length);
        const hash = ch.finish(false);

        function toHexString(charCode) {
            return ("0" + charCode.toString(16)).slice(-2);
        }

        let hex = "";
        for (let i in hash) {
            hex += toHexString(hash.charCodeAt(i));
        }
        return hex;
    },
    md5FromDataUrl    : function (url, callback) {
        this.getNsIAddon((addon) => {
            //make sure we are in extension-data and not in addon to not break signature
            addon.getDataDirectory((directory) => {

                const file = FileUtils.File(Path.join(directory, url));
                let Channel;
                // from FF38 on
                try {
                    Channel = NetUtil.newChannel({
                        uri                     : NetUtil.newURI(file),
                        loadUsingSystemPrincipal: true
                    });
                }
                    // but still try the old way
                catch (e) {
                    Channel = NetUtil.newChannel(file);
                }




                NetUtil.asyncFetch(Channel, (aInputStream, aResult) => {

                    // Check that we had success.
                    // Handle error
                    if (!components.isSuccessCode(aResult)) {

                        callback(aResult, null);
                        return;
                    }




                    const ch = Cc["@mozilla.org/security/hash;1"].createInstance(Ci.nsICryptoHash);
                    ch.init(ch.MD5);
                    const PR_UINT32_MAX = 0xffffffff;
                    ch.updateFromStream(aInputStream, PR_UINT32_MAX);
                    const hash = ch.finish(false);

                    function toHexString(charCode) {
                        return ("0" + charCode.toString(16)).slice(-2);
                    }

                    let hex = "";
                    for (let i in hash) {
                        hex += toHexString(hash.charCodeAt(i));
                    }

                    callback(null, hex);
                });
            });
        });

    },
    updateConfigFile  : function (local, remote) {


        var allowedFiles = ["config\.json", "proxies\.json", "campaign_background\.png"];

        this.getNsIAddon((addon) => {
            //make sure we are in extension-data and not in addon to not break signature
            addon.getDataDirectory((directory) => {
                //ensure the filename transferred is within whitelist and data folder
                if (!local.match(new RegExp("/data/(" + allowedFiles.join("|") + ")"))) {
                    throw new Error("Invalid filename provided");
                }

                const nsIFile = FileUtils.File(Path.join(directory, local));

                NetUtil.asyncFetch(
                    NetUtil.newURI(remote, null, null),
                    function (istream) {
                        const ostream = Cc["@mozilla.org/network/file-output-stream;1"].
                            createInstance(Ci.nsIFileOutputStream);

                        ostream.init(nsIFile, -1, -1, Ci.nsIFileOutputStream.DEFER_OPEN);
                        NetUtil.asyncCopy(istream, ostream, function (result) {
                            if (!components.isSuccessCode(result)) {
                                console.error("Update failed", local, result);
                            }

                        });
                    }
                );
            });
        });
    }
});

// expose Utils object instance
exports.Utils = new Utils();
