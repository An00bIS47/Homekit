const fs = require('fs');
const express = require("express");
const Parser = require('./FirmwareParser.js');
const path = require('path');
const mime = require('mime');
const https = require('https');



const host = '0.0.0.0';
const app = express();

var config = require('./config.js');

app.use(express.json());
app.use(express.urlencoded({ extended: true }));


console.log("Starting update server");

const key = fs.readFileSync(config.key);
const cert = fs.readFileSync(config.cert);
const server = https.createServer({key: key, cert: cert }, app);


function isInStorage(data){

	for (let i=0; i<config.firmwares.length; i++){
		var item = config.firmwares[i];

		if (item.md5 == data.md5){
			return true;			
		}
	}
	return false;
}

function isGreaterVersion(version, version_to_check){
	// undefined on error
	// 1 on greater
	// 0 on smaller or equal
	var array_version = version.split(/(?:\.|\+)+/);
	var array_version_to_check = version_to_check.toString().split(/(?:\.|\+)+/); 

	if ( (array_version.length < 4 ) && (array_version_to_check < 4) ){
		return undefined;
	}

	for (let i=0; i < array_version.length; i++){
		// console.log(array_version[i] + " <> " + array_version_to_check[i]);	
		if (array_version[i] < array_version_to_check[i]){
			//console.log("true");
			return true;	
		} 
	}
	return false;
}

function isFeatureCompatible(feature_rev_binary, feature_rev_to_check){	

	var feature_rev_binary_to_check = parseInt(parseInt(feature_rev_to_check, 16).toString(2));
	
	if ( feature_rev_binary &= feature_rev_binary_to_check == feature_rev_binary_to_check ) {
		return true;
	}
	return false;
}

function fromDir(startPath,filter,callback){

    //console.log('Starting from dir '+startPath+'/');

    if (!fs.existsSync(startPath)){
        console.log("no dir ",startPath);
        return;
    }

    var files=fs.readdirSync(startPath);
    for(var i=0;i<files.length;i++){
        var filename=path.join(startPath,files[i]);
        var stat = fs.lstatSync(filename);
        if (stat.isDirectory()){
            fromDir(filename,filter,callback); //recurse
        }
        else if (filter.test(filename)) callback(filename);
    };
};


function createStorageDir(folderName) {
	try {
	  	if (!fs.existsSync(folderName)) {
	   		fs.mkdirSync(folderName)
	  	}
	} catch (err) {
	  	console.error(err)
	}	
}



app.post('/api/update', (req, res) => {
	console.log(req);
	console.log("<< Looking for updates for version " + req.body.version + " rev: " + req.body.feature_rev);

	console.log("-- No of available firmwares: " + config.firmwares.length);

	var latestVersion = undefined;

	for (let i=0; i < config.firmwares.length; i++) {
		var item = config.firmwares[i];

		;

		if ( (item.name === req.body.name) && (item.brand == req.body.brand) ){

			if (isFeatureCompatible(item.feature_rev_binary, req.body.feature_rev)) {
				if (latestVersion == undefined) {
					latestVersion = item;
				} else {					
					if (isGreaterVersion(latestVersion.version, item)){
						latestVersion = item;
					}
				}
			}

		}
	}

	if (latestVersion === undefined){
		console.log(">> No update found! - 204");
		res.sendStatus(204);
		return;
	} else if (isGreaterVersion(req.body.version, latestVersion.version)) {
		console.log(">> Returning latest feature compatible firmware version: " + latestVersion.version + " rev: " + latestVersion.feature_rev);
	
		var response = JSON.parse(JSON.stringify(latestVersion));
		delete response.firmware_file;
		delete response.feature_rev_binary;

		res.send(response);
	} else {
		console.log(">> No update found! - 204");
		res.sendStatus(204);
		return;
	}
	

});

app.get('/api/update', function(req, res){

	console.log(req);
	
	var name = req.query.name;
	var version = req.query.version;
	var feature_rev = req.query.feature_rev;
	var brand = req.query.brand;

	console.log("<< Request to download firmware: " + version + " rev: " + feature_rev);
	version = version.replace(/\./g, "_");
	version = version.replace(/ /g, "_");
	
	//var file = __dirname + '/firmwares/' + name + "_" + brand + "_" + version + "_" + feature_rev + ".bin";
	var file = __dirname + '/firmwares/' + name + "_" + version + "_" + feature_rev + ".bin";

	//var file = __dirname + '/' + 'Homekit.bin';

	if (!fs.existsSync(file)) {
		console.log(">> No update found! - 204");
		res.sendStatus(204);
		return;
	}

	var filename = path.basename(file);
	var mimetype = mime.lookup(file);

	//res.setHeader('transfer-encoding', ''); // <-- add this line
	res.setHeader('Content-disposition', 'attachment; filename=' + filename);
	res.setHeader('Content-type', mimetype);
	
	console.log(">> Serving firmware file: " + file);

	var filestream = fs.createReadStream(file);
	filestream.pipe(res);
});

server.listen(config.port, host, () =>
	console.log('Example app listening on port ' + config.port + '!'),
);


function processNewFile(filename){
	var parser = new Parser(filename);

	var data = parser.processFirmware();
	if ( data !== undefined ){
		//console.log("-- Firmware parsed!");		

		if (isInStorage(data)){
			//console.log("-- Firmware already in storage!");
		} else {
			console.log("-- Adding new firmware version " + data.version + " rev: "+ data.feature_rev);
			file_version = data.version;

			file_version = file_version.replace(/\./g, "_");
			file_version = file_version.replace(/\+/g, "_");

			var new_file_path = "./firmwares/" + data.name + "_" + file_version + "_" + data.feature_rev + ".bin";		
					
			if (filename != new_file_path) {
				console.log("-- Moving firmware to " + new_file_path);
				fs.renameSync(filename, new_file_path);		
				data.firmware_file = new_file_path;	
			}
					
			config.firmwares.push(data);
		}

	}
}

function watchdog() {	
	fromDir('./',/\.bin$/, function(filename){
	    //console.log('-- Found firmware: ',filename);
	    processNewFile(filename);
	});
} setInterval(watchdog, 10000); //time is in ms
