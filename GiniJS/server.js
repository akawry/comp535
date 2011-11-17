var express = require('express');
var app = express.createServer();

app.get('/', function(req, res){
	res.sendfile(__dirname + '/index.html');
});

app.get(/.*\.js/, function(req, res){
	console.log("Serving static js file: %s", req.url);
	res.sendfile(__dirname + '/' + req.url);
});

app.get(/.*\.(gif|png)/, function(req, res){
	console.log("Serving static image file: %s", req.url);
	res.sendfile(__dirname + '/' + req.url);
});

app.listen(9000, function(){
	console.log("listening on port 9000...");
});
