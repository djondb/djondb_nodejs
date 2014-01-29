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
#include "wrapcursor.h"
#include "djondb_client.h"
#include "v8.h"
#include "nodeutil.h"
#include <string>
#include <sstream>

using namespace v8;
using namespace djondb;

Persistent<Function> WrapCursor::constructorCursor;

WrapCursor::WrapCursor() {}
WrapCursor::~WrapCursor() {}

void WrapCursor::Init(Handle<Object> target) {
	//Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("WrapCursor"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//Prototype
	//global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	tpl->PrototypeTemplate()->Set(String::NewSymbol("next"),
			FunctionTemplate::New(next)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("previous"),
			FunctionTemplate::New(previous)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("current"),
			FunctionTemplate::New(current)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("length"),
			FunctionTemplate::New(length)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("releaseCursor"),
			FunctionTemplate::New(releaseCursor)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("seek"),
			FunctionTemplate::New(seek)->GetFunction());

	constructorCursor = Persistent<Function>::New(tpl->GetFunction());
	//target->Set(String::NewSymbol("WrapCursor"), constructor);
}

Handle<Value> WrapCursor::New(const Arguments& args) {
	HandleScope scope;

	WrapCursor* obj = new WrapCursor();

	Local<External> external = Local<External>::Cast(args[0]);

	DjondbCursor* cursor = (DjondbCursor*)external->Value();
	obj->setCursor(cursor);
	obj->Wrap(args.This());

	return args.This();
}

Handle<Object> WrapCursor::NewInstance(DjondbCursor* cursor) {
	HandleScope scope;

	const unsigned argc = 1;
	Handle<External> external = External::New(cursor);

	Handle<Value> argv[argc] = { external };

	Local<Object> instance = constructorCursor->NewInstance(argc, argv);
	return scope.Close(instance);
}

Handle<Value> WrapCursor::next(const v8::Arguments& args) {
	HandleScope scope;

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());
	bool result = obj->_cursor->next();

	return scope.Close(Boolean::New(result));
}

Handle<Value> WrapCursor::previous(const v8::Arguments& args) {
	HandleScope scope;

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());
	bool result = obj->_cursor->previous();

	return scope.Close(Boolean::New(result));
}

Handle<Value> WrapCursor::current(const v8::Arguments& args) {
	HandleScope scope;

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

	BSONObj* current = obj->_cursor->current();

	if (current == 0) {
		return v8::ThrowException(v8::String::New("current is null, you should call next() before calling current()"));
	}
	printf("current %d \n", current);

	char* str = current->toChar();

	v8::Handle<v8::Value> jsonValue = parseJSON(v8::String::New(str));

	return scope.Close(jsonValue);
}

Handle<Value> WrapCursor::length(const v8::Arguments& args) {
	HandleScope scope;

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

	__int32 current = obj->_cursor->length();

	return scope.Close(v8::Integer::New(current));
}

Handle<Value> WrapCursor::releaseCursor(const v8::Arguments& args) {
	HandleScope scope;

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

	obj->_cursor->releaseCursor();

	return scope.Close(v8::Undefined());
}

v8::Handle<v8::Value> WrapCursor::seek(const v8::Arguments& args) {
	if (args.Length() < 1) {
		return v8::ThrowException(v8::String::New("usage: cursor.seek(position)"));
	}

	v8::HandleScope scope;
	if (!args[0]->IsInt32()) {
		return v8::ThrowException(v8::String::New("usage: cursor.seek(position)"));
	}
	int pos = args[0]->NumberValue();

	try {
		WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

		obj->_cursor->seek(pos);

		return scope.Close(v8::Undefined());
	} catch (ParseException e) {
		return v8::ThrowException(v8::String::New("the filter expression contains an error\n"));
	}
}

void WrapCursor::setCursor(djondb::DjondbCursor* cursor) {
	this->_cursor = cursor;
}
