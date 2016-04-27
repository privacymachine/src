function handleOnOff() {
    jQuery("#switch_active").prop(
        "checked",
        jQuery("#unblock_youtube").prop("checked") || jQuery("#unblock_websites").prop("checked")
    );
}

self.port.on("change_settings", function (setting) {
    jQuery.each(setting, function (key, value) {
        jQuery("#" + key).attr("checked", value).prop("checked", value);
    });
    handleOnOff();
});

jQuery("#unblock_youtube, #unblock_websites, #trusted_links").on("change", function () {
    var settings = {},
        currentItem = jQuery(this);
    settings[currentItem.attr("id")] = currentItem.prop("checked");
    self.port.emit("change_settings", settings);
    handleOnOff();
});

jQuery("#switch_active").on("change", function () {
    jQuery("#unblock_youtube, #unblock_websites").prop("checked", jQuery("#switch_active").prop("checked")).change();
});

jQuery("#storage_button, #feedback_button").on("click", function () {
    self.port.emit(jQuery(this).attr("data-event"), null);
});
