var addon = require('djondb');
//var addon = require('./build/Release/djondb');

console.log(addon);

var manager = new addon.WrapConnectionManager();
var con = manager.getConnection("localhost");
con.open();

con.dropNamespace("test_nodejs", "nodens");
con.insert("test_nodejs", "nodens", { name: "Test" });

var res = con.find("test_nodejs", "nodens");

console.log(res);

var guid = res[0]._id;
var res2 = con.findByKey("test_nodejs", "nodens", guid);

console.log("res2");
console.log(res2);

manager.releaseConnection(con);
