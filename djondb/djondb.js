const net = require('net');
const uuidV4 = require('uuid/v4');

var COMMANDTYPE = {
   INSERT: 0,
   UPDATE: 1,
   FIND: 2,
   CLOSECONNECTION: 3,
   DROPNAMESPACE: 4,
   SHUTDOWN: 5,
   SHOWDBS: 6,
   SHOWNAMESPACES: 7,
   REMOVE: 8,
   COMMIT: 9,
   ROLLBACK: 10,
   FETCHCURSOR: 11,
   FLUSHBUFFER: 12,
   CREATEINDEX: 13,
   BACKUP: 14,
   RCURSOR: 15,
   PERSISTINDEXES: 16,
   EXECUTEQUERY: 17,
   EXECUTEUPDATE: 18
};

var CURSORSTATUS = {
   CS_LOADING: 1,
   CS_RECORDS_LOADED: 2,
   CS_CLOSED: 3
};

function DjondbConnection(host, port) {
   this.host = host;
   this.port = port;
};

function DjondbCursor(command, cursorId, firstPage) {
   this._command = command;
   this._cursorId = cursorId;
   this._rows = firstPage;
   this._position = 0;
   this._current = null;

   if (this._rows == null) {
      this._count = 0;
   } else {
      this._count = this._rows.length;
   }

   if (cursorId != null) {
      this._status = CURSORSTATUS.CS_LOADING;
   } else {
      this._status = CURSORSTATUS.CS_RECORDS_LOADED;
   }
};

DjondbCursor.prototype = {
   next: function(callback) {
      var self = this;
		if (self._status == CURSORSTATUS.CS_CLOSED) {
			throw "Cursor is closed";
		}
		var result = true;
		if (self._count > self._position) {
			self._current = self._rows[self._position];
			self._position += 1;
         if (callback) {
            callback.apply(this, [result]);
         }
		} else {
			if (self._status == CURSORSTATUS.CS_LOADING) {
            this._command.fetchRecords(self._cursorId, (error, page) => {
               if (error) {
                  if (callback) {
                     callback.apply(this, [error, null]);
                  } else {
                     return;
                  }
               }
               if (!page) {
                  self._status = CURSORSTATUS.CS_RECORDS_LOADED;
                  result = false;
               } else {
                  self._rows = self._rows.concat(page);
                  self._count = self._rows.length;
                  result = self.next(callback);
               }
               if (callback) {
                  callback.apply(this, [error, result]);
               }
            });
         } else {
            result = false;
            if (callback) {
               callback.apply(this, [result]);
            }
         }
      }
   },

   nextPage: function(callback) {
      var self = this;
      if (self._status == CURSORSTATUS.CS_CLOSED) {
         throw "Cursor is closed";
      }
      if (self._status == CURSORSTATUS.CS_LOADING) {
         this._command.fetchRecords(self._cursorId, (error, page) => {
            if (error) {
               if (callback) {
                  callback.apply(this, [error, null]);
               } else {
                  return;
               }
            }
            if (!page) {
               self._status = CURSORSTATUS.CS_RECORDS_LOADED;
            } else {
               self._rows = self._rows.concat(page);
               self._count = self._rows.length;
               self._pos += page.length;
            }
            if (callback) {
               callback.apply(this, [undefined, page]);
            }
         });
      } else {
         if (callback) {
            callback.apply(this, [null]);
         }
      }
   },

   previous: function() {
      var self = this;
      if (self._status == CURSORSTATUS.CS_CLOSED) {
         throw new DjondbException("Cursor is closed");
      }
      var result = true;
      if ((self._count > 0) && (self._position > 0)) {
         self._position -= 1;
         self._current = self._rows[self._position];
      } else {
         result = false;
      }
      return result;
   },

   current: function() {
      var self = this;
      return self._current;
   }
};

function Command(netInput, netOutput) {
   var self = this;
   self._netInput = netInput;
   self._netOutput = netOutput;
   self._activeTransactionId = undefined;
   self._resultCode = undefined;
   self._resultMessage = undefined;
   self.callbacks = [];
   self.executingCommand = false;
   self.pendingCommands = [];
};

