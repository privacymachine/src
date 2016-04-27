var identifier,
    htmlSelectors = self.options.htmlSelectors,
    wasBlocked = false;

self.port.on("identifier", function (id) {
    identifier = id;
});

self.port.on("template", function (name, content) {
    switch (name) {
        case "search.html":
            jQuery(htmlSelectors.filter.join(", ")).first().prepend(content);
            self.port.emit("status", identifier);
            break;

        case "social.html":
            jQuery(htmlSelectors.filter.join(", ")).first().prepend(content);
            self.port.emit("social_request", identifier);
            jQuery("#YouTubeUnblockerSocialHint").addClass("search");
            break;
    }
});

self.port.on("status", function (data) {

    

    if (data.status === "unblocked") {

        self.port.emit("template", "social.html");

        jQuery("#YouTubeUnblockerFlag").attr("class", data.country);
        jQuery("#YouTubeUnblockerSearch").addClass("unblocked").show();

    }
    else if (data.status === "disabled" || data.status === "error") {

        var count = 0;
        jQuery(data.countries).each(function (index, countryCode) {
            if (data.status !== "error" || countryCode !== data.country) {
                if (count === 0) {
                    jQuery("#YouTubeSearchSelectUnblock").addClass(countryCode);
                }
                count++;
                jQuery("<option/>", {
                    "text" : countryCode,
                    "value": countryCode,
                    "class": countryCode
                }).appendTo("#YouTubeSearchSelectUnblock");
            }
            jQuery("#YouTubeUnblockerSearch").show();
        });

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
    wasBlocked = true;
    jQuery("#YouTubeUnblockerSearch").addClass("unblocking");
});

self.port.on("blocked", function () {
    jQuery("#YouTubeUnblockerSearch").removeClass("unblocking").addClass("failed").show();
});

self.port.on("error", function () {
    jQuery("#YouTubeUnblockerSearch").removeClass("unblocking").addClass("error").show();
});

self.port.on("unblocked", function () {
    if (wasBlocked) {
        setTimeout(function () {
            document.location.reload();
        }, 600);
    }
});

self.port.on("disabled", function () {
    document.location.reload();
});

self.port.on("destroy", function () {
    jQuery("#YouTubeUnblockerSocialHint").remove();
    jQuery("#YouTubeUnblockerSearch").remove();
    self.port.emit("destroyed", null);
});

jQuery(function () {
    self.port.emit("template", "search.html");
    jQuery(document).on(
        "click",
        "#YouTubeUnblockerSearchButtonUnblock, #YouTubeUnblockerSearchButtonReload",
        function () {
            var countryCode = jQuery("#YouTubeSearchSelectUnblock").val();
            self.port.emit("start", countryCode);
        }
    );
    jQuery(document).on(
        "click",
        "#YouTubeUnblockerSearchButtonCancel",
        function () {
            self.port.emit("stop", null);
        }
    );
    jQuery(document).on("click", "#YouTubeUnblockerSocialHintClose", function () {
        self.port.emit("social_close", null);
        jQuery("#YouTubeUnblockerSocialHint").remove();
        return false;
    });

    jQuery(document).on("change", "#YouTubeSearchSelectUnblock", function () {
        jQuery(this).attr("class", jQuery(this).find(":selected").attr("class"));
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
