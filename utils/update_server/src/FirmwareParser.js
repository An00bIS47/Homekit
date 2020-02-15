const fs = require('fs');
const crypto = require('crypto');

function FirmwareParser(firmware_file) {
	this.firmware_file = firmware_file;
	this.isValidFirmware = false;

	this.firmware_size = undefined;
	this.version = undefined;
	this.brand = undefined;
	this.feature_rev = undefined;
	this.feature_rev_binary = undefined;
	this.md5 = undefined;
	this.sha256 = undefined;
}


FirmwareParser.prototype.processFirmware = function() {	
	this.firmware_binary = fs.readFileSync(this.firmware_file);
	if (this.parseFirmware(this.firmware_binary)){
		// console.log("name: " + this.name)
		// console.log("brand: " + this.brand)
		// console.log("version: " + this.version)
		// console.log("feature rev.: " + this.feature_rev_binary)	
		// console.log("firmware size: " + this.firmware_size)		
		result = {}
		result["name"] = this.name.toString();
		result["brand"] = this.brand.toString();
		result["version"] = this.version.toString();
		result["md5"] = this.md5.toString();

		result["firmware_file"] = this.firmware_file.toString();		
		result["firmware_size"] = this.firmware_size;
		result["feature_rev_binary"] = this.feature_rev_binary.toString();
		result["feature_rev"] = parseInt(this.feature_rev_binary.toString(), 2).toString(16);
		
		return result;
	} 

	return undefined;
};



FirmwareParser.prototype.parseFirmware = function(data) {

	this.isValidFirmware = (data.indexOf("25484f4d454b49545f45535033325f465725", 0, "hex") !== -1)

	if (!this.isValidFirmware) {
		console.log("Error: Not a valid firmware!")
		return false;
	}
	this.name = this.extractValues(data, "bf84e41354", "93446ba775");
	this.brand = this.extractValues(data, "fb2af568c0", "6e2f0feb2d");
	this.version = this.extractValues(data, "6a3f3e0ee1", "b03048d41a");
	this.feature_rev_binary = this.extractValues(data, "6a3f3e0ee2", "b03048d41b");
	this.firmware_size	= data.length;	
	this.md5 = crypto.createHash('md5').update(data).digest("hex")

	return true;
}


FirmwareParser.prototype.getFilesizeInBytes = function(filename) {
    var stats = fs.statSync(filename)
    var fileSizeInBytes = stats["size"]
    return fileSizeInBytes
}

FirmwareParser.prototype.extractValues = function(data, pattern_start, pattern_end, offset=5) {
	var idx_start = data.indexOf(pattern_start, 0, "hex") + offset;
	var idx_end   = data.indexOf(pattern_end, 0, "hex");
	if (idx_start !== -1) {
		return Buffer.from(data.slice(idx_start, idx_end));	
	}
}



module.exports = FirmwareParser;