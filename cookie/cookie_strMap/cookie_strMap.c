/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "cookie_strMap.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char  *str; // str cookie decode
	size_t size; // ambas as strings tem o mesmo tamanho -> str e value, always
	
	map_t map;
} Cookie_StrMap_o;


typedef Cookie_StrMap_o* Cookie_StrMap_t;

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////
static Cookie_StrMap_t _Cookie_StrMap_New();
static Cookie_StrMap_t _Cookie_StrMap_Singleton();
static void _Cookie_StrMap_Fill_Map(const char *str, const int   strLen, char *strDecode, map_t map);
static void _Cookie_Set(const char *key, const char *value, const size_t expires_sec, const char *domain, const char *path, const bool isSecure, const bool isHttpOnly, FILE *output);
////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////
static void
_Cookie_StrMap_Fill_Map(const char *str,
						const int   strLen,
						char *strDecode,
						map_t map)
{
	int posInitNextStr = 0;
	for(int i=0; i < strLen; ++i)
	{
		///////////////////////////////////////////////////////////////////
		// busca, decodifica chave do ENCODE - um par de key e conteúdo
		///////////////////////////////////////////////////////////////////
		const char *key = &str[i];
		if(key == NULL) {
			Error("in cookie the key of the HTTP COOKIE.\n"
				"HTTP COOKIE = \"%s\"", str);
		}
		
		int keyLen = 0;
		for(; i < strLen && key[keyLen] != '='; ++keyLen) {++i;} // descobre o tamanho de key
		
		
		char *keyDecode = &strDecode[posInitNextStr];
		if(CWeb_Percent_Decode(keyDecode, key, keyLen) == false) { // decode the key
			Error("decoding key in HTTP COOKIE.\nkey enconding: \"%s\"\n"
				"key decode (wrong value): \"%s\"\nKey length: %d\n"
				"HTTP COOKIE: \"%s\"\n", key, keyDecode, keyLen, str);
		}
		posInitNextStr += strlen(keyDecode) +2; // atualiza a nova posição inicial
		
		///////////////////////////////////////////////////////////////////
		// busca, decodifica contéudo da STR ENCODE - um par de key e contéudo
		///////////////////////////////////////////////////////////////////
		const char *content = &str[++i]; // seta o início da string do content
		int contentLen = 0;
		for(; i < strLen && content[contentLen] != ';';
			++contentLen) { ++i; } // descobre o tamanho do content
		
		
		char *contentDecode = &strDecode[posInitNextStr]; // recebe a posição no array de conteúdo própria para guardar o valor
		if(CWeb_Percent_Decode(contentDecode, content, contentLen) == false) { // decode o conteúdo do par do get
			Error("decoding content in HTTP COOKIE.\ncontent enconding: \"%s\"\nkey decode (wrong value): \"%s\"\ncontent length: %d\nHTTP COOKIE: \"%s\"\n", content, contentDecode, contentLen, str);
		}
		posInitNextStr += strlen(contentDecode) +2; // atualiza a nova posição inicial
		// atualiza a posição - final do conteúdo é "; "
		// é necessário para buscar o próximo key se houver
		++i;
		
		///////////////////////////////////////////////////////////////////
		// insere no map o key e o conteúdo - atualiza a posição na array do get
		///////////////////////////////////////////////////////////////////
		map->Set(map->self, keyDecode, contentDecode);
	}
}

static void
_Cookie_Set(const char *key, const char *value,
			const size_t expires_sec,
			const char *domain, const char *path,
			const bool isSecure, const bool isHttpOnly,
			FILE *output)
{
	///////////////////////////////////////////////////////////////////
	// check the args
	///////////////////////////////////////////////////////////////////
	if(key == NULL) {
		Error("Cookie Set Key is NULL.");
	}
	if(strlen(key) < 1) {
		Error("Cookie Set Key is a empty string.");
	}
	
	if(value == NULL) {
		Error("Cookie Set Value is NULL.\n key is \"%s\"", key);
	}
	if(strlen(value) < 1) {
		Error("Cookie Set Value is a empty string.");
	}
	
	
	if(expires_sec < 0) {
		Error("Cookie Set Expires seconds cannot be less than zero.\n"
			"Uses 0 to not set expires time to cookie.\n"
			"Expires seconds passed is %d", expires_sec);
	}
	
	///////////////////////////////////////////////////////////////////
	// cookie set -
	///////////////////////////////////////////////////////////////////
	fprintf(output, "Set-Cookie: %s=%s",
		key, CWeb_Percent_Encode(value, strlen(value)));
		
	///////////////////////////////////////////////////////////////////
	// cookie set - time
	///////////////////////////////////////////////////////////////////
	if(expires_sec > 0)
	{
		time_t expires = time(NULL) + expires_sec;
		struct tm *localTimeExp = localtime(&expires);
		if(localTimeExp == NULL) {
			Error("Local Time is NULL.\n returned from localtime() C function.");
		}
		
		char *st = asctime(localTimeExp);
		if(st == NULL) {
			Error("String of Local Time is NULL.\nreturned from asctime() C function.");
		}
		fprintf(output, "; Expires=");
		// segue o padrão RFC 7231, section 7.1.1.2: Date 	Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content - https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
		// Date: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
		// Date: Wed, 21 Oct 2015 07:28:00 GMT 
		// DDD MMM dd hh:mm:ss YYYY - C time string
		fprintf(output, "%c%c%c, %c%c %c%c%c %c%c%c%c %c%c:%c%c:%c%c GMT",
			st[0], st[1], st[2], // week day
			st[8], st[9], // day
			st[4], st[5], st[6], // month
			st[20], st[21], st[22], st[23], // year
			st[11], st[12], // hour
			st[14], st[15], // minute
			st[17], st[18]); // second
	}
	
	///////////////////////////////////////////////////////////////////
	// seta os outros atributos
	///////////////////////////////////////////////////////////////////
	if(domain != NULL)
	{
		if(strlen(domain) < 1) {
			Error("Cookie Set Domain cannot be a empty string.");
		}
		
		fprintf(output, "; Domain=%s", domain);
	}
	
	if(path != NULL)
	{
		if(strlen(path) < 1) {
			Error("Cookie Set Path cannot be a empty string.");
		}
		
		fprintf(output, "; Path=%s", path);
	}
	
	if(isSecure == true)
	{
		fprintf(output, "; Secure");
	}
	
	if(isHttpOnly == true)
	{
		fprintf(output, "; HttpOnly");
	}
	
	fprintf(output, "\r\n"); // para indicar o fim do cookie
}

