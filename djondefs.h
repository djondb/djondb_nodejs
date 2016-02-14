#ifndef DJONDB_DEFS_H
#define DJONDB_DEFS_H
#include <node.h>
#include <node_object_wrap.h>

#include <stdio.h>
#define THROW_NODE_EXCEPTION(isolate, message) \
	isolate->ThrowException(v8::String::NewFromUtf8(isolate, message)); \
	return;

#define PRINT_NODE(message) \
	FILE* p = fopen("log.txt", "a+"); \
	if (!p) { \
		exit(1); \
	} \
	fprintf(p, "%s\n", message);  \
	fclose(p);


#endif // DJONDB_DEFS_H

