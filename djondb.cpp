/*
 * =====================================================================================
 *
 *       Filename:  djondb.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/08/2012 11:18:12 PM
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

#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "wrapcursor.h"
#include "wrapconnection.h"
#include "wrapconnectionmanager.h"

using namespace v8;

void InitAll(Local<Object> target) {
	  WrapCursor::Init(target);
	  WrapConnection::Init(target);
	  WrapConnectionManager::Init(target);
}

NODE_MODULE(djondb, InitAll)