////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////
static Cookie_StrMap_t
_Cookie_StrMap_New()
{
	Cookie_StrMap_t cookie = MM_Malloc(sizeof(Cookie_StrMap_o));
	if(cookie == NULL) {
		Error("Allocated Space for Cookie - String Map");
	}
	
	cookie->str = MM_Malloc(2048);
	if(cookie->str == NULL) {
		Error("Allocated Space for Cookie - String Default");
	}
	
	cookie->size = 2048;
	
	cookie->map = Abstract_Factory_Common(Map);
	if(cookie->map == NULL) {
		Error("creating map");
	}
	
	return cookie;
}


static Cookie_StrMap_t
_Cookie_StrMap_Singleton()
{
	static Cookie_StrMap_t cookie = NULL;
	
	if(cookie == NULL)
	{
		cookie = _Cookie_StrMap_New();
		if(cookie == NULL)
		{
			Error("could not create singleton instance of Cookie String Map.");
		}
	}
	
	return cookie;
}


////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
/**
 * Insere a string do http cookie in a map format
 */
bool
CWeb_Cookie_Init()
{
	Cookie_StrMap_t cookie = _Cookie_StrMap_Singleton();
	cookie->map->Clean(cookie->map->self); // remove todas as chaves do map
	
	char *cookieStr = getenv("HTTP_COOKIE");
	if(cookieStr == NULL) {
//		MError("NO COOKIE");
		return false;
	}
	
	// verifica se existe espaco para guardar o cookie
	size_t cookieStrLen = strlen(cookieStr);
	if(cookie->size <= cookieStrLen)
	{
		cookie->size = cookieStrLen +1; // for character `\0`
		MM_Free(cookie->str);
		
		cookie->str = MM_Malloc(cookie->size);
		if(cookie->str == NULL) {
			Error("Allocated Space for Cookie - String Default");
		}
	}
//	MError("http-cookie init::\n\"%s\"", cookieStr);
	_Cookie_StrMap_Fill_Map(cookieStr, cookieStrLen, cookie->str, cookie->map);
	
	return true;
}


char*
CWeb_Cookie_Get(const char *key,
				bool *hasKey)
{
	if(key == NULL) { // check arg
		Error("the key to fetch must be not NULL.");
	}
	
	Cookie_StrMap_t cookie = _Cookie_StrMap_Singleton();
	
	if(cookie->map->HasKey(cookie->map->self, key) == true) {
		if(hasKey != NULL) {
			*hasKey = true;
		}
		return cookie->map->Get(cookie->map->self, key);
	} else {
		int len = -1;
		char **_key = cookie->map->Key(cookie->map->self, &len);
		MError("List of all keys in HTTP COOKIE that be parsed:");
		for(int i=0; i < len; ++i) {
			fprintf(stderr, "[%d] :: \"%s\"\n", i+1, _key[i]);
		}
		
		MError("fetch for a no key of HTTP COOKIE.\nfectch key = \"%s\"", key);
		
		if(hasKey != NULL) { // set hasKey
			*hasKey = false;
		}
	}
	
	return NULL; // nunca é alcançado
}



char*
CWeb_Cookie_Set(const char *key, const char *value,
				const size_t expires_sec,
				const char *domain, const char *path,
				const bool isSecure, const bool isHttpOnly)
{
	///////////////////////////////////////////////////////////////////
	// Create temp file to keep string
	///////////////////////////////////////////////////////////////////
	FILE *tmp = tmpfile();
	if(tmp == NULL) {
		Error("Cookie Set Cannot Create a temporary file.");
	}
	
	///////////////////////////////////////////////////////////////////
	// create cookie
	///////////////////////////////////////////////////////////////////
	_Cookie_Set(key, value, expires_sec, domain, 
				path, isSecure, isHttpOnly, tmp);
	
	///////////////////////////////////////////////////////////////////
	// return the string
	///////////////////////////////////////////////////////////////////
	rewind(tmp);
	char *cookie = FileUtil_StrMap(tmp, "file"); // já fecha o arquivo
	return cookie;
}

void 
CWeb_Cookie_Print(const char *key, const char *value,
					const size_t expires_sec,
					const char *domain, const char *path,
					const bool isSecure, const bool isHttpOnly)
{
	_Cookie_Set(key, value, expires_sec, domain, 
				path, isSecure, isHttpOnly, stdout);

}









