/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "post_strMap.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////
typedef struct
{

	char  *str; // keep all the post encode content - key + value - é o post tal como recebido no stdin
	char  *value; // only the value of the post's decode content
	size_t size; // ambas as strings tem o mesmo tamanho -> str e value, always
	
	map_t map;
} Post_StrMap_o;


typedef Post_StrMap_o* Post_StrMap_t;

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////
static Post_StrMap_t _Post_StrMap_New();
static Post_StrMap_t _Post_StrMap_Singleton();
static int _Post_StrMap_Percent_Decode(char* out, const char* in, const int maxDecode);
////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////
static inline void
_Post_StrMap_Fill_Map(const char *postStr,
                     const size_t postStrLen,
                     char *postStrDecode,
                     map_t map)
{
	int posInitNextStr = 0;
	for(int i=0; i < postStrLen; ++i)
	{
		///////////////////////////////////////////////////////////////////
		// busca, decodifica chave do post - um par de key e conteúdo
		///////////////////////////////////////////////////////////////////
		const char *key = &postStr[i];
		if(key == NULL) {
			Error("in post the key of the HTTP REQUEST METHOD POST.\nHTTP REQUEST METHOD POST = \"%s\"", postStr);
		}
		
		int keyLen = 0;
		for(; i < postStrLen && key[keyLen] != '='; ++keyLen) {++i;} // descobre o tamanho de key
		
		
		char *keyDecode = &postStrDecode[posInitNextStr];
		if(_Post_StrMap_Percent_Decode(keyDecode, key, keyLen) != 0) { // decode the key
			Error("decoding key in HTTP REQUEST METHOD POST.\nkey enconding: \"%s\"\nkey decode (wrong value): \"%s\"\nKey length: %d\nHTTP post: \"%s\"\n", key, keyDecode, keyLen, postStr);
		}
		posInitNextStr += strlen(keyDecode) +2; // atualiza a nova posição inicial
		
		///////////////////////////////////////////////////////////////////
		// busca, decodifica contéudo do post - um par de key e contéudo
		///////////////////////////////////////////////////////////////////
		const char *content = &postStr[++i]; // seta o início da string do content
		int contentLen = 0;
		for(; i < postStrLen && content[contentLen] != '&';
			++contentLen) { ++i; } // descobre o tamanho do content
		
		
		char *contentDecode = &postStrDecode[posInitNextStr]; // recebe a posição no array de conteúdo própria para guardar o valor
		if(_Post_StrMap_Percent_Decode(contentDecode, content, contentLen) != 0) { // decode o conteúdo do par do post
			Error("decoding content in HTTP REQUEST METHOD POST.\ncontent enconding: \"%s\"\nkey decode (wrong value): \"%s\"\ncontent length: %d\nHTTP POST: \"%s\"\n", content, contentDecode, contentLen, postStr);
		}
		posInitNextStr += strlen(contentDecode) +2; // atualiza a nova posição inicial
		
		///////////////////////////////////////////////////////////////////
		// insere no map o key e o conteúdo - atualiza a posição na array do post
		///////////////////////////////////////////////////////////////////
		map->Set(map->self, keyDecode, contentDecode);
	}
}



