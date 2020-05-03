
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <headers/stackTracer.h>
#include "cweb.h"

#define _ printf("Get here...: %d\n", __LINE__);




void test1();
//void test2(ArrayList_noSync_t list);
//void test3(ArrayList_noSync_t list);

int main(void) {
	MM_Init();
	StackTracer_Init();
 	
 	test1();
 	
	return 0;
}

void test1() {
	printf("=====================================================\n");
	printf("Func: %s\n", __func__);
	printf("=====================================================\n");
	
	char str[256] = "this is a fuck test+.";
	printf("in: \"%s\" len: %d\n", str, strlen(str));
	char *out = CWeb_Percent_Encode(str, strlen(str));
	if(out == NULL) abort();
	printf("encode: \"%s\"\n", out);
	
	if(strcmp(out, "this+is+a+fuck+test%2B.") != 0) {
		printf("ERROR - LINE: %d\n", __LINE__);
	} else {
		printf("TEST OK\n");
	}
	
	
	strcpy(str, "\"\'!@#$%^&*(){}[]\\|?/<>,.");
	printf("\n\nin: \"%s\" len: %d\n", str, strlen(str));
	out = CWeb_Percent_Encode(str, strlen(str));
	if(out == NULL) abort();
	printf("encode: \"%s\"\n", out);
	if( strcmp(out, "%22%27%21%40%23%24%25%5E%26*%28%29%7B%7D%5B%5D%5C%7C%3F%2F%3C%3E%2C.") != 0) {
		printf("ERROR - LINE: %d\n", __LINE__);
		printf("Exp: \"%s\"\n", "%22%27%21%40%23%24%25%5E%26*%28%29%7B%7D%5B%5D%5C%7C%3F%2F%3C%3E%2C.");
		printf("Get: \"%s\"\n", out);
	} else {
		printf("TEST OK\n");
	}
	
	
	strcpy(str, "strcpy(str, \"\"\'!@#$%^&*(){}[]\\|?/<>,.");
	printf("\n\nin: \"%s\" len: %d\n", str, strlen(str));
	out = CWeb_Percent_Encode(str, strlen(str));
	if(out == NULL) abort();
	printf("encode: \"%s\"\n", out);
	if( strcmp(out, "strcpy%28str%2C+%22%22%27%21%40%23%24%25%5E%26*%28%29%7B%7D%5B%5D%5C%7C%3F%2F%3C%3E%2C.") != 0) {
		printf("ERROR - LINE: %d\n", __LINE__);
		printf("Exp: \"%s\"\n", "strcpy%28str%2C+%22%22%27%21%40%23%24%25%5E%26*%28%29%7B%7D%5B%5D%5C%7C%3F%2F%3C%3E%2C.");
		printf("Get: \"%s\"\n", out);
	} else {
		printf("TEST OK\n");
	}
}








