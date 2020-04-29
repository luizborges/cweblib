
#include <assert.h>
#include <headers/stackTracer.h>
#include <headers/session_fileMap.h>


#define _ printf("Get here...: %d\n", __LINE__);


void test1();
//void test2(ArrayList_noSync_t list);
//void test3(ArrayList_noSync_t list);

int main(void) {
	MM_Init();
	StackTracer_Init();
	CWeb_Session_Init("", 300, 1800);
	test1(); 	
 	
	return 0;
}

void test1() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	
}