////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////
static int
_Post_StrMap_Percent_Decode(char* out,
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
static Post_StrMap_t
_Post_StrMap_New()
{
	Post_StrMap_t post = MM_Malloc(sizeof(Post_StrMap_o));
	if(post == NULL) {
		Error("Allocated Space for Client Input Post - String Map");
	}
	
	post->str = MM_Malloc(2048);
	if(post->str == NULL) {
		Error("Allocated Space for Client Input Post - String To Keep Post Enconding Content: key + value");
	}
	
	post->value = MM_Malloc(2048);
	if(post->value == NULL) {
		Error("Allocated Space for Client Input Post - String To Keep Post Decode Value");
	}
	
	post->size = 2048;
	
	post->map = Abstract_Factory_Common(Map);
	if(post->map == NULL) {
		Error("creating map");
	}
	
	return post;
}


static Post_StrMap_t
_Post_StrMap_Singleton()
{
	static Post_StrMap_t post = NULL;
	
	if(post == NULL)
	{
		post = _Post_StrMap_New();
		if(post == NULL)
		{
			Error("could not create singleton instance of Client Input Post.");
		}
	}
	
	return post;
}
////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
/**
 * Insere a string do http request method post in a map format
 */
bool
CWeb_ClientInput_Post_Init()
{
	Post_StrMap_t post = _Post_StrMap_Singleton();	
	post->map->Clean(post->map->self); // remove todas as chaves do map


	// verifica se é possível inicializar o post
	char *rm = getenv("REQUEST_METHOD");
	if(rm == NULL) return false;
	if(strcmp(rm, "POST") != 0) {
		return false;
	}
	char *ct = getenv("CONTENT_TYPE");
	if(ct == NULL) { 
		fprintf(stderr, "CWEB- CLIENT INPUT - POST_STRMAP - getenv(\"CONTENT_TYPE\") is NULL.\n"
			"Function: \"%s\"\nLine: %d\nFile: \"%s\"\n", __func__,__LINE__, __FILE__);
		return false;
	}
	if(strcmp(ct, "application/x-www-form-urlencoded") != 0) {
		fprintf(stderr, "CWEB- CLIENT INPUT - POST_STRMAP - getenv(\"CONTENT_TYPE\") is \"%s\".\n"
			"This library only implemented getenv(\"CONTENT_TYPE\") = \"application/x-www-form-urlencoded\"\n"
			"Function: \"%s\"\nLine: %d\nFile: \"%s\"\n",
			ct, __func__,__LINE__, __FILE__);
		return false;
	}
	
	///////////////////////////////////////////////////////////////////
	// verifica o tamanho do post, verfica o tamanho dos buffers
	///////////////////////////////////////////////////////////////////
	char *postSizeStr = getenv("CONTENT_LENGTH");
	size_t postSize = strtol(postSizeStr, NULL, 0);
	
	if(postSize >= post->size)
	{
		MM_Free(post->str);
		MM_Free(post->value);
		
		post->size = postSize + 1; // to keep character '\0'
		
		post->str = MM_Malloc(post->size);
		if(post->str == NULL) {
		Error("Allocated Space for Client Input Post - String To Keep Post Enconding Content:"
			"key + value\nPost Size String: %ld\nCONTENT_LENGTH string: \"%s\"\n"
			"CONTENT_LENGTH: %ld", post->size, postSizeStr, postSize);
	}
	
		post->value = MM_Malloc(post->size);
		if(post->value == NULL) {
		Error("Allocated Space for Client Input Post - String To Keep Post Decode Value\nPost Size String: %ld\nCONTENT_LENGTH string: \"%s\"\nCONTENT_LENGTH: %ld", post->size, postSizeStr, postSize);
		}
	}
	
	// insere o post dentro da string para realiar a leitura
	size_t read = fread(post->str, sizeof(char), postSize, stdin);
	if(read != postSize) {
		Error("reading post content and putting it into postStr.\nRead in post content: %ld\n"
			"CONTENT_LENGTH string: \"%s\"\nCONTENT_LENGTH: %ld", read, postSizeStr, postSize);
	}
	
	_Post_StrMap_Fill_Map(post->str, postSize, post->value, post->map); // decodifica e insere os values do ost dentro do map
	
	return true; // foi possível inicializar o HTTP REQUEST METHOD POST no mapa de busca
}


/**
 * 
 */
char*
ClientInput_Post_StrMap_Get(const char *post_key)
{
	if(post_key == NULL) { // check arg
		Error("the key to fetch must be not NULL.");
	}
	
	Post_StrMap_t post = _Post_StrMap_Singleton();
	
	if(post->map->HasKey(post->map->self, post_key) == true) {
		return post->map->Get(post->map->self, post_key);
	} else {
		int len = -1;
		char **key = post->map->Key(post->map->self, &len);
		fprintf(stderr, "List of all keys in HTTP REQUEST METHOD POST that be parsed.\n");
		for(int i=0; i < len; ++i) {
			fprintf(stderr, "[%d] :: \"%s\"\n", i+1, key[i]);
		}
		
		Error("fetch for a no key of HTTP REQUEST METHOD POST.\nfectch key = \"%s\"", post_key);
	}
	
	return NULL; // nunca é alcançado
}














