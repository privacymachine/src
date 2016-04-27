const { Class } = require("sdk/core/heritage"),
    self = require("sdk/self"),
    {ActionButton} = require("sdk/ui/button/action"),
    {emit} = require("sdk/event/core"),
    {EventTarget}  = require("sdk/event/target");

const Button = Class({

    EVENT_ON_CLICK: "button_click",
    extends       : EventTarget,
    sdkButton     : null,
    install       : function () {
        this.sdkButton = ActionButton({
            id     : "youtube-unblocker",
            label  : "YouTube Unblocker",
            icon   : {
                "16": self.data.url("img/icon-16.png"),
                "32": self.data.url("img/icon-32.png"),
                "64": self.data.url("img/icon-64.png")
            },
            onClick: () => {
                emit(this, this.EVENT_ON_CLICK, this.sdkButton);
            }
        });
    },
    uninstall     : function () {
        if (this.sdkButton) {
            this.sdkButton.destroy();
            this.sdkButton = null;

        }
    }
});

exports.Button = Button;
