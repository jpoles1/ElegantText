Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://jpoles1.github.io/ElegantText/';
  //var url = "http://192.168.1.150:8080/"
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
  dict['txt_R'] = parseInt(textColor.substring(2, 4), 16);
  dict['txt_G'] = parseInt(textColor.substring(4, 6), 16);
  dict['txt_B'] = parseInt(textColor.substring(6), 16);
  dict['ac_R'] = parseInt(accentColor.substring(2, 4), 16);
  dict['ac_G'] = parseInt(accentColor.substring(4, 6), 16);
  dict['ac_B'] = parseInt(accentColor.substring(6), 16);
  dict['blt_vibrate'] = configData['blt_vibrate']
  dict['blt_indicator'] = configData['blt_indicator']


  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});
