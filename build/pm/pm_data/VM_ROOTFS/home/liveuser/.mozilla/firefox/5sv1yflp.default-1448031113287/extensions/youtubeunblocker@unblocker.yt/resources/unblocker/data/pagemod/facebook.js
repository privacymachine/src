jQuery(function () {
    self.port.emit("check_status", window.location.href);
});

self.port.on("check_status", function (data) {
    
    if (data.status === "blocked" || data.status === "unblocked") {
        var embed = jQuery("div#player"),
            query = data.url.split("?"),
            params;

        if (data.status === "unblocked") {
            params = query[1] ? "?" + query[1] + "&wmode=opaque" : "?wmode=opaque";
        }
        else {
            params = query[1] ? "?" + query[1].replace("autoplay=1", "autoplay=0") + "&wmode=opaque" : "?wmode=opaque";
        }

        var url = "https://www.youtube.com/embed/" + data.videoId + params + "&blocked=true&facebook=true";

        window.location.href = url;
    }
});
