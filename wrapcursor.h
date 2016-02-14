#ifndef WRAP_CURSOR_H
#define WRAP_CURSOR_H

#include "djondefs.h"
#include <string>
#include <vector>
#include "djondb_client.h"

/**
 * @brief This class is a wrapper for the DjondbCursor
 */
class WrapCursor: public node::ObjectWrap
{
	public:
		static void Init(v8::Handle<v8::Object> target);
		static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

		WrapCursor(djondb::DjondbCursor* cursor);
		virtual ~WrapCursor();

		djondb::DjondbCursor* _cursor;
		static v8::Persistent<v8::Function> constructorCursor;
	protected:
	private:

		/**
		 * @brief checks if more elements could be retrieved
		 *
		 * @return true if more BSONObj are ready, false otherwise
		 */
		static void next(const v8::FunctionCallbackInfo<v8::Value>& args);
		/**
		 * @brief checks if there are elements in the front of the cursor
		 *
		 * @return true of not BOF, false otherwise
		 */
		static void previous(const v8::FunctionCallbackInfo<v8::Value>& args);
		/**
		 * @brief Returns the current loaded element, the client should call next() method before calling this, if not
		 * an unexpected behavior could occur
		 *
		 * @return the current element or NULL if the next method was not called
		 */
		static void current(const v8::FunctionCallbackInfo<v8::Value>& args);

		/**
		 * @brief This will retrieve the length of the rows contained in the cursor, if the cursor is still loading then
		 * all the rows will be retrieved from the server. This method should be used with care, because it will try to
		 * retrieve every row from the server and it may contain several pages
		 *
		 * @return length of the cursor
		 */
		static void length(const v8::FunctionCallbackInfo<v8::Value>& args);

		/**
		 * @brief Releases the cursor from the server, the client should use this method if the cursor is no longer required
		 */
		static void releaseCursor(const v8::FunctionCallbackInfo<v8::Value>& args);

		/**
		 * @brief Will place the current row in the desired position
		 *
		 * @param position
		 */
		static void seek(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif // WRAP_CURSOR_H
