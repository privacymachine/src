self.port.on('translate', function (file) {
    self.port.emit(
        'html_translated',
        file,
        document.getElementsByTagName('body')[0].outerHTML
    );
});
