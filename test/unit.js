var djondb = require('../djondb');
var assert = require('assert');

console.log("connecting");
var con = new djondb.DjondbConnection('localhost');

function collectAllFromCursor(cursor, callback) {
   var self = this;
   if (self.collectCallBack) {
      console.log("a pending collectcallback will be overriden");
      assert.fail();
   }
   self.collectCallBack = callback;
   var records = [];
   var nextPage = function() {
      cursor.nextPage(function(error, page) {
         assert.ifError(error);
         if (page) {
            records = records.concat(page);
            nextPage();
         } else {
            var originalCallback = self.collectCallBack;
            originalCallback(records);
            delete self.collectCallBack;
         }
      });
   };
   nextPage();
}

function helperFind(db, ns, callback) {
   con.find(db, ns, "*", "", function(error, cursor) {
      assert.ifError(error);
      return collectAllFromCursor(cursor, callback);
   });
}

function testInsertFind(callback) {
   console.log("testInsertFind");
   con.dropNamespace("testdb", "testns", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testns", { "name": "JohnInsert", "lastName": "Smith", "age": 10, "salary": 150000.12, "b": true}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testns", function(records) {
            assert.equal(1, records.length);
            assert.equal(true, records[0].b);
            callback();
         });
      });
   });
}

function testCursor(callback) {
   console.log("testCursor");
   con.dropNamespace("testdb", "testcursor", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testcursor", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
         assert.ifError(error);
         con.insert("testdb", "testcursor", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
            assert.ifError(error);
            var result = [];
            con.find("testdb", "testcursor", "*", "", (err, cursor) => {
               cursor.next((err, hasNext) => {
                  assert.ifError(err);
                  assert.equal(true, hasNext);
                  var element = cursor.current();
                  assert.equal(true, element != undefined);
                  result.push(element);
                  assert.equal(result.length, 1);
                  callback();
               });
            });
         });
      });
   });
}

function testEndianess(callback) {
   console.log("testEndianess");
   //var expected = "\xa9\xf3\x00\x00";
   var expected = "\xa9\xf3\x00\x00" + // 62377
      "\xb2\x00\x00\x00"; // 178

   var buffer = new Buffer(1024*100, 'hex');
   buffer.writeInt32LE(62377, 0);
   buffer.writeInt32LE(178, 4);

   var expect1 = buffer.readInt32LE(0);
   assert.equal(expect1, 62377);
   var expect2 = buffer.readInt32LE(4);
   assert.equal(expect2, 178);

   var data = buffer.toString('utf8', 0, 16);
   var numbers = "";
   for (var x = 0; x < 16; x++) {
      numbers += " " + data.charCodeAt(x);
   }
   console.log("data              " + numbers);
   var numbersExpected = "";
   for (var x = 0; x < 8; x++) {
      numbersExpected += " " + expected.charCodeAt(x);
   }
   console.log("expected :        " + numbersExpected);

   var resultByConcatenation = "";
   for (var x = 0; x < 8; x++) {
      resultByConcatenation += String.fromCharCode(buffer[x]);
   }
   var numbersByConcat = "";
   for (var x = 0; x < 8; x++) {
      numbersByConcat += " " + resultByConcatenation.charCodeAt(x);
   }
   console.log("resultbycontact : " + numbersExpected);

   assert.equal(resultByConcatenation, expected);
   // rhia one always fails, that's why I'd to implement it using concat
   //assert.equal(data, expected);
   callback();
   //   '\xa9\xf3\x00\x00\x00\x00\x00\x00'
}

function testInsertHighNumbers(callback) {
   console.log("testInsertHighNumbers");
   con.dropNamespace("testdb", "testhn", function(error) {
      assert.ifError(error);
      var salary = 62377;
      con.insert("testdb", "testhn", { "name": "JohnInsert", "lastName": "Smith", "salary": salary}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testhn", function(records) {
            assert.equal(1, records.length);
            assert.equal(salary, records[0].salary);
            callback();
         });
      });
   });
}

function testInsertFindUpdate(callback) {
   console.log("testInsertFindUpdate");
   con.dropNamespace("testdb", "testinsupd", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testinsupd", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testinsupd", function(records) {
            assert.equal(1, records.length);
            records[0].age = 20;
            con.update("testdb", "testinsupd", records[0], function(error) {
               assert.ifError(error);
               con.find("testdb", "testinsupd", "*", "age = 20", function(error, cursor) {
                  assert.ifError(error);
                  collectAllFromCursor(cursor, function(results) {
                     assert.equal(1, results.length);
                     callback();
                  });
               });
            });
         });
      });
   });
}

