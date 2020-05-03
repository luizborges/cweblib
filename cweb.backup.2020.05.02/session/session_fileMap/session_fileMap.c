/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "session_fileMap.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////
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

// conteúdo da sessão
typedef struct
{
	void *value;
	size_t size; // size = sizeof() * numElem
} Content_o;

typedef Content_o* Content_t;

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////
static FILE*
_Session_Get_File(Session_t session, const char *sid) 
{
	//char *sid = CWeb_Cookie_Get("sid", NULL);
	if(sid == NULL) {
		return NULL;
	}
	
//	char TESTSID[20] = "test"; // for test
//	sid = TESTSID; // for test
	if(sid == NULL) {
		MError("No \"sid\" for loading File Session.\nsid is NULL.");
		return NULL;
	}
	
	int size = strlen(session->dir) + strlen(sid) + 9; // 9 = strlen(".session") + '\0'
	session->fname = (char*)MM_Malloc(size*sizeof(char));
	if(session == NULL) {
		Error("Allocated Space for Sesion File Name.\n Size is %d", size);
	}
	sprintf(session->fname, "%s%s.session", session->dir, sid); // recebe o nome do arquivo
	
	FILE *fs = fopen(session->fname, "rb");
	if(fs == NULL) {
		MError("File Session is \"%s\"\nerro is %d\nstr erro is \"%s\"",
			session->fname, errno, strerror(errno));
	}
	
	return fs;
}

static bool
_Session_Is_Expired(Session_t session,
					FILE *f)
{
	time_t ts;
	
	if(fread(&ts, sizeof(time_t), 1, f) != 1) {
		Error("reading session time of creation.\nSession File is \"%s\"\n"
			"File pointer is %p\nerro is %d\nstr erro is \"%s\"",
			session->fname, f, errno, strerror(errno));
	}
	
	char strTimeSession[26];
	
	if(fread(strTimeSession, sizeof(char), 26, f) != 26) {
		Error("reading session string time of creation.\nSession File is \"%s\"\n"
			"Session Time is %li\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
			ts, session->fname, f, errno, strerror(errno));
	}
	
	time_t current = time(NULL);
	char strc[26];
	strcpy(strc, ctime(&current)); // it is necessary, sometimes ctime gives errors.

	time_t diff = current - ts;

	if(diff > session->life) {
		MError("This section is expired.\nSession life is %li\nCurrent is %li\n"
		"Time Session is %li\nTime difference is %li",
		session->life, current, ts, diff);
		return true;
	}
	
	
	
	return false;
}

static void
_Session_Load(Session_t session,
				FILE *f)
{
   	if(fread(&session->maxLenMapKey, sizeof(session->maxLenMapKey), 1, f) != 1) {
   		Error("reading session the value of greatest length map key.\n"
		"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
		session->fname, f, errno, strerror(errno));
	}
	
   	if(session->maxLenMapKey < 2) { // check value
		Error("session maximum length of key is less than 2.\n"
		"session maximum length of key is %d", session->maxLenMapKey);
	}
	
   	fprintf(stderr, "%s::session->maxLenMapKey is %d\n", __func__, session->maxLenMapKey);
   	char *key = MM_Malloc(session->maxLenMapKey*sizeof(char));
   	if(key == NULL) {
		Error("In allocation memory for reading key of session.\n"
		"The value of greatest length map key of session is %d.\nSession File is \"%s\"\n"
		"File pointer is %p", session->maxLenMapKey, session->fname, f);
	}
	
	int numKey = -1;
   	if(fread(&numKey, sizeof(int), 1, f) != 1) {
   		Error("reading the number  map key.\n"
		"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
		session->fname, f, errno, strerror(errno));
	}
	
   	if(numKey < 1) { // check value
		Error("number of map key is less than 2.\n"
		"number of map key is %d", numKey);
	}
	
	fprintf(stderr, "%s::numKey is %d\n", __func__, numKey);
	
   	for(int i=0; i < numKey; ++i)
	{
   		fprintf(stderr, "%s::[%d] of %d\n", __func__, i+1, numKey);
   		int len = -1;
   		if(fread(&len, sizeof(int), 1, f) != 1) {
			Error("reading the length of map key.\nid of key is %d\n"
				"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
				i, session->fname, f, errno, strerror(errno));
		}
		
   		if(len < 2) {
			Error("Session size of length of key map is less than 2\n"
			"size of length of key map is is %d\n"
			"Session File is \"%s\"\nFile pointer is %p\nNumber of Key is %d",
			len, session->fname, f, numKey);
		}
		
   		fprintf(stderr, "%s::len is %d\n", __func__, len);
		
   		if(fread(key, sizeof(char), len, f) != len) {
			Error("reading map key.\nid of key is %d\nLength of key is %d"
				"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
				i, len, session->fname, f, errno, strerror(errno));
		}
		
   		fprintf(stderr, "%s::key is \"%s\"\n", __func__, key);
		
		Content_t cont = MM_Malloc(sizeof(Content_o));
		if(cont == NULL) {
			Error("In allocation memory for session content.\nKey is \"%s\"\n"
				"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
				key, session->fname, f, errno, strerror(errno));
		}
		
   		if(fread(&cont->size, sizeof(size_t), 1, f) != 1) {
			Error("reading session data content size on session file.\nkey is \"%s\"\n"
				"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
				key, session->fname, f, errno, strerror(errno));
		}
		
		if(cont->size < 1) {
			Error("Session size of value of key session is less than 1\n"
			"Size of memory to alloc value is %li\n"
			"Session File is \"%s\"\nFile pointer is %p\nKey is \"%s\"",
			cont->value, session->fname, f, key);
		}
		
		cont->value = MM_Malloc(cont->size);
   		if(cont->value == NULL) {
			Error("In allocation memory for reading value of key of session.\n"
			"The value of greatest length map key of session is %d.\n"
			"Session File is \"%s\"\nFile pointer is %p\n"
			"erro is %d\nstr erro is \"%s\"",
			session->maxLenMapKey, session->fname, f, errno, strerror(errno));
		}
		
   		if(fread(cont->value, 1, cont->size, f) != cont->size) {
			Error("reading session data content on session file.\nkey is \"%s\"\n"
				"Session File is \"%s\"\nFile pointer is %p\nerro is %d\nstr erro is \"%s\"",
				key, session->fname, f, errno, strerror(errno));
		}
		
   		session->map->Set(session->map->self, key, cont); // insere no map
	}
}

