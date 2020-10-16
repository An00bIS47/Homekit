var request = require('request');

var options = {
  'method': 'GET',
  'url': 'https://esp32-CB3DC4/api/config',
  'headers': {
    'Connection': 'keep-alive',
    'Authorization': 'Basic QWRtaW46c2VjcmV0'
  },
  'rejectUnauthorized': false
};



request(options, function (error, response) {
  if (error) throw new Error(error);
  console.log(response.body);
  currentConfig = response.body
});

