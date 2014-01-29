/*
 * =====================================================================================
 *
 *       Filename:  djondbconnection.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/12/2012 08:56:01 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Juan Pablo Crossley (Cross), crossleyjuan@gmail.com
 *   Organization:  djondb
 *
 * This file is part of the djondb project, for license information please refer to the LICENSE file,
 * the application and libraries are provided as-is and free of use under the terms explained in the file LICENSE
 * Its authors create this application in order to make the world a better place to live, but you should use it on
 * your own risks.
 * 
 * Also, be adviced that, the GPL license force the committers to ensure this application will be free of use, thus
 * if you do any modification you will be required to provide it for free unless you use it for personal use (you may 
 * charge yourself if you want), bare in mind that you will be required to provide a copy of the license terms that ensures
 * this program will be open sourced and all its derivated work will be too.
 * =====================================================================================
 */
#include "wrapconnection.h"
#include "wrapcursor.h"
#include "djondb_client.h"
#include "v8.h"
#include "nodeutil.h"
#include <string>
#include <sstream>

using namespace v8;
using namespace djondb;

Persistent<Function> WrapConnection::constructor;

// Extracts a C string from a V8 Utf8Value.
v8::Handle<v8::Value> parseJSON(v8::Handle<v8::Value> object)
{
	v8::HandleScope scope;

	v8::Handle<v8::Context> context = v8::Context::GetCurrent();
	v8::Handle<v8::Object> global = context->Global();

	v8::Handle<v8::Object> JSON = global->Get(v8::String::New("JSON"))->ToObject();
	v8::Handle<v8::Function> JSON_parse = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::New("parse")));

	return scope.Close(JSON_parse->Call(JSON, 1, &object));
}

v8::Handle<v8::Value> toJson(v8::Handle<v8::Value> object, bool beautify = false)
{
	v8::HandleScope scope;

	v8::Handle<v8::Context> context = v8::Context::GetCurrent();
	v8::Handle<v8::Object> global = context->Global();

	v8::Handle<v8::Object> JSON = global->Get(v8::String::New("JSON"))->ToObject();
	v8::Handle<v8::Function> JSON_stringify = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::New("stringify")));

	if (!beautify) {
		return scope.Close(JSON_stringify->Call(JSON, 1, &object));
	} else {
		v8::Handle<v8::Value> args[3];
		args[0] = object;
		args[1] = v8::Null();
		args[2] = v8::Integer::New(4);
		return scope.Close(JSON_stringify->Call(JSON, 3, args));
	}
}

WrapConnection::WrapConnection() {}
WrapConnection::~WrapConnection() {}

void WrapConnection::Init(Handle<Object> target) {
	//Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("WrapConnection"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//Prototype
	//global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	tpl->PrototypeTemplate()->Set(String::NewSymbol("open"),
			FunctionTemplate::New(open)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("close"),
			FunctionTemplate::New(close)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("isOpen"),
			FunctionTemplate::New(isOpen)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("shutdown"),
			FunctionTemplate::New(shutdown)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("insert"),
			FunctionTemplate::New(insert)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("findByKey"),
			FunctionTemplate::New(findByKey)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("find"),
			FunctionTemplate::New(find)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("update"),
			FunctionTemplate::New(update)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("remove"),
			FunctionTemplate::New(remove)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("dropNamespace"),
			FunctionTemplate::New(dropNamespace)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("dbs"),
			FunctionTemplate::New(dbs)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("namespaces"),
			FunctionTemplate::New(namespaces)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("host"),
			FunctionTemplate::New(host)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("beginTransaction"),
			FunctionTemplate::New(beginTransaction)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("commitTransaction"),
			FunctionTemplate::New(commitTransaction)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rollbackTransaction"),
			FunctionTemplate::New(rollbackTransaction)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("executeUpdate"),
			FunctionTemplate::New(executeUpdate)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("executeQuery"),
			FunctionTemplate::New(executeQuery)->GetFunction());

	constructor = Persistent<Function>::New(tpl->GetFunction());
	//target->Set(String::NewSymbol("WrapConnection"), constructor);
}

Handle<Value> WrapConnection::New(const Arguments& args) {
	HandleScope scope;

	WrapConnection* obj = new WrapConnection();
	
	Local<External> external = Local<External>::Cast(args[0]);

	DjondbConnection* con = (DjondbConnection*)external->Value();
	obj->setConnection(con);
	obj->Wrap(args.This());

	return args.This();
}

Handle<Object> WrapConnection::NewInstance(DjondbConnection* con) {
	HandleScope scope;

	const unsigned argc = 1;
	Handle<External> external = External::New(con);

	Handle<Value> argv[argc] = { external };
	Local<Object> instance = constructor->NewInstance(argc, argv);

	return scope.Close(instance);
}

Handle<Value> WrapConnection::open(const v8::Arguments& args) {
	HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	bool result = obj->_con->open();

	return scope.Close(Boolean::New(result));
}

Handle<Value> WrapConnection::close(const v8::Arguments& args) {
	HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	obj->_con->close();

	return scope.Close(v8::Undefined());
}

