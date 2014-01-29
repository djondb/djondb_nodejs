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
		static v8::Handle<v8::Object> NewInstance(djondb::DjondbCursor* con);

		WrapCursor();
		~WrapCursor();

		void setCursor(djondb::DjondbCursor* cursor);

		djondb::DjondbCursor* _cursor;
	protected:
	private:

		/**
		 * @brief checks if more elements could be retrieved
		 *
		 * @return true if more BSONObj are ready, false otherwise
		 */
		static v8::Handle<v8::Value> next(const v8::Arguments& args);
		/**
		 * @brief checks if there are elements in the front of the cursor
		 *
		 * @return true of not BOF, false otherwise
		 */
		static v8::Handle<v8::Value> previous(const v8::Arguments& args);
		/**
		 * @brief Returns the current loaded element, the client should call next() method before calling this, if not
		 * an unexpected behavior could occur
		 *
		 * @return the current element or NULL if the next method was not called
		 */
		static v8::Handle<v8::Value> current(const v8::Arguments& args);

		/**
		 * @brief This will retrieve the length of the rows contained in the cursor, if the cursor is still loading then
		 * all the rows will be retrieved from the server. This method should be used with care, because it will try to
		 * retrieve every row from the server and it may contain several pages
		 *
		 * @return length of the cursor
		 */
		static v8::Handle<v8::Value> length(const v8::Arguments& args);

		/**
		 * @brief Releases the cursor from the server, the client should use this method if the cursor is no longer required
		 */
		static v8::Handle<v8::Value> releaseCursor(const v8::Arguments& args);

		/**
		 * @brief Will place the current row in the desired position
		 *
		 * @param position
		 */
		static v8::Handle<v8::Value> seek(const v8::Arguments& args);

		static v8::Persistent<v8::Function> constructorCursor;
		static v8::Handle<v8::Value> New(const v8::Arguments& args);
};

#endif // WRAP_CURSOR_H
