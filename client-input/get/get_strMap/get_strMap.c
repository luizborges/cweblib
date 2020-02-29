/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "get_strMap.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct
{

//
	char *strD; // 2048 - size default with character '\0'
	int strDSize;
	
	char *strL; // para tratar quando a getenv("REQUEST_METHOD") for maior que 2048
	int strLSize;
	
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
static int _Get_StrMap_Percent_Decode(char* out, const char* in, const int maxDecode);
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
		if(_Get_StrMap_Percent_Decode(keyDecode, key, keyLen) != 0) { // decode the key
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
		if(_Get_StrMap_Percent_Decode(contentDecode, content, contentLen) != 0) { // decode o conteúdo do par do get
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
static int
_Get_StrMap_Percent_Decode(char* out,
                           const char* in,
                           const int maxDecode)
{
    static const char tbl[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };
    char c, v1, v2, *beg=out;
    int numDecode = 0;
    if(in != NULL) {
//        while(((c=*in++) != '\0') && numDecode < maxDecode) {
		while(numDecode < maxDecode) {
			c=*in++;
            if(c == '%') {
                if((v1=tbl[(unsigned char)*in++])<0 || 
                   (v2=tbl[(unsigned char)*in++])<0) {
                    *beg = '\0';
                    return -1;
                }
                c = (v1<<4)|v2;
                numDecode += 2;
            } else if(c == '+') {
            	*out++ = ' ';
            	++numDecode;
            	continue;
            }
            *out++ = c;
            ++numDecode;
        }
    }
    *out = '\0';
    return 0;
}

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
	
	get->strD = MM_Malloc(2048);
	if(get->strD == NULL) {
		Error("Allocated Space for Client Input Get - String Default");
	}
	
	get->strDSize = 2048;
	get->strL = NULL;
	get->strLSize = -1;
	
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
	// verifica se é possível inicializar o get
	char *rm = getenv("REQUEST_METHOD");
	if(rm == NULL) return false;
	if(strcmp(rm, "GET") != 0) {
		return false;
	}
	
	Get_StrMap_t get = _Get_StrMap_Singleton();
	
	get->map->Clean(get->map->self); // remove todas as chaves do map
	
	///////////////////////////////////////////////////////////////////
	// recebe o get string
	///////////////////////////////////////////////////////////////////
	char *getStr = getenv("QUERY_STRING"); // recupera a string 
	if(getStr == NULL) {
		Error("HTTP REQUEST METHOD GET STRING is NULL.");
	}
	int getStrLen = strlen(getStr);
	
	char *getStrDecode = NULL; // string para unificação do tratamento
	if(getStrLen < get->strDSize) // dentro do tamanho padrão
	{
		getStrDecode = get->strD;
	} else // maior que o tamanho padrão
	{
		if(get->strL == NULL) // se a string long not initialized
		{
			get->strLSize = getStrLen +1; // por causa do charactere '\0'
			get->strL = MM_Malloc(get->strLSize);
			if(get->strL == NULL) {
				Error("in allocated memory to http request method get string.\n"
					"GET String length (return of strlen()): %d\nGET String = \"%s\"", getStrLen, getStr);
			}
		}
		else if(getStrLen >= get->strLSize) // se a string long for menor que o GET
		{
			get->strLSize = getStrLen +1; // por causa do charactere '\0'
			get->strL = MM_Realloc(get->strL, get->strLSize);
			if(get->strL == NULL) {
				Error("in allocated memory to http request method get string.\n"
					"GET String length (return of strlen()): %d\nGET String = \"%s\"", getStrLen, getStr);
			}
		}
		//else {} // - não é necessário fazer nada quando o GET é menor que a string long
		getStrDecode = get->strL;
	}
	
	///////////////////////////////////////////////////////////////////
	// insere a string no map
	///////////////////////////////////////////////////////////////////
	_Get_StrMap_Fill_Map(getStr, getStrLen, getStrDecode, get->map);
	
	
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














