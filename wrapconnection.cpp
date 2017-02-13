/*
 * =====================================================================================
 *
 *       Filename:  wrapconnection.cpp
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

Persistent<Function> WrapConnection::connection_constructor;

#define RETURN_CURSOR(cursor) \
		Local<External> hcur = External::New(Isolate::GetCurrent(), cursor); \
		const unsigned argc = 1; \
		Local<Value> argv[argc] = { hcur }; \
 \
		Local<Function> cons = Local<Function>::New(isolate, WrapCursor::constructorCursor); \
		Local<Object> instance = cons->NewInstance(argc, argv); \
\
		args.GetReturnValue().Set(instance);

WrapConnection::WrapConnection(DjondbConnection* con): _con(con) {
}


WrapConnection::~WrapConnection() {
	if (_con != NULL) {
		djondb::DjondbConnectionManager::releaseConnection(_con);
		_con = NULL;
	}
}

void WrapConnection::Init(v8::Local<v8::Object> exports) {
	Isolate* isolate = exports->GetIsolate();
	//Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "WrapConnection"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//Prototype
	//global->Set(v8::String::NewFromUtf8(isolate, "print"), v8::FunctionTemplate::New(Print));
	NODE_SET_PROTOTYPE_METHOD(tpl, "open", open);
	NODE_SET_PROTOTYPE_METHOD(tpl, "close", close);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isOpen", isOpen);
	NODE_SET_PROTOTYPE_METHOD(tpl, "shutdown", shutdown);
	NODE_SET_PROTOTYPE_METHOD(tpl, "insert", insert);
	NODE_SET_PROTOTYPE_METHOD(tpl, "findByKey", findByKey);
	NODE_SET_PROTOTYPE_METHOD(tpl, "find", find);
	NODE_SET_PROTOTYPE_METHOD(tpl, "update", update);
	NODE_SET_PROTOTYPE_METHOD(tpl, "remove", remove);
	NODE_SET_PROTOTYPE_METHOD(tpl, "dropNamespace", dropNamespace);
	NODE_SET_PROTOTYPE_METHOD(tpl, "dbs", dbs);
	NODE_SET_PROTOTYPE_METHOD(tpl, "namespaces", namespaces);
	NODE_SET_PROTOTYPE_METHOD(tpl, "host", host);
	NODE_SET_PROTOTYPE_METHOD(tpl, "beginTransaction", beginTransaction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "commitTransaction", commitTransaction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rollbackTransaction", rollbackTransaction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "executeUpdate", executeUpdate);
	NODE_SET_PROTOTYPE_METHOD(tpl, "executeQuery", executeQuery);

	connection_constructor.Reset(isolate, tpl->GetFunction());
}

void WrapConnection::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	const unsigned argc = 1;
	Local<Value> argv[argc] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, connection_constructor);
	Local<Object> instance = cons->NewInstance(argc, argv);

	args.GetReturnValue().Set(instance);

}

void WrapConnection::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		Local<External> hcon = Local<External>::Cast(args[0]->ToObject());
		djondb::DjondbConnection* con = (djondb::DjondbConnection*)hcon->Value();

		WrapConnection* wcon = new WrapConnection(con);
		wcon->Wrap(args.Holder());
		args.GetReturnValue().Set(args.Holder());
	} else {
		// Invoked as plain function `MyObject(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, connection_constructor);
		args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}

#define UNWRAPCONNECTION() \
	ObjectWrap::Unwrap<WrapConnection>(args.Holder());

#define RETURN_UNDEFINED() \
	args.GetReturnValue().Set(v8::Undefined(isolate));

void WrapConnection::open(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	WrapConnection* obj = UNWRAPCONNECTION();
	bool result = obj->_con->open();

	args.GetReturnValue().Set(v8::Boolean::New(isolate, result));
}

void WrapConnection::close(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	WrapConnection* obj = UNWRAPCONNECTION();
	obj->_con->close();

	args.GetReturnValue().Set(v8::Undefined(isolate));
}

void WrapConnection::isOpen(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	WrapConnection* obj = UNWRAPCONNECTION();

	bool result = obj->_con->isOpen();

	args.GetReturnValue().Set(v8::Boolean::New(isolate, result));
}

void WrapConnection::shutdown(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	try {
		WrapConnection* obj = UNWRAPCONNECTION();

		bool result = obj->_con->shutdown();

		args.GetReturnValue().Set(v8::Boolean::New(isolate, result));
	} catch (DjondbException exc) {
		THROW_NODE_EXCEPTION(isolate, exc.what());
		return;
	}
}

void WrapConnection::insert(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() < 3) {
		THROW_NODE_EXCEPTION(isolate, "insert expects 3 arguments. usage: insert(db, namespace, json)");
		return;
	}

	v8::String::Utf8Value str(args[0]);
	std::string db = ToCString(str);
	v8::String::Utf8Value str2(args[1]);
	std::string ns = ToCString(str2);
	std::string json;
	if (args[2]->IsObject()) {
		v8::String::Utf8Value strValue(toJson(isolate, args[2], false));
		json = ToCString(strValue);
	} else {
		v8::String::Utf8Value sjson(args[2]);
		json = ToCString(sjson);
	}

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		obj->_con->insert(db.c_str(), ns.c_str(), json.c_str());

		args.GetReturnValue().Set(v8::Undefined(isolate));
	} catch (DjondbException exc) {
		THROW_NODE_EXCEPTION(isolate, exc.what());
		return;
	}
}

void WrapConnection::find(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() < 2) {
		THROW_NODE_EXCEPTION(isolate, "Wrong number of arguments. Use find(db, namespace)\ndb.find(db, namespace, filter) or db.find(db, namespace, select, filter)");
		return;
	}

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
		WrapConnection* obj = UNWRAPCONNECTION();

		DjondbCursor* cursor = obj->_con->find(db.c_str(), ns.c_str(), select.c_str(), filter.c_str());

		RETURN_CURSOR(cursor);
	} catch (ParseException e) {
		THROW_NODE_EXCEPTION(isolate, e.what());
		return;
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::findByKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() != 3) {
		THROW_NODE_EXCEPTION(isolate, "usage: db.findByKey(db, namespace, key)");
		return;
	}

	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);
	v8::String::Utf8Value str(args[1]);
	std::string ns = ToCString(str);
	v8::String::Utf8Value strFilter(args[2]);
	std::string filter = ToCString(strFilter);

	try {
		WrapConnection* obj = UNWRAPCONNECTION();

		BSONObj* result = obj->_con->findByKey(db.c_str(), ns.c_str(), filter.c_str());

		char* sresult; 
		if (result != NULL) {
			sresult = result->toChar();

			delete result;
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, sresult));
		} else {
			RETURN_UNDEFINED();
		}

	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::update(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() < 3) {
		THROW_NODE_EXCEPTION(isolate, "usage: db.update(db, namespace, json)");
		return;
	}

	v8::String::Utf8Value str(args[0]);
	std::string db = ToCString(str);
	v8::String::Utf8Value str2(args[1]);
	std::string ns = ToCString(str2);
	std::string json;
	if (args[2]->IsObject()) {
		v8::String::Utf8Value strValue(toJson(isolate, args[2], false));
		json = ToCString(strValue);
	} else {
		v8::String::Utf8Value sjson(args[2]);
		json = ToCString(sjson);
	}

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		obj->_con->update(db.c_str(), ns.c_str(), json.c_str());

		RETURN_UNDEFINED();
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::remove(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() < 4) {
		THROW_NODE_EXCEPTION(isolate, "usage: db.remove(db, namespace, id, revision)");
		return;
	}

	v8::String::Utf8Value str(args[0]);
	std::string db = ToCString(str);
	v8::String::Utf8Value str2(args[1]);
	std::string ns = ToCString(str2);
	v8::String::Utf8Value str3(args[2]);
	std::string id = ToCString(str3);
	v8::String::Utf8Value str4(args[3]);
	std::string revision = ToCString(str4);

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		obj->_con->remove(db.c_str(), ns.c_str(), id.c_str(), revision.c_str());

		RETURN_UNDEFINED();
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::dropNamespace(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() < 2) {
		THROW_NODE_EXCEPTION(isolate, "usage: dropNamespace(db, namespace)");
		return;
	}

	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);
	v8::String::Utf8Value str(args[1]);
	std::string ns = ToCString(str);

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		bool result = obj->_con->dropNamespace(db.c_str(), ns.c_str());

		args.GetReturnValue().Set(v8::Boolean::New(isolate, result));
	} catch (DjondbException& e) {
		THROW_NODE_EXCEPTION(isolate, e.what());
		return;
	}
}

void WrapConnection::dbs(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() != 0) {
		THROW_NODE_EXCEPTION(isolate, "usage: db.dbs()");
		return;
	}

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		std::vector<std::string>* dbs = obj->_con->dbs();

		v8::Handle<v8::Array> result = v8::Array::New(isolate);
		int index = 0;
		for (std::vector<std::string>::iterator i = dbs->begin(); i != dbs->end(); i++) {
			std::string n = *i;
			result->Set(v8::Number::New(isolate, index), v8::String::NewFromUtf8(isolate, n.c_str()));
			index++;
		}
		delete dbs;
		args.GetReturnValue().Set(result);
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::namespaces(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() < 1) {
		THROW_NODE_EXCEPTION(isolate, "usage: db.namespaces(db)");
		return;
	}

	v8::String::Utf8Value strDB(args[0]);
	std::string db = ToCString(strDB);

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		std::vector<std::string>* ns = obj->_con->namespaces(db.c_str());

		v8::Handle<v8::Array> result = v8::Array::New(isolate);
		int index = 0;
		for (std::vector<std::string>::iterator i = ns->begin(); i != ns->end(); i++) {
			std::string n = *i;
			result->Set(v8::Number::New(isolate, index), v8::String::NewFromUtf8(isolate, n.c_str()));
			index++;
		}
		delete ns;
		args.GetReturnValue().Set(result);
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::beginTransaction(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() > 0) {
		THROW_NODE_EXCEPTION(isolate, "usage: beginTransaction()");
		return;
	}

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		if ((obj == NULL) || (obj->_con == NULL)){
			THROW_NODE_EXCEPTION(isolate, "You're not connected to any db, please use: getConnection(server, [port])");
			return;
		}
		obj->_con->beginTransaction();

		RETURN_UNDEFINED();
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::commitTransaction(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() > 0) {
		THROW_NODE_EXCEPTION(isolate, "usage: commitTransaction()");
		return;
	}

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		if ((obj == NULL) || (obj->_con == NULL)) {
			THROW_NODE_EXCEPTION(isolate, "You're not connected to any db, please use: getConnection(server, [port])");
			return;
		}
		obj->_con->commitTransaction();

		RETURN_UNDEFINED();
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::rollbackTransaction(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.Length() > 0) {
		THROW_NODE_EXCEPTION(isolate, "usage: rollbackTransaction()");
		return;
	}

	try {
	WrapConnection* obj = UNWRAPCONNECTION();
	if ((obj == NULL) || (obj->_con == NULL)) {
		THROW_NODE_EXCEPTION(isolate, "You're not connected to any db, please use: getConnection(server, [port])");
		return;
	}
	obj->_con->rollbackTransaction();

	RETURN_UNDEFINED();
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::host(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	try {
		WrapConnection* obj = UNWRAPCONNECTION();

		std::string result = obj->_con->host();

		args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, result.c_str()));
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::setConnection(djondb::DjondbConnection* con) {
	this->_con = con;
}

void WrapConnection::executeUpdate(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	if (args.Length() != 1) {
		THROW_NODE_EXCEPTION(isolate, "usage: executeUpdate(query)");
		return;
	}

	v8::String::Utf8Value str(args[0]);
	std::string query = ToCString(str);

	try {
		WrapConnection* obj = UNWRAPCONNECTION();
		if ((obj == NULL) || (obj->_con == NULL)) {
			THROW_NODE_EXCEPTION(isolate, "You're not connected to any db, please use: getConnection(server, [port])");
			return;
		}
		obj->_con->executeUpdate(query.c_str());

		RETURN_UNDEFINED();
	} catch (ParseException e) {
		THROW_NODE_EXCEPTION(isolate, "there is an error in the expression.\n");
		return;
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

void WrapConnection::executeQuery(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	if (args.Length() != 1) {
		THROW_NODE_EXCEPTION(isolate, "usage: executeQuery(dql)");
		return;
	}

	v8::String::Utf8Value strQuery(args[0]);
	std::string query = ToCString(strQuery);

	WrapConnection* obj = UNWRAPCONNECTION();
	if ((obj == NULL) || (obj->_con == NULL)) {
		THROW_NODE_EXCEPTION(isolate, "You're not connected to any db, please use: getConnection(server, [port])");
		return;
	}
	try {
		DjondbCursor* cursor = obj->_con->executeQuery(query.c_str());

		if (cursor != NULL) {
			RETURN_CURSOR(cursor);
		} else {
			RETURN_UNDEFINED();
		}
	} catch (ParseException e) {
		THROW_NODE_EXCEPTION(isolate, e.what());
		return;
	} catch (DjondbException ex) {
		THROW_NODE_EXCEPTION(isolate, ex.what());
		return;
	}
}

