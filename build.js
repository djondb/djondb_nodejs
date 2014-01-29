var spawn = require('child_process').spawn,
	 fs = require('fs'),
	 path = require('path');

console.info("Architecture: " + process.arch);
console.info("Platform: " + process.platform);
//Parse args
var force = false;
var arch = process.arch,
	 platform = process.platform,
	 v8 = /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];
	 var args = process.argv.slice(2).filter(function(arg) {
			if (arg === '-f') {
				force = true;
				return false;
			} else if (arg.substring(0, 13) === '--target_arch') {
				arch = arg.substring(14); }
		return true;
	});

if (!{ia32: true, x64: true, arm: true}.hasOwnProperty(arch)) {
	console.error('Unsupported (?) architecture: `'+ arch+ '`');
	process.exit(1);
}

// Test for pre-built library
var modPath = platform+ '-'+ arch+ '-v8-'+ v8;
if (!force) {
	try {
		fs.statSync(path.join(__dirname, 'bin', modPath, 'djondb.node'));
		console.log('`'+ modPath+ '` exists; skipping build');
		return process.exit();
	} catch (ex) {}
}

function checkExists(path) {
	try {
		fs.statSync(path);
		return true;
	} catch (ex) {
		return false;
	}
}

function generateDefs(nodeh) {
	var fs = require('fs');
	var text = "#ifndef DJONDB_DEFS_H\n";
	text += "#define DJONDB_DEFS_H\n";
	text += "#include <" + nodeh + ">\n";
	text += "#endif // DJONDB_DEFS_H\n";
	fs.writeFileSync("djondefs.h", text);
}

function checkDependencies() {
	var libPath;
	var libLocalPath;
	var includePath;
	var includeLocalPath;
	var djonLibrary;
	if (process.platform == "linux") {
		libPath = "/usr/lib";
		libLocalPath = "/usr/local/lib";
		includePath = "/usr/include";
		includeLocalPath = "/usr/local/include";
		djonLibrary = "libdjon-client.so";
	} else if (process.platform = "darwin") {
		libPath = "/usr/lib";
		libLocalPath = "/usr/local/lib";
		includePath = "/usr/include";
		includeLocalPath = "/usr/local/include";
		djonLibrary = "libdjon-client.dylib";
	}
	var exist = checkExists(path.join(libPath, djonLibrary)) || checkExists(path.join(libLocalPath, djonLibrary));
	if (!exist) {
		console.error("djondb client library not found, please install the development package or server and try again");
		process.exit(1);
	}

	generateDefs("node.h");
}

function configure(done) {
	// Configure
	spawn(
			process.platform === 'win32' ? 'node-gyp.cmd' : 'node-gyp',
			['configure'].concat(args),
			{customFds: [0, 1, 2]}).on('exit', 
				function(err) {
					if (err) {
						if (err === 127) {
							console.error(
								'node-gyp not found! Please upgrade your install of npm!. You will need to execute: \n npm install -g node-gyp.'
								);
						} else {
							console.error('Build failed');
						}
						return process.exit(err);
					} else {
						done();
					}
				});

}

//Build it
function build() {
	spawn(
			process.platform === 'win32' ? 'node-gyp.cmd' : 'node-gyp',
			['build'].concat(args),
			{customFds: [0, 1, 2]})
	.on('exit', function(err) {
		if (err) {
			if (err === 127) {
				console.error(
					'node-gyp not found! Please upgrade your install of npm!. You will need to execute: \n npm install -g node-gyp.'
					);
			} else {
				console.error('Build failed');
			}
			return process.exit(err);
		}
	});
}

checkDependencies();
configure(build);
