#ifndef WRAP_CONNECTIONMANAGER_H
#define WRAP_CONNECTIONMANAGER_H

#include "djondefs.h"

class WrapConnectionManager: public node::ObjectWrap
{
	public:
		static void Init(v8::Local<v8::Object> target);
		/** Default constructor */
		static v8::Local<v8::Object> NewInstance(v8::Isolate* isolate);

		static void getConnection(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void releaseConnection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void test(const v8::FunctionCallbackInfo<v8::Value>& args);

	protected:
	private:
		WrapConnectionManager();
		virtual ~WrapConnectionManager();

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static v8::Persistent<v8::Function> constructor;
};

#endif // WRAP_CONNECTIONMANAGER_H
