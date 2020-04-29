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
	if(fread(&ts, sizeof(time_t), 1, f) != 1) {
		Error("reading session time of creation.\nSession File is \"%s\"\n"
			"File pointer is %p", session->fname, f);
	}
	
	time_t current = time(NULL);
	
	time_t diff = current - ts;
	if(diff > session->life) {
		MError("This section is expired.\nTime Session: %li\nStr Time Session: \"%s\"\n"
		"Current Time: %li\nStr Current Time: \"%s\"\nMax life Time Session (seg) is %li\n",
		"Time difference between current time and session time is %li seg.",
		ts, ctime(&ts), current, ctime(&current), session->life, diff);
		return true;
	}
	
	char *strTimeSession;
	if(fread(strTimeSession, 26, 1, f) != 1) {
		Error("reading session string time of creation.\nSession File is \"%s\"\n"
			"Session Time is %li\nFile pointer is %p", ts, session->fname, f);
	}
	
	return false;
}

static void
_Session_Load(Session_t session,
				FILE *f)
{
	if(fread(&session->maxLenMapKey, sizeof(session->maxLenMapKey), 1, f) != 1) {
		Error("reading session the value of greatest length map key.\n"
		"Session File is \"%s\"\nFile pointer is %p", session->fname, f);
	}
	
	char *key = MM_Malloc(session->maxLenMapKey*sizeof(char));
	if(key == NULL) {
		Error("In allocation memory for reading key of session.\n"
		"The value of greatest length map key of session is %d.\nSession File is \"%s\"\n"
		"File pointer is %p", session->maxLenMapKey, session->fname, f);
	}
	
	char end[2];
	sprintf(end, "%c", EOF);
	while(FileUtil_Get_Str(key, session->maxLenMapKey, f) != -1)
	{
		Content_t cont = MM_Malloc(sizeof(Content_o));
		if(cont == NULL) {
			Error("In allocation memory for session content.\nKey is \"%s\"\n"
				"Session File is \"%s\"\nFile pointer is %p", key, session->fname, f);
		}
		
		if(fread(&cont->size, sizeof(size_t), 1, f) != 1) {
			Error("reading session data content size on session file.\nkey is \"%s\"\n"
				"Session File is \"%s\"\nFile pointer is %p", key, session->fname, f);
		}
		
		if(fread(cont->value, cont->size, 1, f) != 1) {
			Error("reading session data content on session file.\nkey is \"%s\"\n"
				"Session File is \"%s\"\nFile pointer is %p", key, session->fname, f);
		}
		
		session->map->Set(session->map->self, key, cont); // insere no map
	}
	
	if(!feof(f)) { // check the end of file
		Error("Not end of Session File. - Something goes wrong."
			"Session File is \"%s\"\nFile pointer is %p", session->fname, f);
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
void
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
	fclose(fs);
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
		Error("Size of Session Content Value cannot be 0.");
	}
		
	Content_t cont = MM_Malloc(sizeof(Content_o));
	if(cont == NULL) {
		Error("In allocation memory for session content.\nKey is \"%s\"\n"
			"size of content value is %lu", key, size);
	}
	
	Session_t session = _Session_Singleton();
	session->map->Set(session->map->self, key, cont);
	
	if(strlen(key) >= session->maxLenMapKey) { // update max len key
		session->maxLenMapKey = strlen(key) +1;
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
	
	////////////////////////////////////////////////////////////////////////////////
	// Insert data in file session
	////////////////////////////////////////////////////////////////////////////////
	FILE *f = fopen(fname, "w");
	if(f == NULL) {
		Error("Creating session file.\nfile name is \"%s\"\nVariable errno is %d\n"
		"Default msg error is \"%s\"", fname, errno, strerror(errno));
	}
	
	if(fwrite(&current, sizeof(time_t), 1, f) != 1) {
		Error("writting session time of creation.\nTime is %li\nStr Time is \"%s\"",
			current, ctime(&current));
	}
	if(fwrite(ctime(&current), 26, 1, f ) != 1) {
		Error("writting session name string time of creation\nTime is %lu\n"
			"Str Time is \"%s\"", current, ctime(&current));
	}
	if(fwrite(&session->maxLenMapKey, sizeof(int), 1, f) != 1) {
		Error("writting session the value of greatest length map key.\n"
			"max len is %lu", session->maxLenMapKey);
	}
	
	int len = -1;
	char **key = session->map->Key(session->map->self, &len);
	for(int i=0; i < len; ++i) { // write the map file
		Content_t cont = session->map->Get(session->map->self, key[i]);
		if(fwrite(key[i], strlen(key[i])+1, 1, f) != 1) {
			Error("writting session name variable on session file.\nkey[%d] is \"%s\"\n"
			"total length is %lu", i, key[i], strlen(key[i])+1);
		}
		if(fwrite(&cont->size, sizeof(size_t), 1, f) != 1) {
			Error("writting session data content size on session file.\nkey[%d] is \"%s\"\n"
			"size of value is %lu", i, key[i], cont->size);
		}
		if(fwrite(cont->value, cont->size, 1, f) != 1) {
			Error("writting session data content on session file.\nkey[%d] is \"%s\"\n"
			"size of value is %lu", i, key[i], cont->size);
		}
	}
	
	fclose(f);
	session->isConfig = false;
	
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

































