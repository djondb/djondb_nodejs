/*
 * =====================================================================================
 *
 *       Filename:  nodeutil.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/12/2012 08:41:23 PM
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
#include "nodeutil.h"

std::string ToCString(const v8::String::Utf8Value& value) {
	return *value ? std::string(*value) : "<string conversion failed>";
}

v8::Handle<v8::Value> parseJSON(v8::Isolate* isolate, v8::Handle<v8::Value> object)
{
	v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Handle<v8::Object> global = context->Global();

	v8::Handle<v8::Object> JSON = global->Get(v8::String::NewFromUtf8(isolate, "JSON"))->ToObject();
	v8::Handle<v8::Function> JSON_parse = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::NewFromUtf8(isolate, "parse")));

	return scope.Escape(JSON_parse->Call(JSON, 1, &object));
}

v8::Handle<v8::Value> toJson(v8::Isolate* isolate, v8::Handle<v8::Value> object, bool beautify)
{
	v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Handle<v8::Object> global = context->Global();

	v8::Handle<v8::Object> JSON = global->Get(v8::String::NewFromUtf8(isolate, "JSON"))->ToObject();
	v8::Handle<v8::Function> JSON_stringify = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::NewFromUtf8(isolate, "stringify")));

	if (!beautify) {
		return scope.Escape(JSON_stringify->Call(JSON, 1, &object));
	} else {
		v8::Handle<v8::Value> args[3];
		args[0] = object;
		args[1] = v8::Null(isolate);
		args[2] = v8::Integer::New(isolate, 4);
		return scope.Escape(JSON_stringify->Call(JSON, 3, args));
	}
}

