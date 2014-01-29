#ifndef WRAP_CONNECTION_H
#define WRAP_CONNECTION_H

#include "djondefs.h"
#include <string>
#include <vector>
#include "djondb_client.h"

class WrapConnection: public node::ObjectWrap
{
	public:
		static void Init(v8::Handle<v8::Object> target);
		static v8::Handle<v8::Object> NewInstance(djondb::DjondbConnection* con);

		WrapConnection();
		~WrapConnection();

		void setConnection(djondb::DjondbConnection* con);

		djondb::DjondbConnection* _con;
	protected:
	private:

		static v8::Handle<v8::Value> open(const v8::Arguments& args);
		static v8::Handle<v8::Value> close(const v8::Arguments& args);
		static v8::Handle<v8::Value> isOpen(const v8::Arguments& args);

		static v8::Handle<v8::Value> shutdown(const v8::Arguments& args);

		static v8::Handle<v8::Value> insert(const v8::Arguments& args);
		static v8::Handle<v8::Value> findByKey(const v8::Arguments& args);
		static v8::Handle<v8::Value> find(const v8::Arguments& args); 
		static v8::Handle<v8::Value> update(const v8::Arguments& args);
		static v8::Handle<v8::Value> remove(const v8::Arguments& args);

		static v8::Handle<v8::Value> dropNamespace(const v8::Arguments& args);
		static v8::Handle<v8::Value> dbs(const v8::Arguments& args);
		static v8::Handle<v8::Value> namespaces(const v8::Arguments& args);

		static v8::Handle<v8::Value> beginTransaction(const v8::Arguments& args);
		static v8::Handle<v8::Value> commitTransaction(const v8::Arguments& args);
		static v8::Handle<v8::Value> rollbackTransaction(const v8::Arguments& args);

		static v8::Handle<v8::Value> host(const v8::Arguments& args);

		static v8::Handle<v8::Value> executeUpdate(const v8::Arguments& args);
		static v8::Handle<v8::Value> executeQuery(const v8::Arguments& args);

		static v8::Persistent<v8::Function> constructor;
		static v8::Handle<v8::Value> New(const v8::Arguments& args);
};

#endif // WRAP_CONNECTION_H
