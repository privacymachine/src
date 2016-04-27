var videoId = null,
    htmlSelectors = self.options.htmlSelectors,
    isError = false,
    wasBlocked = false;

self.port.on("identifier", function (id) {
    videoId = id;
    self.port.emit("start", null);
});

self.port.on("template", function (name, content) {
    switch (name) {

        case "video.html":
            jQuery(htmlSelectors.player.join(", ")).first().prepend(content);

            if (wasBlocked === false && isError === false) {
                jQuery("#YouTubeUnblockerVideo").addClass("failed").show();
            }
            else if (isError === true) {
                jQuery("#YouTubeUnblockerVideo").addClass("error").show();
            }
            else {
                jQuery("#YouTubeUnblockerVideo").addClass("video unblocking").show();
                jQuery("#YouTubeUnblockerTheme").css("background-image", "url(" + self.options.campaignImage + ")");
                setTimeout(function () {
                    jQuery("#YouTubeUnblockerAnimation").addClass("shake");
                }, 2000);
            }
            break;

        case "social.html":
            jQuery(htmlSelectors.content.join(", ")).first().prepend(content);
            jQuery("#YouTubeUnblockerSocialHint").addClass("video");
            self.port.emit("social_request", videoId);
            break;
    }
});

self.port.on("social_request", function (size) {
    if (size === "hidden") {
        return;
    }

    var height,
        width,
        locale = jQuery("#YouTubeUnblockerSocialHintShareLocale").text();

    if (size === "small") {
        height = "21";
        if (locale === "de_DE") {
            width = "130";
        }
        else {
            width = "85";
        }
    }
    else {
        height = "65";
        if (locale === "de_DE") {
            width = "80";
        }
        else {
            width = "48";
        }
    }

    var params = {
        "href"       : "https://www.facebook.com/YouTubeUnblocker",
        "appId"      : "169589813122791",
        "locale"     : locale,
        "layout"     : (size === "small") ? "button_count" : "box_count",
        "action"     : "like",
        "colorscheme": "light",
        "width"      : width,
        "height"     : height,
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

    jQuery("#YouTubeUnblockerSocialHint").addClass(size + " " + locale).show();
    if (size === "big") {
        self.port.emit("social_show");
    }
});

self.port.on("unblocking", function () {
    if (!wasBlocked) {
        wasBlocked = true;
        self.port.emit("template", "video.html");
    }
});

self.port.on("blocked", function () {
    if (wasBlocked) {
        jQuery("#YouTubeUnblockerVideo").removeClass("unblocking").addClass("failed");
    }
    else {
        self.port.emit("template", "video.html");
    }
});

self.port.on("unblocked", function () {
    if (wasBlocked) {
        setTimeout(function () {
            document.location.reload();
        }, 300);
    }
    else {
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

    jQuery(document).on("click", "#YouTubeUnblockerSocialHintClose", function () {
        self.port.emit("social_close", null);
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