////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////
static Session_t
_Session_New()
{
	Session_t session = (Session_t)MM_Malloc(sizeof(Session_o));
	if(session == NULL) {
		Error("Allocated Space for Sesion - File Map");
	}
	
	session->map = Abstract_Factory_Common(Map);
	if(session->map == NULL) {
		Error("creating map");
	}
	
	session->isConfig = false;
	session->life = -1;
	session->fname = NULL;
	session->dir = NULL;
	session->maxLenMapKey = -1; // indica map vazio
	
	return session;
}


static Session_t
_Session_Singleton()
{
	static Session_t session = NULL;
	
	if(session == NULL)
	{
		session = _Session_New();
		if(session == NULL)
		{
			Error("could not create singleton instance of Session File Map.");
		}
	}
	
	return session;
}


////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
void*
CWeb_Session_Init(const char *DirFileSession,
				  const time_t lifeSession,
				  const time_t del)
{
	Session_t session = _Session_Singleton();
	
	if(DirFileSession != NULL)
	{
		int size = strlen(DirFileSession);
		session->dir = (char*)MM_Malloc((size+1)*sizeof(char));
		if(session->dir == NULL) {
			Error("In allocation memory for directory.\nSize is %ld\n"
				"Directory is \"%s\"", size+1, DirFileSession);
		}
		strcpy(session->dir, DirFileSession);
		
		// TODO - trocar para quando for o windows ou linux
		// insere o character '/' ao final da str
		if(session->dir[size-1] != '/' && strlen(session->dir) > 1) {
			session->dir = (char*)MM_Realloc(session->dir, (size+2)*sizeof(char));
			if(session->dir == NULL) {
				Error("In reallocation memory for directory.\nSize is %ld\n"
					"Directory is \"%s\"", size+2, DirFileSession);
			}
			
			session->dir[size] = '/';
			session->dir[size+1] = '\0';
		}
	}
	
	session->life = lifeSession;
	
	//_Session_Clean(del); // TODO - implementation

	session->isConfig = true;
	return session;
}

bool
CWeb_Session_Load(const char *sid)
{
   	Session_t session = _Session_Singleton();
   	if(session->isConfig == false) {
	   	Error("Session is not configured.");
   	}
   	
   	FILE* fs = _Session_Get_File(session, sid);
   	if(fs == NULL) {
   		return false;
   	}
   	
   	if(_Session_Is_Expired(session, fs) == true) {
   		return false;
   	}
   	
   	_Session_Load(session, fs);
   	fclose(fs);
   	
   	
//  	fprintf(stderr, "\n%s::****************************\n", __func__);
//   	fprintf(stderr, "%s::CHECK MAP SESSION\n", __func__);
// 	int x = -1;
//	char **r = session->map->Key(session->map->self, &x);
/*	for(int i=0; i < x; ++i) {
		fprintf(stderr, "%s::[%d] of %d\n", __func__, i, x);
		fprintf(stderr, "%s::key is \"%s\"\n", __func__, r[i]);
	} */
    return true;
}