function testInsertFindRemove(callback) {
   console.log("testInsertFindRemove");
   con.dropNamespace("testdb", "testinsupdrem", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testinsupdrem", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testinsupdrem", function(records) {
            assert.equal(1, records.length);
            con.remove("testdb", "testinsupdrem", records[0]["_id"], records[0]["_revision"], function(error) {
               assert.ifError(error);
               con.find("testdb", "testinsupdrem", "*", "", function(error, cursor) {
                  assert.ifError(error);
                  collectAllFromCursor(cursor, function(results) {
                     assert.equal(0, results.length);
                     callback();
                  });
               });
            });
         });
      });
   });
}

function testInsertFindUpdateWithDQL(callback) {
   console.log("testInsertFindUpdateWithDQL");
   con.executeQuery("drop namespace 'testdb', 'testinsupddql'", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testinsupddql", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testinsupddql", function(records) {
            assert.equal(1, records.length);
            records[0].age = 20;
            var updatedql = "update " + JSON.stringify(records[0]) + " into testdb:testinsupddql";

            con.executeQuery(updatedql, function(error) {
               console.log("executeQuery");
               assert.ifError(error);
               con.executeQuery("select * from testdb:testinsupddql where age = 20", function(error, cursor) {
                  assert.ifError(error);
                  collectAllFromCursor(cursor, function(results) {
                     assert.equal(1, results.length);
                     callback();
                  });
               });
            });
         });
      });
   });
}

function testShows(callback) {
   console.log("testShows");
   con.showDbs(function(error, dbs) {
      assert.ifError(error);
      var dbResult = {};
      for (var i = 0; i < dbs.length; i++) {
         db = dbs[i];
         dbResult[db] = [];
         con.showNamespaces(db, function(error, nss) {
            assert.ifError(error);
            for (var j = 0; j < nss.length; j++) {
               var ns = nss[j];
               dbResult[db].push(ns);
            };
         });
      };
      callback();
   });
}

function testPagedFind(callback) {
   var self = this;
   console.log("testPagedFind");
   con.dropNamespace("testdb", "pagedfind", function(error) {
      assert.ifError(error);
      var find = function() {
         helperFind("testdb", "pagedfind", function(result) {
            assert.equal(result.length, 100);
            callback();
         });
      }
      var insert = function(error, result) {
         assert.ifError(error);
         if (!self.count) self.count = 0;
         self.count++;
         if (self.count <= 100) {
            con.insert("testdb", "pagedfind", { "name": "JohnPaged", "lastName": "Smith", "age": 10}, insert);
         } else {
            find();
         }
      }
      insert();
   });
}

function testTXCommit(callback) {
   console.log("testTXCommit");
   con.beginTransaction();

   con.dropNamespace("testdb", "testtx", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testtx", { "name": "JohnX", "lastName": "SmithX" }, function(error, result) {
         assert.ifError(error);
         helperFind("testdb", "testtx", function(results) {
            assert.equal(1, results.length);

            con.commitTransaction(function(error) {
               assert.ifError(error);
               helperFind("testdb", "testtx", function(results) {
                  assert.equal(1, results.length);
                  callback();
               });
            });
         });
      });
   });
}

function testTXRollback(callback) {
   console.log("testTXRollback");
   con.beginTransaction();

   con.dropNamespace("testdb", "testtxrollback", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testtxrollback", { "name": "JohnRollback", "lastName": "Smith" }, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testtxrollback", function(results) {
            assert.equal(1, results.length);

            con.rollbackTransaction(function(error) {
               assert.ifError(error);
               helperFind("testdb", "testtxrollback", function(results) {
                  assert.equal(0, results.length);
                  callback();
               });
            });
         });
      });
   });
}

function testIndex(callback) {
   console.log("testIndex");
   con.dropNamespace("testdb", "testindex", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testindex", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testindex", function(records) {
            assert.equal(1, records.length);
            var indexDef = {
               db: "testdb",
               ns: "testindex",
               name: "test",
               fields: [
                  { path: "name" }
               ]
            };
            con.createIndex(indexDef, function(error) {
               assert.ifError(error);
               con.find("testdb", "testindex", "*", "name = 'John'", function(error, cursor) {
                  assert.ifError(error);
                  cursor.nextPage(function(error, page) {
                     assert.ifError(error);
                     if (page) {
                        rows = page.length;
                     } else {
                        rows = 0;
                     }
                     assert.equal(1, records.length);
                     callback();
                  });
               });
            });
         });
      });
   });
}

