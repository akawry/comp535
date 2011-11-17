var express = require('express');
var app = express.createServer();

app.get('/', function(req, res){
	res.sendfile(__dirname + '/index.html');
});

app.get(/.*\.js/, function(req, res){
	console.log("Serving static js file: %s\n", req.url);
	res.sendfile(__dirname + '/' + req.url);
});

app.listen(9000);