void*
CWeb_Session_Get(const char *key,
				size_t *size)
{
	if(key == NULL) {
		Error("Key Session cannot be NULL.");
	}
	
	Session_t session = _Session_Singleton();
	
	if(session->map->HasKey(session->map->self, key) == true) {
		Content_t cont = session->map->Get(session->map->self, key);
		if(size != NULL) { // pode ser que o não se tenha interesse em recuperar o size
			*size = cont->size;
		}
		return cont->value;
	}
	
	int len = -1;
	char **_key = session->map->Key(session->map->self, &len);
	MError("fetch for a no key of SESSION.\nfectch key = \"%s\"\n"
		"List of all keys in SESSION that be parsed:\nNumber of keys is %d\n", key, len);
	for(int i=0; i < len; ++i) {
		fprintf(stderr, "[%d] :: \"%s\"\n", i+1, _key[i]);
	}
	
	return NULL;
}

void
CWeb_Session_Set(const char *key,
				 const void *value,
				 const size_t size)
{
	if(key == NULL) {
		Error("Key Session cannot be NULL.");
	}
	if(strlen(key) < 1) {
		Error("Key Session cannot be a empty string.");
	}
	if(size < 1) {
		Error("Size of Session Content Value cannot be 0.\n"
			"Size is sizeof(type) * numElements.");
	}
	
	Content_t cont = MM_Malloc(sizeof(Content_o));
	if(cont == NULL) {
		Error("In allocation memory for session content.\nKey is \"%s\"\n"
			"size of content value is %lu", key, size);
	}
	
	cont->value = (void*)value;
	cont->size  = (size_t)size;
	
	Session_t session = _Session_Singleton();
	session->map->Set(session->map->self, key, cont);
	
	int len = strlen(key) +1;
	/*printf("\n%s::session->maxLenMapKey is %d\n", __func__, session->maxLenMapKey);
	printf("%s::key is \"%s\"\n", __func__, key);
	printf("%s::strlen(key) is %li\n", __func__, strlen(key));*/
	
	if(len >= session->maxLenMapKey) { // update max len key
		session->maxLenMapKey = strlen(key) +1;
	}
	//printf("%s::session->maxLenMapKey is %d\n", __func__, session->maxLenMapKey);
	
	int x = -1;
	char **r = session->map->Key(session->map->self, &x);
	for(int i=0; i < x; ++i) {
		fprintf(stderr, "%s::[%d] of %d\n", __func__, i+1, x);
		fprintf(stderr, "%s::key is \"%s\"\n", __func__, r[i]);
	}
}

void*
CWeb_Session_Del(const char *key,
				size_t *size)
{
	if(key == NULL) {
		Error("Key Session cannot be NULL.");
	}
	
	Session_t session = _Session_Singleton();
	
	if(session->map->HasKey(session->map->self, key) == true) {
		Content_t cont = session->map->Del(session->map->self, key);
		if(size != NULL) { // pode ser que o não se tenha interesse em recuperar o size
			*size = cont->size;
		}
		
		if((strlen(key)+1) == session->maxLenMapKey) // discover the new maxLenMapKey
		{
			session->maxLenMapKey = -1; // reset value to treat empty map
			int len = -1;
			char **_key = session->map->Key(session->map->self, &len);
			for(int i=0; i < len; ++i) { // discover the greastest value
				if(strlen(_key[i]) >= session->maxLenMapKey) {
					session->maxLenMapKey = strlen(_key[i]) +1;
				}
			}
		}
		
		return cont->value;
	}
		int len = -1;
		char **_key = session->map->Key(session->map->self, &len);
		MError("Fetch for a no key of SESSION.\nfectch key = \"%s\"\n"
			"List of all keys in SESSION that be parsed:", key);
		for(int i=0; i < len; ++i) {
			fprintf(stderr, "[%d] :: \"%s\"\n", i+1, _key[i]);
		}
	
	return NULL;
}

