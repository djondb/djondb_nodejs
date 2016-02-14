/*
 * =====================================================================================
 *
 *       Filename:  wrapconnectionmanager.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/12/2012 07:13:57 PM
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
#include "wrapconnectionmanager.h"
#include "wrapconnection.h"
#include "djondb_client.h"
#include "v8.h"
#include "nodeutil.h"
#include <string>
#include <sstream>

using namespace v8;
using namespace djondb;

v8::Persistent<v8::Function> WrapConnectionManager::constructor;

WrapConnectionManager::WrapConnectionManager() {}
WrapConnectionManager::~WrapConnectionManager() {}

void WrapConnectionManager::Init(Handle<Object> target) {
	v8::Isolate* isolate = target->GetIsolate();

	/*
	//Prepare constructor template
	Local<FunctionTemplate> tpl = v8::FunctionTemplate::New(isolate);
	tpl->SetClassName(v8::String::NewFromUtf8(isolate, "WrapConnectionManager"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//Prototype
	//global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	NODE_SET_PROTOTYPE_METHOD(tpl, "getConnection", getConnection);
	NODE_SET_PROTOTYPE_METHOD(tpl, "releaseConnection", releaseConnection);

	constructor.Reset(isolate, tpl->GetFunction());
	target->Set(v8::String::NewFromUtf8(isolate, "WrapConnectionManager"), tpl->GetFunction());
	*/

	NODE_SET_METHOD(target, "getConnection", getConnection);
	NODE_SET_METHOD(target, "releaseConnection", releaseConnection);
}

void WrapConnectionManager::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		WrapConnectionManager* obj = new WrapConnectionManager();
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	} else {
		// Invoked as plain function `WrapConnectionManager(...)`, turn into construct call.
		printf("b\n");
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}

v8::Local<v8::Object> WrapConnectionManager::NewInstance(v8::Isolate* isolate) {
	const unsigned argc = 0;
	v8::Handle<Value> argv[argc] = { };
	Local<Function> cons = v8::Local<v8::Function>::New(isolate, constructor);
	v8::Local<v8::Object> instance = cons->NewInstance(argc, argv);

	return instance;

}

void WrapConnectionManager::getConnection(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	if (args.Length() < 1) {
		isolate->ThrowException(v8::String::NewFromUtf8(isolate, "usage: getConnection(host, [port])"));
		return;
	}

	v8::String::Utf8Value str(args[0]);
	std::string host = ToCString(str);
	djondb::DjondbConnection* con;
	if (args.Length() > 1) {
		v8::String::Utf8Value str2(args[1]);
		int port = args[1]->NumberValue();
		con = DjondbConnectionManager::getConnection(host.c_str(), port);
	} else {
		con = DjondbConnectionManager::getConnection(host.c_str());
	}

	Local<External> hcon = External::New(Isolate::GetCurrent(), con);
	const unsigned argc = 1;
	Local<Value> argv[argc] = { hcon };

	Local<Function> cons = Local<Function>::New(isolate, WrapConnection::connection_constructor);
	Local<Object> instance = cons->NewInstance(argc, argv);

	args.GetReturnValue().Set(instance);
}

void WrapConnectionManager::releaseConnection(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	WrapConnection* con = ObjectWrap::Unwrap<WrapConnection>(args[0]->ToObject());

	DjondbConnectionManager::releaseConnection(con->_con);

	args.GetReturnValue().Set(v8::Null(args.GetIsolate()));
}
