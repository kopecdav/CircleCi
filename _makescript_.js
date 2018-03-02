const fs = require("fs");
const path = require("path");
const child_process = require('child_process')

var project = process.argv[2];
if (!project) {
    console.log("First param must be build project.");
    process.exit();
}

var target = process.argv[3];
if (!target) {
    console.log("Second param must be build target.");
    process.exit();
}

var command = "";
var gccBin = "";

if (process.argv.length == 5) {
    command = process.argv[4];
} else if (process.argv.length == 6) {
    command = process.argv[5];
    gccBin = process.argv[4];
}

if (!command) {
    command = "all";
}

var walk = function(dir) {
    var results = [];
    var list = fs.readdirSync(dir);
    list.forEach(function(file) {
        var fileAll = dir + '/' + file;
        var stat = fs.statSync(fileAll);
        if (stat && stat.isDirectory()) {
            var w = walk(fileAll);
            w.forEach(function(fileW) {
                results.push(file+'/'+fileW);
            });
        } else {
            results.push(file);
        }
    });
    return results
};

var getIncludes = function(code) {
    var regExp = /^\s*#include "([^.]+)\.h"/gm;
    var ret = [];

    var includesOne;
    while ((includesOne = regExp.exec(code)) !== null) {
        ret.push(includesOne[1]);
    }
    return ret;
};

if (command == "all") {

    var fPath = __dirname+"/"+project;
    fPath = path.resolve(fPath);

    var dir = walk(fPath);

    fs.writeFileSync(fPath+"/main.cpp", fs.readFileSync(fPath+"/main.cpp", "utf8"), {encoding: "utf8"});

    var main = fs.readFileSync(fPath+"/main.cpp", "utf8");

    var includes = [];

    var includesNames = getIncludes(main);

    dir.forEach(function (file) {
        if (file == "main.cpp") return;

        if (file.indexOf(".cpp", file.length - ".cpp".length) !== -1) {
            includes.push(file.substr(0, file.length - ".cpp".length));
        }

        if (file.indexOf(".c", file.length - ".c".length) !== -1) {
            includes.push(file.substr(0, file.length - ".c".length));
        }

        if ((file.indexOf(".cpp", file.length - ".cpp".length) !== -1) || (file.indexOf(".c", file.length - ".c".length) !== -1) || (file.indexOf(".h", file.length - ".h".length) !== -1)) {

            Array.prototype.push.apply(includesNames, getIncludes(fs.readFileSync(fPath+"/"+file, "utf8")));
        }
    });

    //unique
    includesNames = Array.prototype.filter.call(includesNames, function (value, index, self) {
        return self.indexOf(value) === index;
    });

    var includesNamesFinal = [];

    includesNames.forEach(function (include) {
        if (/^([A-Za-z0-9_-]+)$/.test(include)) {
            includesNamesFinal.push("INCLUDE_"+include.toUpperCase()+"=1");
        }
    });

    includesNamesFinal = includesNamesFinal.join(" ");
    includes = includes.join(" ");

    var makeCommandIncludes = "";
    if (includes) {
        makeCommandIncludes = " BUILD_INCLUDES=\""+includes+"\"";
    }

    var makeCommand = "make -f _makefile_ BUILD_ID="+project+" BUILD_TARGET="+target+" GCC_BIN=\""+gccBin+"\" USING_FOLDER_MAIN=1 CUSTOM_BUILD_ID=RND_BUILD_ID_"+Math.round(Math.random()*1000)+""+makeCommandIncludes+" "+includesNamesFinal;

    console.log("MAKE COMMAND: "+makeCommand);

    try {
        //var out = child_process.execSync(makeCommand, {cwd: __dirname});

        var parts = makeCommand.split(" ");
		var returncode = 0;

        const ls = child_process.spawn(parts[0], parts.slice(1), {cwd: __dirname, shell: true});

        ls.stdout.on('data', (data) => {
          process.stdout.write(data.toString("utf8"));
        });

        ls.stderr.on('data', (data) => {
          process.stdout.write(data.toString("utf8"));
        });

        ls.on('close', (code) => {
            console.log("");
            console.log(`Compile process exited with code ${code}`);
			
			// EXIT CODE 
			process.exit(code);
        });
		
		
		

    } catch (e) {
        //console.log(e.toString("utf8"));
    }

}

if (command == "clean") {

    var fPath = __dirname+"/"+project;
    fPath = path.resolve(fPath);

    var dir = walk(fPath);

    dir.forEach(function (file) {
        if ((file.indexOf(".d", file.length - ".d".length) !== -1) || 
            (file.indexOf(".o", file.length - ".o".length) !== -1) || 
            (file.indexOf(".bin", file.length - ".bin".length) !== -1) || 
            (file.indexOf(".elf", file.length - ".elf".length) !== -1) || 
            (file.indexOf(".map", file.length - ".map".length) !== -1) || 
            (file.indexOf(".hex", file.length - ".hex".length) !== -1)) {

            fs.unlinkSync(fPath+"/"+file);
            console.log("Removed file "+project+"/"+file);
        }
    });

    var fPathLibs = __dirname+"/_libs_";
    fPathLibs = path.resolve(fPathLibs);

    var dirLibs = walk(fPathLibs);

    dirLibs.forEach(function (file) {
        if ((file.indexOf(target+".d", file.length - target+".d".length) !== -1) || 
            (file.indexOf(target+".o", file.length - target+".o".length) !== -1)) {

            fs.unlinkSync(fPathLibs+"/"+file);
            console.log("Removed file _libs_/"+file);
        }
    });

}

