
#include <assert.h>
#include <headers/stackTracer.h>
#include "cookie_strMap.h"


#define _ printf("Get here...: %d\n", __LINE__);


void test1();
void test2();
//void test3(ArrayList_noSync_t list);

int main(void) {
	MM_Init();
	StackTracer_Init();

	test1(); 	
 	test2();
 	
	return 0;
}

void test1() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	
	char *cookie = CWeb_Cookie_Set("sid", 
		"my ful name", 180, "www.google.com",
		"/home/borges", true, true);
	printf("cookie 1::\n\"%s\"\n", cookie);
	time_t t = time(NULL);
	printf("time: %s\n", ctime(&t));
}

void test2() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	
	CWeb_Cookie_Print("sid", 
		"my ful name", 180, "www.google.com",
		"/home/borges", true, true);
	time_t t = time(NULL);
	printf("time: %s\n", ctime(&t));
}