Command.prototype = {
   pushCallback: function(callback) {
      var self = this;
      self.callbacks.push(callback);
   },

   pushPendingCommand: function(command, parameters) {
      var self = this;
      var pendingCommand = { command: command, parameters: parameters };
      self.pendingCommands.push(pendingCommand);
   },

   dataReady: function(data, len) {
      var self = this;
      self._netInput.append(data, len);
      while (self._netInput.available() && (self.callbacks.length > 0)) {
         var callback = self.callbacks.shift();
         callback(data);
      }
   },

   executePendingCommand: function() {
      var self = this;
      if (self.pendingCommands.length > 0) {
         var pendingCommand = self.pendingCommands.shift();
         pendingCommand.command.apply(this, pendingCommand.parameters );
      }
   },

   clearError: function() {
      var self = this;
      self._resultCode = undefined;
      self._resultMessage = undefined;
   },

   checkError: function() {
      var self = this;

      if (self._resultCode > 0) {
         var resultCode = self._resultCode;
         var resultMessage = self._resultMessage;
         self.clearError();
         return {
            resultCode: resultCode,
            message: resultMessage,
            toString: function() { return "Error: " + this.resultCode + ". Description: " + this.message }
         };
      } else {
         return undefined;
      }
   },

   writeHeader: function(commandType) {
      var self = this;
      self.executingCommand = true;
      self._netOutput.reset();
      var version = "3.5.60822";
      self._netOutput.writeString(version);
      self._netOutput.writeInt(commandType);
      self.writeOptions(self._netOutput);
   },

   writeOptions: function() {
      var self = this;
      var options = {};
      if (self._activeTransactionId != undefined) {
         options["_transactionId"] = self._activeTransactionId;
      }
      self._netOutput.writeBSON(options);
   },

   readErrorInformation: function() {
      var self = this;
      var resultCode = self._netInput.readInt();
      var resultMessage = undefined;
      if (resultCode > 0) {
         resultMessage = self._netInput.readString();
      }
      if (!self._resultCode) {
         self._resultCode = resultCode;
         self._resultMessage = resultMessage;
      }
   },

   readResults: function(processFunction, data, callback) {
      var self = this;

      var result = processFunction.apply(this);

      self.readErrorInformation(self._netInput);
      var error = self.checkError();
      if (callback) callback(error, result);
      self._netInput.reset();
      self.executingCommand = false;
      self.executePendingCommand();
   },

   processShowDbsResult: function() {
      var self = this;
      var results = self._netInput.readInt();

      dbs = [];
      for (var x = 0; x < results; x++) {
         dbs.push(self._netInput.readString());
      }
      return dbs;
   },

   showDbs: function(callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.showDbs, [callback]);
      } else {
         self.writeHeader(COMMANDTYPE.SHOWDBS);

         self.pushCallback(function(data) {
            self.readResults(self.processShowDbsResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processShowNamespacesResult: function() {
      var self = this;
      var results = self._netInput.readInt();
      dbs = [];
      for (var x = 0; x < results; x++) {
         dbs.push(self._netInput.readString());
      }

      return dbs;
   },

   showNamespaces: function(db, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.showNamespaces, [db, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.SHOWNAMESPACES);

         self._netOutput.writeString(db);
         self.pushCallback(function(data) {
            self.readResults(self.processShowNamespacesResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processInsertResult: function() {
      var self = this;
      var result = self._netInput.readInt();

      return result;
   },

   insert: function(db, ns, obj, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.insert, [db, ns, obj, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.INSERT);
         self._netOutput.writeString(db);
         self._netOutput.writeString(ns);
         self._netOutput.writeBSON(obj);
         self.pushCallback(function(data) {
            self.readResults(self.processInsertResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processUpdateResult: function() {
      var self = this;
      var result = self._netInput.readBoolean();

      return result;
   },

   update: function(db, ns, obj, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.update, [db, ns, obj, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.UPDATE);

         self._netOutput.writeString(db);
         self._netOutput.writeString(ns);
         self._netOutput.writeBSON(obj);
         self.pushCallback(function(data) {
            self.readResults(self.processUpdateResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processFindResult: function() {
      var self = this;
      var cursorId = self._netInput.readString();
      var flag = self._netInput.readInt();
      var results = [];
      if (flag == 1) {
         results = self._netInput.readBSONArray();
      }

      result = new DjondbCursor(this, cursorId, results);
      return result;
   },

   find: function(db, ns, select, filter, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.find, [db, ns, select, filter, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.FIND); // find command

         self._netOutput.writeString(db);
         self._netOutput.writeString(ns);
         self._netOutput.writeString(filter);
         self._netOutput.writeString(select);
         self.pushCallback(function(data) {
            self.readResults(self.processFindResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processFetchRecordsResult: function()  {
      var self = this;
      var flag = self._netInput.readInt();
      var results = null;
      if (flag == 1) {
         results = self._netInput.readBSONArray();
      }

      return results;
   },

   fetchRecords: function(cursorId, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.fetchRecords, [cursorId, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.FETCHCURSOR); // fetch command

         self._netOutput.writeString(cursorId);
         self.pushCallback(function(data) {
            self.readResults(self.processFetchRecordsResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   beginTransaction: function() {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.beginTransaction, []);
      } else {
         self._activeTransactionId = uuidV4();
         self.executingCommand = false;
         self.executePendingCommand();
      }
   },

   processCommitTransactionResult: function()  {
      var self = this;

      return null;
   },

   commitTransaction: function(callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.commitTransaction, [callback]);
      } else {
         self.executingCommand = true;
         if (self._activeTransactionId) {
            self.writeHeader(COMMANDTYPE.COMMIT);

            self._netOutput.writeString(self._activeTransactionId);
            self._activeTransactionId = null;
            self.pushCallback(function(data) {
               self.readResults(self.processCommitTransactionResult, data, callback);
            });
            self._netOutput.flush();
         } else {
            throw 'Nothing to commit, you need beginTransaction before committing or rollback';
         }
      }
   },

   processRollbackTransactionResult: function()  {
      var self = this;

      return null;
   },

   rollbackTransaction: function(callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.rollbackTransaction, [callback]);
      } else {
         self.executingCommand = true;
         if (self._activeTransactionId) {
            self.writeHeader(COMMANDTYPE.ROLLBACK);

            self._netOutput.writeString(self._activeTransactionId);
            self._activeTransactionId = null;
            self.pushCallback(function(data) {
               self.readResults(self.processRollbackTransactionResult, data, callback);
            });
            self._netOutput.flush();
         } else {
            throw 'Nothing to commit, you need beginTransaction before committing or rollback';
         }
      }
   },

   processCreateIndexResult: function()  {
      var self = this;

      return null;
   },

   createIndex: function(indexDef, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.createIndex, [indexDef, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.CREATEINDEX);

         self._netOutput.writeBSON(indexDef);
         self.pushCallback(function(data) {
            self.readResults(self.processCreateIndexResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processBackupResult: function()  {
      var self = this;

      var result = self._netInput.readInt();

      return result;
   },

   backup: function(db, destFile, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.backup, [db, destFile, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.BACKUP);

         self._netOutput.writeString(db);
         self._netOutput.writeString(destFile);
         self.pushCallback(function(data) {
            self.readResults(self.processBackupResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   createStandardOutputForNonCursorResult: function(success) {
      var self = this;

      var result = {
         date: new Date(),
         success: success
      }

      return result;
   },

   wrapDbsResultAsCursor: function(dbs) {
      var self = this;

      var dbsArray = [];
      for (var x = 0; x < dbs.length; x++) {
         var dbObject = {
            db: dbs[x]
         };
         dbsArray.push(dbObject);
      }

      return new DjondbCursor(null, null, dbsArray);
   },

   wrapNamespacesResultAsCursor: function(nss) {
      var self = this;

      var nssArray = [];
      for (var x = 0; x < nss.length; x++) {
         var nsObject = {
            ns: nss[x]
         };
         nssArray.push(nsObject);
      }

      return new DjondbCursor(null, null, nssArray);
   },

   processExecuteQueryResult: function()  {
      var self = this;

      var flag = self._netInput.readInt();
      var result;

      if (flag == 1) {
         var commandType = self._netInput.readInt();
         if (commandType == COMMANDTYPE.INSERT) {
            result = self.processInsertResult();
            result = self.createStandardOutputForNonCursorResult(result == 1);
         } else if (commandType == COMMANDTYPE.UPDATE) {
            result = self.processUpdateResult(data, callback);
            result = self.createStandardOutputForNonCursorResult(result == 1);
         } else if (commandType == COMMANDTYPE.FIND) {
            result = self.processFindResult();
         } else if (commandType == COMMANDTYPE.DROPNAMESPACE) {
            result = self.processDropNamespaceResult();
            result = self.createStandardOutputForNonCursorResult(result == 1);
         } else if (commandType == COMMANDTYPE.SHOWDBS) {
            result = self.processShowDbsResult();
            result = self.wrapDbsResultAsCursor(result);
         } else if (commandType == COMMANDTYPE.SHOWNAMESPACES) {
            result = self.processShowNamespacesResult();
            result = self.wrapNamespacesResultAsCursor(result);
         } else if (commandType == COMMANDTYPE.REMOVE) {
         }
         // Execute is a wrapper therefore the first error has to be discarded
         self.readErrorInformation(self._netInput);
      } else {
         self.readErrorInformation(self._netInput);
      }
      return result;
   },

   executeQuery: function(query, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.executeQuery, [query, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.EXECUTEQUERY);

         self._netOutput.writeString(query);
         self.pushCallback(function(data) {
            self.readResults(self.processExecuteQueryResult, data, callback);
         });
         self._netOutput.flush();
      }
   },

   processDropNamespaceResult: function() {
      var self = this;
      var result = self._netInput.readInt();

      return result;
   },

   dropNamespace: function(db, ns, callback) {
      var self = this;
      if (self.executingCommand) {
         self.pushPendingCommand(self.dropNamespace, [db, ns, callback]);
      } else {
         self.writeHeader(COMMANDTYPE.DROPNAMESPACE);

         self._netOutput.writeString(db);
         self._netOutput.writeString(ns);
         self.pushCallback(function(data) {
            self.readResults(self.processDropNamespaceResult, data, callback);
         });
         self._netOutput.flush();
      }
   }
}

function NetworkInput(client) {
   var self = this;
   self.bufferPos = 0;
   self.bufferLen = 0;

   var init = function(client) {
      self.client = client;
      self.buffer = new BufferWrapper(1024*100);
   };
   init(client);
}

NetworkInput.prototype = {
   reset: function() {
      var self = this;
      self.buffer = new BufferWrapper(1024*100);
      self.bufferLen = 0;
      self.bufferPos = 0;
   },

   available: function() {
      var self = this;
      return (self.bufferLen > self.bufferPos);
   },

   append: function(data, len) {
      var self = this;

      /*
      for (var x = 0; x < len; x++) {
         self.buffer[self.bufferPos + x] = data.charCodeAt(x);
      }
      */
      self.buffer.write(data, self.bufferPos, len, 'hex');
      self.bufferLen += len;
   },

   readBoolean: function() {
      var self = this;
      var i = self.buffer.readChar(self.bufferPos);
      var b = (i == 1);
      self.bufferPos += 1;
      return b;
   },

   readInt: function() {
      var self = this;
      var i = self.buffer.readInt(self.bufferPos);
      self.bufferPos += 4;
      return i;
   },

   readLong: function() {
      var self = this;
      var lower = self.readInt();
      var higher = self.readInt();
      var result = (higher << 32) | lower;
      return result;
   },

   readString: function() {
      var self = this;
      var len = self.readInt();
      var res = self.buffer.read(self.bufferPos, self.bufferPos + len);
      self.bufferPos += len;
      return res;
   },

   readBSON: function() {
      var self = this;
      // temporal implementation
      var len = self.readLong();
      var result = {};
      for (var x = 0; x < len; x++) {
         var key = self.readString();
         var type = self.readLong();

         var value;
         if (type == 0) {
            value = self.readInt();
         } else if (type == 1) {
            value = self.readDouble();
         } else if (type == 2) {
            value = self.readLong();
         } else if (type == 4) {
            value = self.readString();
         } else if (type == 5) {
            value = self.readBSON();
         } else if (type == 6) {
            value = self.readBSONArray();
         } else if (type == 10){
            value = self.readBoolean();
         }
         result[key] = value;
      };

      return result;
   },

   readBSONArray: function() {
      var self = this;

      var len = self.readLong();
      var result = [];
      for (var x = 0; x < len; x++) {
         var o = self.readBSON();
         result.push(o);
      }

      return result;
   },

   on: function(ev, callback) {
      var self = this;
      self.client.on('data', (data) => {
         var len = self.client.bytesRead;
         callback(data, len);
      });
   }

}

function BufferWrapper(size) {
   var self = this;
   self._bufferSize = size;
   self._buffer = new Buffer(1024*100, 'hex');
   self._buffer.fill(0);
}

BufferWrapper.prototype = {
   writeInt: function(i, pos) {
      var self = this;
      self._buffer.writeInt32LE(i, pos);
   },

   write: function(s, pos, len) {
      var self = this;
      self._buffer.write(s, pos, len, 'hex');
   },

   getBufferData: function(len) {
      var self = this;
      /*
      console.log("getBufferData");
      var result = "";
      for (var x = 0; x < len; x++) {
         result += String.fromCharCode(self._buffer[x]);
      }
      */
      var result = self._buffer.toString("utf8", 0, len);
      return result;
   },

   writeChar: function(c) {
      var self = this;
      self._buffer[self.bufferPos] = c;
   },

   readChar: function(pos) {
      var self = this;
      return self._buffer[pos];
   },

   readInt: function(pos) {
      var self = this;
      return self._buffer.readInt32LE(pos);
   },

   read: function(initPos, finalPos) {
      var self = this;
      var res = self._buffer.toString('utf-8', initPos, finalPos);
      return res;
   }

}

function NetworkOutput(client) {
   var self = this;
   self.bufferPos = 0;
   self.bufferLen = 0;

   var init = function(client) {
      self.client = client;
      self.buffer = new BufferWrapper(1024*100, 'hex');
   };
   init(client);
}

NetworkOutput.prototype = {
   reset: function() {
      var self = this;
      self.buffer = new BufferWrapper(1024*100);
      self.bufferLen = 0;
      self.bufferPos = 0;
   },

   writeBSON: function(o) {
      var self = this;
      if (typeof(o) != "object") {
         throw "Illegal Object";
      }
      // temporal implementation
      var len = 0;
      for (var prop in o) {
         len++;
      }
      self.writeLong(len);
      for (var key in o) {
         self.writeString(key);
         var value = o[key];
         var type = typeof(value);
         if (type == "number") {
            // check if Integer
            if (parseInt(value) == value) {
               self.writeLong(0);
               self.writeInt(value);
            } else {
               self.writeLong(1);
               self.writeDouble(value);
            }
            // Long does not exist, check how to know the size
         } else if (type == "long") {
            self.writeLong(2);
            self.writeLong(value);
         } else if (type == "string") {
            self.writeLong(4);
            self.writeString(value);
         } else if (type == "object") {
            if (Array.isArray(value)) {
               self.writeLong(6);
               self.writeBSONArray(value);
            } else {
               self.writeLong(5);
               self.writeBSON(value);
            }
            // there's no type array
         } else if (type == "array") {
         } else if (type == "boolean") {
            self.writeLong(10);
            self.writeBoolean(value);
         } 
      }
   },

   writeBSONArray: function(a) {
      var self = this;

      var len = a.length;
      self.writeLong(len);
      for (var x = 0; x < len; x++) {
         var o = a[x];
         self.writeBSON(o);
      }
   },

   writeString: function(s) {
      var self = this;
      self.writeInt(s.length);
      self.buffer.write(s, self.bufferPos);
      self.bufferPos += s.length;
      self.bufferLen += s.length;
   },

   writeLong: function(l) {
      var self = this;
      var highMap = 0xffffffff00000000;
      var lowMap = 0x00000000ffffffff;
      var higher = (l & highMap) >> 32;
      var lower = (l & lowMap);
      self.writeInt(lower);
      self.writeInt(higher);
   },

   writeBoolean: function(b) {
      var self = this;
      var c;
      if (b) {
         c = String.fromCharCode(1);
      } else {
         c = String.fromCharCode(0);
      }
      self.buffer.writeChar(c);
      self.bufferPos += 1;
      self.bufferLen += 1;
   },

   writeInt: function(i) {
      var self = this;
      self.buffer.writeInt(i, self.bufferPos);
      self.bufferPos += 4;
      self.bufferLen += 4;
   },

   flush: function() {
      var self = this;
      /*
       * encoding to utf8 is creating an unexpected behaviour
       * for reference check the test/testEndianess
      var data = self.buffer.toString('utf8', 0, self.bufferLen);
      */
      var result = self.buffer.getBufferData(self.bufferLen);
      self.client.write(result);
      self.reset();
   }
};

DjondbConnection.prototype = {
   open: function(callback) {
      var self = this;
      self.error = undefined;
      self.client = net.createConnection({ port: self.port, host: self.host}, () => {
         self.client.setEncoding('hex');
         self.connected = true;
         self._netInput = new NetworkInput(self.client);
         self._netOutput = new NetworkOutput(self.client);
         self.command = new Command(self._netInput, self._netOutput);
         self._netInput.on('data', (data, len) => {
            self.command.dataReady(data, len);
         });
         if (callback) callback.apply(this);
      });
   },

   showDbs: function(callback) {
      var self = this;
      self.command.showDbs((error, dbs) => {
         if (callback) {
            callback(error, dbs);
         }
      });
   },

   showNamespaces: function(db, callback) {
      var self = this;

      if (!db) {
         throw "You must provide a db name";
      }

      self.command.showNamespaces(db, (error, namespaces) => {
         if (callback) {
            callback(error, namespaces);
         }
      });
   },

   insert: function(db, ns, obj, callback) {
      var self = this;
      self.command.insert(db, ns, obj, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   update: function(db, ns, obj, callback) {
      var self = this;
      self.command.update(db, ns, obj, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   find: function(db, ns, select, filter, callback) {
      var self = this;
      self.command.find(db, ns, select, filter, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   fetchRecords: function(cursorId, callback) {
      var self = this;
      self.command.fetchRecords(cursorId, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   beginTransaction: function(cursorId, callback) {
      var self = this;
      self.command.beginTransaction();
   },

   commitTransaction: function(callback) {
      var self = this;
      self.command.commitTransaction((error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   rollbackTransaction: function(callback) {
      var self = this;
      self.command.rollbackTransaction((error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   createIndex: function(indexDef, callback) {
      var self = this;
      self.command.createIndex(indexDef, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   backup: function(db, destFile, callback) {
      var self = this;
      self.command.backup(db, destFile, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   executeQuery: function(query, callback) {
      var self = this;
      self.command.executeQuery(query, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   },

   dropNamespace: function(db, ns, callback) {
      var self = this;
      self.command.dropNamespace(db, ns, (error, result) => {
         if (callback) {
            callback(error, result);
         }
      });
   }

};

var djondb = {
   DjondbConnection: DjondbConnection
};

module.exports = djondb;
