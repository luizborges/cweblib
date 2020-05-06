
#include <assert.h>
#include <headers/stackTracer.h>
#include <headers/memoryManager.h>
//#include "session_fileMap.h"
#include <headers/cweb.h>


#define _ printf("Get here...: %d\n", __LINE__);

typedef struct
{
	char *dir;
	char *fname;
	time_t life; // life session
	bool isConfig; // to check if is lost configuration
	int  maxLenMapKey; // maior tamanho da key do map - strlen(key) + 1
	
	map_t map;
} Session_o;

typedef Session_o* Session_t;

char *sid = NULL;

void test1();
void test2();
void test3();
void test4();
//void test3(ArrayList_noSync_t list);
Session_t session;
int main(void) {
	MM_Init();
	StackTracer_Init();
	session = CWeb_Session_Init("/media/borges/data/cloud/Mega/programming/c/cweblib/session/session_fileMap/test", 300, 1800);
	test1(); 
	test2();
	test3();
	test4();
	return 0;
}

void test1() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	CWeb_Session_Set("sid", "1234567890", strlen("1234567890")+1);
	int *sum = MM_Malloc(sizeof(int));
	*sum = 9;
	CWeb_Session_Set("sum", sum, sizeof(int));
	int x = -1;
	char **r = session->map->Key(session->map->self, &x);
	for(int i=0; i < x; ++i) {
		printf("%s::[%d] of %d\n", __func__, i, x);
		printf("%s::key is \"%s\"\n", __func__, (char*)r[i]);
	}
	sid = CWeb_Session_Save();
}

void test2() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	CWeb_Session_Load(sid);
}

void test3() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	size_t s;
	char *str = CWeb_Session_Get("sid", &s);
	printf("%s::%s is \"%s\"\n", __func__, "sid", str);
	
	int *i = CWeb_Session_Get("sum", &s);
	printf("%s::%s is \"%d\" | size is %li | sizeof(int) is %lu\n",
	__func__, "sum", *i, s, sizeof(int));
	
	int *j = MM_Malloc(10*sizeof(int));
	for(int z=0; z < 10; z++) {
		j[z] = -z;
	}
	CWeb_Session_Set("array", j, 10*sizeof(int));
	CWeb_Session_Del("sum", NULL);
	sid = CWeb_Session_Save();
}


void test4() {
	printf("\n*******************************************************\n");
	printf("Func: %s\n", __func__);
	printf("*******************************************************\n");
	CWeb_Session_Load(sid);
	size_t s;
	char *str = CWeb_Session_Get("sid", &s);
	printf("%s::%s is \"%s\"\n", __func__, "sid", str);
	
	int *j = CWeb_Session_Get("array", &s);
	s = s / sizeof(int);
	printf("%s::size *j is %ld \n", __func__, s);
	for(int z=0; z < s; z++) {
		printf("%s::j[%d] = %d \n", __func__, z, j[z]);
	}
	//int *i = CWeb_Session_Get("sum", &s);
	CWeb_Session_End();
/*	CWeb_Session_Load(sid);
	str = CWeb_Session_Get("sid", &s);*/
//	CWeb_Session_Set("array", j, 10*sizeof(int));
//	CWeb_Session_Del("sum", NULL);
//	CWeb_Session_Save();
}



	































