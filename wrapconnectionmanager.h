#ifndef WRAP_CONNECTIONMANAGER_H
#define WRAP_CONNECTIONMANAGER_H

#include "djondefs.h"

class WrapConnectionManager: public node::ObjectWrap
{
	public:
		static void Init(v8::Handle<v8::Object> target);
		/** Default constructor */

		static v8::Handle<v8::Value> getConnection(const v8::Arguments& args);

		static v8::Handle<v8::Value> releaseConnection(const v8::Arguments& args);
	protected:
	private:
		WrapConnectionManager();
		virtual ~WrapConnectionManager();
};

#endif // WRAP_CONNECTIONMANAGER_H
