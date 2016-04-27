var videoId = null,
    htmlSelectors = self.options.htmlSelectors,
    wasBlocked = false,
    isError = false,
    hasTemplate = false;

self.port.on("identifier", function (id) {
    videoId = id;
    self.port.emit("status", id);
});

self.port.on("status", function (status) {
    if (status === "unblocked") {
        self.port.emit("start");
    }
    else if (document.location.search.indexOf("facebook=true") > -1) {
        wasBlocked = true;
        self.port.emit("start");
    }
});

self.port.on("template", function (name, content) {
    switch (name) {
        case "video.html":
            jQuery(htmlSelectors.wrapper.join(", ")).first().prepend(content);

            if (!wasBlocked && !isError) {
                jQuery("#YouTubeUnblockerVideo").addClass("embed failed").show();
            }
            else if (isError === true) {
                jQuery("#YouTubeUnblockerVideo").addClass("embed error").show();
            }
            else {
                jQuery("#YouTubeUnblockerVideo").addClass("embed unblocking").show();
                setTimeout(function () {
                    jQuery("#YouTubeUnblockerAnimation").addClass("shake");
                }, 2000);
            }
            break;

        case "social.html":
            jQuery(htmlSelectors.wrapper.join(", ")).first().prepend(content);

            var locale = jQuery("#YouTubeUnblockerSocialHintShareLocale").text();

            var params = {
                "href"       : "https://www.facebook.com/YouTubeUnblocker",
                "appId"      : "169589813122791",
                "locale"     : locale,
                "layout"     : "button_count",
                "action"     : "like",
                "colorscheme": "light",
                "width"      : (locale === "en_US") ? "85" : "130",
                "height"     : "21",
                "send="      : "false",
                "share"      : "false",
                "show_faces" : "false"
            };

            jQuery("<iframe/>", {
                "id"               : "YouTubeUnblockerFacebookLike",
                "src"              : "//www.facebook.com/plugins/like.php?" + jQuery.param(params),
                "scrolling"        : "no",
                "frameborder"      : "0",
                "allowTransparency": "true"
            }).appendTo("#YouTubeUnblockerSocialHintIframe");

            jQuery("#YouTubeUnblockerSocialHint").addClass("embed " + locale).show();
            break;
    }
});

self.port.on("unblocking", function () {
    if ((!wasBlocked || document.location.search.indexOf("facebook=true") > -1) && !hasTemplate) {
        wasBlocked = true;
        hasTemplate = true;
        self.port.emit("template", "video.html");
    }
});

self.port.on("blocked", function () {

    if (!hasTemplate) {
        self.port.emit("template", "video.html");
    }
    else {
        jQuery("#YouTubeUnblockerVideo").removeClass("unblocking").addClass("failed");
    }

});

self.port.on("unblocked", function () {
    if (wasBlocked === true) {
        setTimeout(function () {
            var newLocation;
            if (document.location.search.indexOf("autoplay=") > -1) {
                newLocation = document.location.href.replace("autoplay=0", "autoplay=1");
            }
            else {
                newLocation = document.location.href + "&autoplay=1";
            }
            document.location.href = newLocation;
        }, 600);
    }
    else {
        jQuery("#YouTubeUnblockerTrigger").remove();
        self.port.emit("template", "social.html");
    }
});

self.port.on("error", function () {
    isError = true;
    self.port.emit("template", "video.html");
});

self.port.on("destroy", function () {
    jQuery("#YouTubeUnblockerSocialHint").remove();
    jQuery("#YouTubeUnblockerVideo").remove();
    self.port.emit("destroyed", null);
});

jQuery(function () {
    jQuery("<a>", {
        id   : "YouTubeUnblockerTrigger",
        text : "",
        href : "#",
        click: function () {
            self.port.emit("start");
            return false;
        }
    }).appendTo("body");

    jQuery(document).on("click", "#YouTubeUnblockerSocialHintClose", function () {
        jQuery("#YouTubeUnblockerSocialHint").remove();
        return false;
    });

    jQuery(document).on("click", "#YouTubeUnblockerSocialHintShareButton", function () {
        var shareUrl = jQuery("#YouTubeUnblockerSocialHintShareLink").text();
        window.open(
            "https://www.facebook.com/sharer/sharer.php?u=" + encodeURIComponent(shareUrl),
            "facebook-share-dialog",
            "toolbar=0,status=0,width=626,height=436"
        );
        return false;
    });

});
