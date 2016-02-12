Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://jpoles1.github.io/ElegantText/';
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  var backgroundColor = configData['background_color'];
  var textColor = configData['text_color'];
  var accentColor = configData['accent_color'];

  var dict = {};
  dict['bg_R'] = parseInt(backgroundColor.substring(2, 4), 16);
  dict['bg_G'] = parseInt(backgroundColor.substring(4, 6), 16);
  dict['bg_B'] = parseInt(backgroundColor.substring(6), 16);

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});