char*
CWeb_Session_Save()
{
	Session_t session = _Session_Singleton();
	
	remove(session->fname); // remove old file
	session->fname = NULL;
	
	////////////////////////////////////////////////////////////////////////////////
	// create file session name
	////////////////////////////////////////////////////////////////////////////////
	srand(time(NULL));
	int al = rand();
	time_t current = time(NULL);
	char *sid = MM_Malloc((sizeof(int)+sizeof(time_t)+1)*sizeof(char));
	if(sid == NULL) {
		Error("In allocation memory for name of file session.\nSize is %ld\n",
			(sizeof(int)+sizeof(time_t)+1)*sizeof(char));
	}
	sprintf(sid, "%li%d", current, al);
	
	int size = strlen(session->dir) + strlen(sid) + strlen(".session") +1;
	char *fname = (char*)MM_Malloc(size*sizeof(char));
	if(fname == NULL) {
		Error("In allocation memory for name of file session.\nSize is %ld\n", size);
	}
	sprintf(fname, "%s%s.session", session->dir, sid);
	fprintf(stderr, "%s::fname is %p | len is %d | sum: %p\n", __func__, fname, size*sizeof(char), fname + size*sizeof(char));
	// FOR TEST
	//char sidT[300] = "test"; // for test
	//sprintf(fname, "%s%s.session", session->dir, sidT);
	//fname = sidT;
	fprintf(stderr, "%s::fname is \"%s\"\n", __func__, fname);
	fprintf(stderr, "%s::session->maxLenMapKey is %d\n", __func__, session->maxLenMapKey);
	
	////////////////////////////////////////////////////////////////////////////////
	// Insert data in file session
	////////////////////////////////////////////////////////////////////////////////
	FILE *f = fopen(fname, "wb");
	if(f == NULL) {
		Error("Creating session file.\nfile name is \"%s\"\nVariable errno is %d\n"
		"Default msg error is \"%s\"\nerro is %d\nstr erro is \"%s\"",
		fname, errno, strerror(errno), errno, strerror(errno));
	}
	
	if(fwrite(&current, sizeof(time_t), 1, f) != 1) {
		Error("writting session time of creation.\nTime is %li\nStr Time is \"%s\""
		"\nerro is %d\nstr erro is \"%s\"",
			current, ctime(&current), errno, strerror(errno));
	}
	
	if(fwrite(ctime(&current), sizeof(char), 26, f ) != 26) {
		Error("writting session name string time of creation\nTime is %lu\n"
			"Str Time is \"%s\"\nerro is %d\nstr erro is \"%s\"",
			current, ctime(&current), errno, strerror(errno));
	}
	
	
	int numKey = -1;
	char **key = session->map->Key(session->map->self, &numKey);
	if(numKey < 1) {
		MError("nothing to save in Session File.\nNumber of Keys is %d", numKey);
		return NULL;
	}
	
	if(fwrite(&session->maxLenMapKey, sizeof(int), 1, f) != 1) {
		Error("writting session the value of greatest length map key.\n"
			"max len is %lu\nerro is %d\nstr erro is \"%s\"",
			session->maxLenMapKey, errno, strerror(errno));
	}
	
	if(fwrite(&numKey, sizeof(int), 1, f) != 1) {
		Error("writting the number of session map key.\n"
			"number of key is %lu\nerro is %d\nstr erro is \"%s\"",
			numKey, errno, strerror(errno));
	}
	
	for(int i=0; i < numKey; ++i) { // write the map file
		Content_t cont = session->map->Get(session->map->self, key[i]);
		
		int len = strlen(key[i]) +1;
		fprintf(stderr, "\n%s::[%d] of %d\n", __func__, i+1, numKey);
		fprintf(stderr, "%s::len is %d\n", __func__, len);
		fprintf(stderr, "%s::key is \"%s\"\n", __func__, key[i]);
		fprintf(stderr, "%s::size is %ld\n", __func__, cont->size);
		fprintf(stderr, "%s::value is \"%s\"\n", __func__, (char*)cont->value);
		if(fwrite(&len, sizeof(int), 1, f) != 1) {
			Error("writting the length of session map key.\n"
			"key is \"%s\"\nLength is %d\nerro is %d\nstr erro is \"%s\"",
			key[i], len, errno, strerror(errno));
		}
		
		if(fwrite(key[i], sizeof(char), len, f) != len) {
			Error("writting session name variable on session file.\nkey[%d] is \"%s\"\n"
			"total length is %lu\nerro is %d\nstr erro is \"%s\"",
			i, key[i], len, errno, strerror(errno));
		}
		if(fwrite(&cont->size, sizeof(size_t), 1, f) != 1) {
			Error("writting session data content size on session file.\nkey[%d] is \"%s\"\n"
			"size of value is %lu\nerro is %d\nstr erro is \"%s\"",
			i, key[i], cont->size, errno, strerror(errno));
		}
		if(fwrite(cont->value, 1, cont->size, f) != cont->size) {
			Error("writting session data content on session file.\nkey[%d] is \"%s\"\n"
			"size of value is %lu\nerro is %d\nstr erro is \"%s\"",
			i, key[i], cont->size, errno, strerror(errno));
		}
	}
	
	fclose(f);
		
	return sid;
}

void
CWeb_Session_End()
{
	Session_t session = _Session_Singleton();
	
	remove(session->fname); // remove old file
	session->fname = NULL;
	
	////////////////////////////////////////////////////////////////////////////////
	// remove all map keys of sessions
	////////////////////////////////////////////////////////////////////////////////
	int len = -1;
	char **key = session->map->Key(session->map->self, &len);
	for(int i=0; i < len; ++i) { // write the map file
		session->map->Del(session->map->self, key[i]);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// reset the values
	////////////////////////////////////////////////////////////////////////////////
	session->maxLenMapKey = -1;
}

































