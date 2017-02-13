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

WrapCursor::WrapCursor(djondb::DjondbCursor* cursor): _cursor(cursor) {}
WrapCursor::~WrapCursor() {}

void WrapCursor::Init(Handle<Object> target) {
	v8::Isolate* isolate = target->GetIsolate();

	//Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(isolate, "WrapCursor"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	//Prototype
	//global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	NODE_SET_PROTOTYPE_METHOD(tpl, "next", next);
	NODE_SET_PROTOTYPE_METHOD(tpl, "previous", previous);
	NODE_SET_PROTOTYPE_METHOD(tpl, "current", current);
	NODE_SET_PROTOTYPE_METHOD(tpl, "length", length);
	NODE_SET_PROTOTYPE_METHOD(tpl, "releaseCursor", releaseCursor);
	NODE_SET_PROTOTYPE_METHOD(tpl, "seek", seek);

	constructorCursor.Reset(isolate, tpl->GetFunction());
}

void WrapCursor::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		/* 
		 * Not sure if this is actually called
		 * */
  		Local<External> hcursor = Local<External>::Cast(args[0]->ToObject());
		djondb::DjondbCursor* cursor = (djondb::DjondbCursor*)hcursor->Value();

		WrapCursor* wcur = new WrapCursor(cursor);
		wcur->Wrap(args.Holder());
		args.GetReturnValue().Set(args.Holder());
	} else {
		// Invoked as plain function `MyObject(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructorCursor);
		args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}

void WrapCursor::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	const unsigned argc = 1;
	Local<Value> argv[argc] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, constructorCursor);
	Local<Object> instance = cons->NewInstance(argc, argv);

	args.GetReturnValue().Set(instance);
}

void WrapCursor::next(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());
	bool result = obj->_cursor->next();

	args.GetReturnValue().Set(Boolean::New(isolate, result));
}

void WrapCursor::previous(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());
	bool result = obj->_cursor->previous();

	args.GetReturnValue().Set(Boolean::New(isolate, result));
}

void WrapCursor::current(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

	BSONObj* current = obj->_cursor->current();

	if (current == 0) {
		THROW_NODE_EXCEPTION(isolate, "current is null, you should call next() before calling current()");
		return;
	}
	char* str = current->toChar();

	v8::Handle<v8::Value> jsonValue = parseJSON(isolate, v8::String::NewFromUtf8(isolate, str));

	args.GetReturnValue().Set(jsonValue);
}

void WrapCursor::length(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

	__int32 current = obj->_cursor->length();

	args.GetReturnValue().Set(v8::Integer::New(isolate, current));
}

void WrapCursor::releaseCursor(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

	obj->_cursor->releaseCursor();

	args.GetReturnValue().Set(v8::Undefined(isolate));
}

void WrapCursor::seek(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	if (args.Length() < 1) {
		THROW_NODE_EXCEPTION(isolate, "usage: cursor.seek(position)");
		return;
	}

	if (!args[0]->IsInt32()) {
		THROW_NODE_EXCEPTION(isolate, "usage: cursor.seek(position)");
		return;
	}
	int pos = args[0]->NumberValue();

	try {
		WrapCursor* obj = ObjectWrap::Unwrap<WrapCursor>(args.This());

		obj->_cursor->seek(pos);

		args.GetReturnValue().Set(v8::Undefined(isolate));
	} catch (ParseException e) {
		THROW_NODE_EXCEPTION(isolate, "the filter expression contains an error\n");
		return;
	}
}

