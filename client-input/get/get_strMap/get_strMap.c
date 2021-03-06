/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../../../cweb.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct
{

//
	char *str; // 2048 - size default with character '\0'
	int  size;
	
	map_t map;
} Get_StrMap_o;


typedef Get_StrMap_o* Get_StrMap_t;
////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////
static Get_StrMap_t _Get_StrMap_New();
static Get_StrMap_t _Get_StrMap_Singleton();
////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////
static inline void
_Get_StrMap_Fill_Map(const char *getStr,
                     const int   getStrLen,
                     char *getStrDecode,
                     map_t map)
{
	int posInitNextStr = 0;
	for(int i=0; i < getStrLen; ++i)
	{
		///////////////////////////////////////////////////////////////////
		// busca, decodifica chave do GET - um par de key e conteúdo
		///////////////////////////////////////////////////////////////////
		const char *key = &getStr[i];
		if(key == NULL) {
			Error("in get the key of the HTTP REQUEST METHOD GET.\nHTTP REQUEST METHOD GET = \"%s\"", getStr);
		}
		
		int keyLen = 0;
		for(; i < getStrLen && key[keyLen] != '='; ++keyLen) {++i;} // descobre o tamanho de key
		
		
		char *keyDecode = &getStrDecode[posInitNextStr];
		if(CWeb_Percent_Decode(keyDecode, key, keyLen) != true) { // decode the key
			Error("decoding key in HTTP REQUEST METHOD GET.\nkey enconding: \"%s\"\nkey decode (wrong value): \"%s\"\nKey length: %d\nHTTP GET: \"%s\"\n", key, keyDecode, keyLen, getStr);
		}
		posInitNextStr += strlen(keyDecode) +2; // atualiza a nova posição inicial
		
		///////////////////////////////////////////////////////////////////
		// busca, decodifica contéudo do GET - um par de key e contéudo
		///////////////////////////////////////////////////////////////////
		const char *content = &getStr[++i]; // seta o início da string do content
		int contentLen = 0;
		for(; i < getStrLen && content[contentLen] != '&';
			++contentLen) { ++i; } // descobre o tamanho do content
		
		
		char *contentDecode = &getStrDecode[posInitNextStr]; // recebe a posição no array de conteúdo própria para guardar o valor
		if(CWeb_Percent_Decode(contentDecode, content, contentLen) != true) { // decode o conteúdo do par do get
			Error("decoding content in HTTP REQUEST METHOD GET.\ncontent enconding: \"%s\"\nkey decode (wrong value): \"%s\"\ncontent length: %d\nHTTP GET: \"%s\"\n", content, contentDecode, contentLen, getStr);
		}
		posInitNextStr += strlen(contentDecode) +2; // atualiza a nova posição inicial
		
		///////////////////////////////////////////////////////////////////
		// insere no map o key e o conteúdo - atualiza a posição na array do get
		///////////////////////////////////////////////////////////////////
		map->Set(map->self, keyDecode, contentDecode);
	}
}


////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////
static Get_StrMap_t
_Get_StrMap_New()
{
	Get_StrMap_t get = MM_Malloc(sizeof(Get_StrMap_o));
	if(get == NULL) {
		Error("Allocated Space for Client Input Get - String Map");
	}
	
	get->str = MM_Malloc(2048);
	if(get->str == NULL) {
		Error("Allocated Space for Client Input Get - String Default");
	}
	
	get->size = 2048;
	
	get->map = Abstract_Factory_Common(Map);
	if(get->map == NULL) {
		Error("creating map");
	}
	
	return get;
}


static Get_StrMap_t
_Get_StrMap_Singleton()
{
	static Get_StrMap_t get = NULL;
	
	if(get == NULL)
	{
		get = _Get_StrMap_New();
		if(get == NULL)
		{
			Error("could not create singleton instance of Client Input Get.");
		}
	}
	
	return get;
}

////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
/**
 * Insere a string do http request method get in a map format
 */
bool
CWeb_ClientInput_Get_Init()
{
	///////////////////////////////////////////////////////////////////
	// incializa o GET e deixa ele limpo
	///////////////////////////////////////////////////////////////////
	// necessario para evitar erros e falhas de seguranca e 
	// ficar sicronizado com o estado atual do http
	Get_StrMap_t get = _Get_StrMap_Singleton();
	get->map->Clean(get->map->self); // remove todas as chaves do map
	
	
	
	// verifica se é possível inicializar o get
	char *rm = getenv("REQUEST_METHOD");
	if(rm == NULL) return false;
	if(strcmp(rm, "GET") != 0) {
		return false;
	}
	
	///////////////////////////////////////////////////////////////////
	// recebe o get string
	///////////////////////////////////////////////////////////////////
	char *getStr = getenv("QUERY_STRING"); // recupera a string 
	if(getStr == NULL) {
		Error("HTTP REQUEST METHOD GET STRING is NULL.");
	}
	int getStrLen = strlen(getStr);
	
	if(get->size <= getStrLen)
	{
		get->size = getStrLen +1; // for character `\0`
		MM_Free(get->str);
		
		get->str = MM_Malloc(get->size);
		if(get->str == NULL) {
			Error("Allocated Space for Get - String Default");
		}
	}
	
	///////////////////////////////////////////////////////////////////
	// insere a string no map
	///////////////////////////////////////////////////////////////////
	_Get_StrMap_Fill_Map(getStr, getStrLen, get->str, get->map);
	
	
	return true; // foi possível inicializar o HTTP REQUEST METHOD GET no mapa de busca
}


/**
 * 
 */
char*
ClientInput_Get_StrMap_Get(const char *get_key)
{
	if(get_key == NULL) { // check arg
		Error("the key to fetch must be not NULL.");
	}
	
	Get_StrMap_t get = _Get_StrMap_Singleton();
	
	if(get->map->HasKey(get->map->self, get_key) == true) {
		return get->map->Get(get->map->self, get_key);
	} else {
		int len = -1;
		char **key = get->map->Key(get->map->self, &len);
		fprintf(stderr, "List of all keys in HTTP REQUEST METHOD GET that be parsed.\n");
		for(int i=0; i < len; ++i) {
			fprintf(stderr, "[%d] :: \"%s\"\n", i+1, key[i]);
		}
		
		Error("fetch for a no key of HTTP REQUEST METHOD GET.\nfectch key = \"%s\"", get_key);
	}
	
	return NULL; // nunca é alcançado
}














