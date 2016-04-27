jQuery(function () {
    var embeds = jQuery("iframe[src*='youtube.com'], embed[src*='youtube.com']");
    jQuery(embeds).each(function () {
        self.port.emit("check_status", jQuery(this).attr("src"));
    });
});

self.port.on("check_status", function (data) {
    if (data.status === "blocked" || data.status === "unblocked") {

        var elements = jQuery("iframe[src*='" + data.url + "'], embed[src*='" + data.url + "']"),
            query = data.url.split("?"),
            url = "//www.youtube.com/embed/" + data.videoId +
                (query[1] ? "?" + query[1] + "&blocked=true" : "?blocked=true");

        elements.each(function () {
            var element = jQuery(this),
                replace = jQuery("<iframe />", {
                    src        : url,
                    frameborder: 0,
                    width      : element.width(),
                    height     : element.height()
                });
            // if object > embed   => remove object
            if (element.parent()[0].nodeName === "OBJECT") {
                element = jQuery(element.parent()[0]);
            }
            replace.insertAfter(element);
            element.remove();
        });
    }
});