function testIndexDQL(callback) {
   console.log("testIndexDQL");
   con.dropNamespace("testdb", "testindexDQL", function(error) {
      assert.ifError(error);
      con.insert("testdb", "testindexDQL", { "name": "JohnInsert", "lastName": "Smith", "age": 10}, function(error) {
         assert.ifError(error);
         helperFind("testdb", "testindexDQL", function(records) {
            assert.equal(1, records.length);
            var indexDef = {
               db: "testdb",
               ns: "testindex",
               name: "test",
               fields: [
                  { path: "name" }
               ]
            };
            con.executeQuery("CREATE INDEX test on testdb:testindexDQL using (name)", function(error) {
               assert.ifError(error);
               con.find("testdb", "testindexDQL", "*", "name = 'John'", function(error, cursor) {
                  assert.ifError(error);
                  cursor.nextPage(function(error, page) {
                     assert.ifError(error);
                     if (page) {
                        rows = page.length;
                     } else {
                        rows = 0;
                     }
                     assert.equal(1, records.length);
                     callback();
                  });
               });
            });
         });
      });
   });
}

function testExecuteQuery(callback) {
   console.log("testExecuteQuery");
   con.executeQuery("drop namespace 'testdb', 'testexecutequery'", function(error, resDrop) {
      var data = { "name": "JohnInsert", "lastName": "Smith", "age": 10}; 
      var insertDQL = "insert " + JSON.stringify(data) + " into testdb:testexecutequery";

      con.executeQuery(insertDQL, function() {
         con.executeQuery("select * from testdb:testexecutequery", function(error, cursor) {
            collectAllFromCursor(cursor, function(records) {
               assert.equal(1, records.length);
               callback();
            });
         });
      });
   });
}

function testExecuteUpdate(callback) {
   console.log("testExecuteUpdate");
   con.executeUpdate("drop namespace 'testdb', 'testexecutequery'", function(error, resDrop) {
      assert.ifError(error);
      var data = { "name": "JohnInsert", "lastName": "Smith", "age": 10}; 
      var insertDQL = "insert " + JSON.stringify(data) + " into testdb:testexecutequery";

      con.executeUpdate(insertDQL, function(error, res) {
         assert.ifError(error);
         con.executeUpdate("select * from testdb:testexecutequery", function(error, res) {
            // select statements are not allowed under executeUpdate
            assert.equal(true, error != undefined);
            assert.equal(601, error.resultCode);
            callback();
         });
      });
   });
}

function testBackup(callback) {
   console.log("testBackup");
   con.backup("testdb", "/tmp/testdb.tar", function(res) {
      callback();
   });
}

function testEmptyQueryError(callback) {
   console.log("testEmptyQueryError");
   con.executeQuery("", function(error, res) {
      assert.equal(true, error != undefined);
      assert.equal(602, error.resultCode);
      callback();
   });
}

function executeTests(tests) {
   var i = 0;
   var executeNextTest = function() {
      if (i < tests.length) {
         var test = tests[i];
         i++;
         test(executeNextTest);
      } else {
         console.log("completed");
         con.close();
      }

   }
   executeNextTest(tests);
}

con.open(function() {
   var tests = [];
   tests.push(testEndianess);
   tests.push(testInsertHighNumbers);
   tests.push(testEmptyQueryError);
   tests.push(testInsertFind);
   tests.push(testShows);
   tests.push(testPagedFind);
   tests.push(testTXCommit);
   tests.push(testTXRollback);
   tests.push(testIndex);
   tests.push(testIndexDQL);
   tests.push(testInsertFindUpdate);
   tests.push(testInsertFindUpdateWithDQL);
   tests.push(testExecuteQuery);
   tests.push(testExecuteUpdate);
   tests.push(testInsertFindRemove);
   tests.push(testCursor);
   /*
   tests.push(testBackup);
   */
   executeTests(tests);
});
   /*


   con.backup("testdb", "testns", function() {
      console.log("backup created");
   });
});
*/
