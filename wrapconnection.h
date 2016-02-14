#ifndef WRAP_CONNECTION_H
#define WRAP_CONNECTION_H

#include "djondefs.h"
#include <string>
#include <vector>
#include "djondb_client.h"

class WrapConnection: public node::ObjectWrap
{
	public:
		static void Init(v8::Local<v8::Object> exports);
		static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

		WrapConnection(djondb::DjondbConnection* con);
		virtual ~WrapConnection();

		void setConnection(djondb::DjondbConnection* con);


		djondb::DjondbConnection* _con;
		static v8::Persistent<v8::Function> connection_constructor;
	protected:
	private:
		static void open(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void close(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void isOpen(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void shutdown(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void insert(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void findByKey(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void find(const v8::FunctionCallbackInfo<v8::Value>& args); 
		static void update(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void remove(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void dropNamespace(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void dbs(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void namespaces(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void beginTransaction(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void commitTransaction(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void rollbackTransaction(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void host(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void executeUpdate(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void executeQuery(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif // WRAP_CONNECTION_H