Handle<Value> WrapConnection::isOpen(const v8::Arguments& args) {
	HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());

	bool result = obj->_con->isOpen();

	return scope.Close(Boolean::New(result));
}

Handle<Value> WrapConnection::shutdown(const v8::Arguments& args) {
	HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());

	bool result = obj->_con->shutdown();

	return scope.Close(Boolean::New(result));
}

Handle<Value> WrapConnection::insert(const v8::Arguments& args) {
	if (args.Length() < 3) {
		return v8::ThrowException(v8::String::New("usage: db.insert(db, namespace, json)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value str(args[0]);
	std::string db = ToCString(str);
	v8::String::Utf8Value str2(args[1]);
	std::string ns = ToCString(str2);
	std::string json;
	if (args[2]->IsObject()) {
		v8::String::Utf8Value strValue(toJson(args[2]));
		json = ToCString(strValue);
	} else {
		v8::String::Utf8Value sjson(args[2]);
		json = ToCString(sjson);
	}

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	obj->_con->insert(db.c_str(), ns.c_str(), json.c_str());

	return scope.Close(v8::Undefined());
}

v8::Handle<v8::Value> WrapConnection::find(const v8::Arguments& args) {
	if (args.Length() < 2) {
		return v8::ThrowException(v8::String::New("usage: db.find(db, namespace)\ndb.find(db, namespace, filter)\ndb.find(db, namespace, select, filter)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);
	v8::String::Utf8Value str(args[1]);
	std::string ns = ToCString(str);
	std::string select = "*";
	std::string filter = "";
	if (args.Length() == 3) {
		v8::String::Utf8Value strFilter(args[2]);
		filter = ToCString(strFilter);
	}
	if (args.Length() == 4) {
		v8::String::Utf8Value strSelect(args[2]);
		select = ToCString(strSelect);
		v8::String::Utf8Value strFilter(args[3]);
		filter = ToCString(strFilter);
	}

	try {
		WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());

		DjondbCursor* cursor = obj->_con->find(db.c_str(), ns.c_str(), select.c_str(), filter.c_str());

		Handle<Object> result = WrapCursor::NewInstance(cursor);

		printf("find ready \n");

		return scope.Close(result);
	} catch (ParseException e) {
		return v8::ThrowException(v8::String::New("the filter expression contains an error\n"));
	}
}

v8::Handle<v8::Value> WrapConnection::findByKey(const v8::Arguments& args) {
	if (args.Length() != 3) {
		return v8::ThrowException(v8::String::New("usage: db.findByKey(db, namespace, key)"));
	}

	v8::HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());

	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);
	v8::String::Utf8Value str(args[1]);
	std::string ns = ToCString(str);
	v8::String::Utf8Value strFilter(args[2]);
	std::string filter = ToCString(strFilter);

	BSONObj* result = obj->_con->findByKey(db.c_str(), ns.c_str(), filter.c_str());

	char* sresult; 
	if (result != NULL) {
		sresult = result->toChar();

		delete result;
		return scope.Close(parseJSON(v8::String::New(sresult)));
	} else {
		return v8::Undefined();
	}

}

v8::Handle<v8::Value> WrapConnection::update(const v8::Arguments& args) {
	if (args.Length() < 3) {
		return v8::ThrowException(v8::String::New("usage: db.update(db, namespace, json)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value str(args[0]);
	std::string db = ToCString(str);
	v8::String::Utf8Value str2(args[1]);
	std::string ns = ToCString(str2);
	std::string json;
	if (args[2]->IsObject()) {
		v8::String::Utf8Value strValue(toJson(args[2]));
		json = ToCString(strValue);
	} else {
		v8::String::Utf8Value sjson(args[2]);
		json = ToCString(sjson);
	}

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	obj->_con->update(db.c_str(), ns.c_str(), json.c_str());

	return v8::Undefined();
}

v8::Handle<v8::Value> WrapConnection::remove(const v8::Arguments& args) {
	if (args.Length() < 4) {
		return v8::ThrowException(v8::String::New("usage: db.remove(db, namespace, id, revision)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value str(args[0]);
	std::string db = ToCString(str);
	v8::String::Utf8Value str2(args[1]);
	std::string ns = ToCString(str2);
	v8::String::Utf8Value str3(args[2]);
	std::string id = ToCString(str3);
	v8::String::Utf8Value str4(args[3]);
	std::string revision = ToCString(str4);

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	obj->_con->remove(db.c_str(), ns.c_str(), id.c_str(), revision.c_str());

	return v8::Undefined();
}

v8::Handle<v8::Value> WrapConnection::dropNamespace(const v8::Arguments& args) {
	if (args.Length() < 2) {
		return v8::ThrowException(v8::String::New("usage: dropNamespace(db, namespace)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);
	v8::String::Utf8Value str(args[1]);
	std::string ns = ToCString(str);

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	bool result = obj->_con->dropNamespace(db.c_str(), ns.c_str());

	return scope.Close(Boolean::New(result));
}

v8::Handle<v8::Value> WrapConnection::dbs(const v8::Arguments& args) {
	if (args.Length() != 0) {
		return v8::ThrowException(v8::String::New("usage: db.dbs()"));
	}

	v8::HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	std::vector<std::string>* dbs = obj->_con->dbs();

	v8::Handle<v8::Array> result = v8::Array::New();
	int index = 0;
	for (std::vector<std::string>::iterator i = dbs->begin(); i != dbs->end(); i++) {
		std::string n = *i;
		result->Set(v8::Number::New(index), v8::String::New(n.c_str()));
		index++;
	}
	delete dbs;
	return scope.Close(result);
}

v8::Handle<v8::Value> WrapConnection::namespaces(const v8::Arguments& args) {
	if (args.Length() < 1) {
		return v8::ThrowException(v8::String::New("usage: db.namespaces(db)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	std::vector<std::string>* ns = obj->_con->namespaces(db.c_str());

	v8::Handle<v8::Array> result = v8::Array::New();
	int index = 0;
	for (std::vector<std::string>::iterator i = ns->begin(); i != ns->end(); i++) {
		std::string n = *i;
		result->Set(v8::Number::New(index), v8::String::New(n.c_str()));
		index++;
	}
	delete ns;
	return scope.Close(result);
}

v8::Handle<v8::Value> WrapConnection::beginTransaction(const v8::Arguments& args) {
	if (args.Length() > 0) {
		return v8::ThrowException(v8::String::New("usage: beginTransaction()"));
	}

	v8::HandleScope handle_scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	if ((obj == NULL) || (obj->_con == NULL)) {
		return v8::ThrowException(v8::String::New("You're not connected to any db, please use: connect(server, [port])"));
	}
	obj->_con->beginTransaction();

	return v8::Undefined();
}

v8::Handle<v8::Value> WrapConnection::commitTransaction(const v8::Arguments& args) {
	if (args.Length() > 0) {
		return v8::ThrowException(v8::String::New("usage: commitTransaction()"));
	}

	v8::HandleScope handle_scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	if ((obj == NULL) || (obj->_con == NULL)) {
		return v8::ThrowException(v8::String::New("You're not connected to any db, please use: connect(server, [port])"));
	}
	obj->_con->commitTransaction();

	return v8::Undefined();
}

v8::Handle<v8::Value> WrapConnection::rollbackTransaction(const v8::Arguments& args) {
	if (args.Length() > 0) {
		return v8::ThrowException(v8::String::New("usage: rollbackTransaction()"));
	}

	v8::HandleScope handle_scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	if ((obj == NULL) || (obj->_con == NULL)) {
		return v8::ThrowException(v8::String::New("You're not connected to any db, please use: connect(server, [port])"));
	}
	obj->_con->rollbackTransaction();

	return v8::Undefined();
}

Handle<Value> WrapConnection::host(const v8::Arguments& args) {
	HandleScope scope;

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());

	std::string result = obj->_con->host();

	return scope.Close(v8::String::New(result.c_str()));
}

void WrapConnection::setConnection(djondb::DjondbConnection* con) {
	this->_con = con;
}

v8::Handle<v8::Value> WrapConnection::executeUpdate(const v8::Arguments& args) {
	if (args.Length() != 1) {
		return v8::ThrowException(v8::String::New("usage: executeUpdate(query)"));
	}

	v8::HandleScope handle_scope;
	v8::String::Utf8Value str(args[0]);
	std::string query = ToCString(str);

	try {
		WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
		if ((obj == NULL) || (obj->_con == NULL)) {
			return v8::ThrowException(v8::String::New("You're not connected to any db, please use: connect(server, [port])"));
		}
		obj->_con->executeUpdate(query.c_str());

		return v8::Undefined();
	} catch (ParseException e) {
		return v8::ThrowException(v8::String::New("there is an error in the expression.\n"));
	} catch (DjondbException e) {
		return v8::ThrowException(v8::String::New("An error ocurred executing the update expression.\n"));
	}
}

v8::Handle<v8::Value> WrapConnection::executeQuery(const v8::Arguments& args) {
	if (args.Length() != 1) {
		return v8::ThrowException(v8::String::New("usage: executeQuery(dql)"));
	}

	v8::HandleScope scope;
	v8::String::Utf8Value strQuery(args[0]);
	std::string query = ToCString(strQuery);

	WrapConnection* obj = ObjectWrap::Unwrap<WrapConnection>(args.This());
	if ((obj == NULL) || (obj->_con == NULL)) {
		return v8::ThrowException(v8::String::New("You're not connected to any db, please use: connect(server, [port])"));
	}
	try {
		DjondbCursor* cursor = obj->_con->executeQuery(query.c_str());

		if (cursor != NULL) {
			Handle<Object> result = WrapCursor::NewInstance(cursor);

			return scope.Close(result);
		} else {
			return v8::Undefined();
		}
	} catch (ParseException e) {
		return v8::ThrowException(v8::String::New("there is an error in the expression.\n"));
	} catch (DjondbException e) {
		return v8::ThrowException(v8::String::New("An error ocurred executing the update expression.\n"));
	}
}

