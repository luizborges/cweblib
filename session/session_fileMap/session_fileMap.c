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
	time_t expire;
	bool isConfig; // to check if is lost configuration
	char *fname;
	
	map_t *map;
} Session_o;

typedef Session_o* Session_t;


typedef struct
{
	void *value;
	size_t size;
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
_Session_Get_File(Session_t session) 
{
	char *sid = CWeb_Cookie_Get("sid");
	if(sid == NULL) {
		return NULL;
	}
	
	int size = strlen(session->dir) + strlen(sid) +1;
	session->fname = (char*)MM_Malloc(size*sizeof(char));
	if(session == NULL) {
		Error("Allocated Space for Sesion File Name.\n Size is %d", size);
	}
	sprintf(session->fname, "%s%s", session->dir, sid); // recebe o nome do arquivo
	
	return fopen(session->fname, "r");
}

static bool
_Session_Is_Expired(Session_t session,
					FILE *f)
{
	time_t ts;
	fscanf(f, "%li", &ts);
	
	time_t current = time(NULL);
	
	time_t diff = current - ts;
	if(diff > session->expire) {
		MError("This section is expired.\nTime Session: %li\nStr Time Session: \"%s\"\n"
		"Current Time: %li\nStr Current Time: \"%s\"\nExpire Time Session (seg) is %li\n",
		ts, FileUtil_Get_Line(f), current, ctime(&time), diff);
		return true;
	}
	
	FileUtil_Get_Line(f); // pula para a próxima linha
	return false;
}

static void
_Session_Load((Session_t session,
				FILE *f)
{
	while((char *key = FileUtil_Get_Line(f)) != NULL)
	{
		size_t size = strlen(key);
		key[size-1] = '\0'; // retira o character '\n' do fim da string
		
		fscanf(f, "%zu", &size); // consegue o tamanho do arquivo
		fgetc(f); // go to next line
		void *cont = MM_Malloc(size);
		if(session->dir == NULL) {
			Error("In allocation memory for load content in session.\nSize is %ld\n", size);
		}
		
		fread(cont, size, 1, f);
		
		session->map->Set(session->map->self, key, cont); // insere no map
		fgetc(f); // go to next line
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
	
	session->sConfig = false;
	session->expire = -1;
	session->fname = NULL;
	session->dir = NULL;
	
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
void
CWeb_Session_Init(char *DirFileSession,
				  time_t lifeSession,
				  time_t del)
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
		
		if(session->dir[size-1] != '/') { // insere o character '/' ao final da str
			session->dir = (char*)MM_Realloc(session->dir, (size+2)*sizeof(char));
			if(session->dir == NULL) {
				Error("In reallocation memory for directory.\nSize is %ld\n"
					"Directory is \"%s\"", size+2, DirFileSession);
			}
			
			session->dir[size] = '/';
			session->dir[size+1] = '\0';
		}
	}
	
	session->expire = lifeSession;
	
	//_Session_Clean(del); // TODO - implementation

	session->isConfig = true;
}

bool
CWeb_Session_Load()
{
	Session_t session = _Session_Singleton();
	
	if(session->isConfig == false) {
		Error("Session is not configured.");
	}
	
	FILE* fs = _Session_Get_File(session);
	if(fs == NULL) {
		return false;
	}
	
	if(_Session_Is_Expired(session, fs) == true) {
		return false;
	}
	
	_Session_Load(session, fs);
	fclose(f);
	return true;
}


void*
CWeb_Session_Get(const char *key)
{
	if(key == NULL) {
		Error("Key Session cannot be NULL.");
	}
	
	Session_t session = _Session_Singleton();
	
	if(session->map->HasKey(session->map->self) == true) {
		return session->map->Get(session->map->self, key);
	} else {
		int len = -1;
		char **_key = session->map->Key(session->map->self, &len);
		MError("List of all keys in SESSION that be parsed:");
		for(int i=0; i < len; ++i) {
			fprintf(stderr, "[%d] :: \"%s\"\n", i+1, _key[i]);
		}
		
		Error("fetch for a no key of SESSION.\nfectch key = \"%s\"", key);
	}
	
	return NULL;
}

void
CWeb_Session_Set(const char *key,
				 const void *content,
				 const size_t size)
{
	if(key == NULL) {
		Error("Key Session cannot be NULL.");
	}
	
	Session_t session = _Session_Singleton();
	
	session->map->Set(session->map->self, key, content);
}

void
CWeb_Session_Save()
{
	Session_t session = _Session_Singleton();
	
	remove(session->fname); // remove old file
	
	////////////////////////////////////////////////////////////////////////////////
	// create file session name
	////////////////////////////////////////////////////////////////////////////////
	srand(time(NULL));
	int al = rand();
	time_t current = time(NULL);
	char *sid = MM_Malloc((sizeof(int)+sizeof(time_t)+1)*sizeof(char));
	if(sid == NULL) {
		Error("In allocation memory for name of file session.\nSize is %ld\n", size);
	}
	sprintf(sid, "%s%s", current, al);
	
	int size = strlen(session->dir) + strlen(sid) + strlen(".session") +1;
	fname = (char*)MM_Malloc(size*sizeof(char));
	if(fname == NULL) {
		Error("In allocation memory for name of file session.\nSize is %ld\n", size);
	}
	sprintf(fname, "%s%s.session", session->dir, sid);
	
	////////////////////////////////////////////////////////////////////////////////
	// Insert data in file session
	////////////////////////////////////////////////////////////////////////////////
	FILE *f = fopen(fname, "w");
	if(f == NULL) {
		Error("Creating session file.\nfile name is \"%s\"", fname);
	}
	
	fprintf(f, "%s\n%s\n", current, ctime(current)); // set the time of the session init
	
	int len = -1;
	char **key = session->map->Key(session->map->self, &len);
	for(int i=0; i < len; ++i) { // write the map file
		cont = session->map->Get(session->map->self, key[i]);
		fprintf(f, "%s\n%s\n", key[i], cont->size);
		if(fwrite(cont->value, cont->size, 1, f) == 0) {
			Error("writting session data on session file.\nkey[%d] is %d\n"
			"size of value is %d", i, key[i], cont->size);
		}
	}
	
	return sid;
}




































