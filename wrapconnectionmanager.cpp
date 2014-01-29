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

WrapConnectionManager::WrapConnectionManager() {}
WrapConnectionManager::~WrapConnectionManager() {}

void WrapConnectionManager::Init(Handle<Object> target) {
	//Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New();
	tpl->SetClassName(String::NewSymbol("WrapConnectionManager"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//Prototype
	//global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getConnection"),
			FunctionTemplate::New(getConnection)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("releaseConnection"),
			FunctionTemplate::New(releaseConnection)->GetFunction());

	Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
	target->Set(String::NewSymbol("WrapConnectionManager"), constructor);
}

Handle<Value> WrapConnectionManager::getConnection(const v8::Arguments& args) {
	if (args.Length() < 1) {
		return v8::ThrowException(v8::String::New("usage: djondb.getConnection(host, [port])"));
	}

	v8::HandleScope scope;
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

	Handle<Object> result = WrapConnection::NewInstance(con);

	return scope.Close(result);
}

Handle<Value> WrapConnectionManager::releaseConnection(const v8::Arguments& args) {
	v8::HandleScope scope;

	WrapConnection* con = ObjectWrap::Unwrap<WrapConnection>(args[0]->ToObject());

	DjondbConnectionManager::releaseConnection(con->_con);

	return scope.Close(v8::Undefined());
}
