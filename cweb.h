/**
 *
 * @descripion: funciona como um header que contém todas as bibliotecas, é necessário apenas adicionar
 * essa biblioteca para ter acesso a todas a biblioteca.
 */
#ifndef CWEB_H
#define CWEB_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#if defined(unix) || defined(__unix) || defined(__unix__) || (defined (__APPLE__) && defined (__MACH__)) // Unix (Linux, *BSD, Mac OS X)
#include <sys/types.h>
#include <dirent.h>
#endif



#include <headers/abstractFactoryCommon.h>
#include <headers/fileUtil.h>
#include <headers/stackTracer.h>


////////////////////////////////////////////////////////////////////////////////
// Includes - Client Output
////////////////////////////////////////////////////////////////////////////////
//#include "client-output/clientOutput_strMap/clientOutput_strMap.h"
//#include <headers/clientOutput_strMap.h>
/*
 * @descripion: como funciona o parser para encontrar a tag <?cweb CONTENT ?>
 * '<?cweb #in "name" ?>' -> inclui o objeto chamado name, que foi colocado na função ClientOuput_Se * t, onde name é o parâmetro 'name'.
 * A tag será então substituída pelo conteúdo do objeto name, pode ser um arquivo ou uma string.
 * Após a inserão, continuará a impressão do objeto de onde parou.
 * '<?cweb@' -> imprime normalemnte o valor '<?cweb' -> ou seja, o caracter '@' será omitido, porém isto  * somente ocorre com o primeiro caracter posterior. - deve-se utilizar tal valor nos comentários, pois o * parser não distingue se a linha está em um comentário ou não.
 * @ TODO - criar uma nova versão que verifica se a linha está dentro de um comentário ou não *
 * NAME parameter in function ClientOuput_Set, have limits min and max - see below.
 * all character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-')
 * O nome da tag inserida no CWeb_Out_Set -> 'name', não pode ser "___tag_add___XXX"
 */
 ////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define CWEB_CLIENTOUTPUT_STRMAP_NAME_MIN 5 // tamanho mínimo para o name - não incluindo o character '\0'
#define CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX 256 // tamanho máximo é 255 | 256 inclui o caracter '\0'

extern int
CWeb_Out_Set(const char *name, const void *output,
			 const char *type, const char *opt);
			 
extern int CWeb_Out_Print();

extern int CWeb_Out_Print_Error();


////////////////////////////////////////////////////////////////////////////////
// Includes - Client Input
////////////////////////////////////////////////////////////////////////////////
//#include "client-input/clientInput_manager/clientInput_manager.h"
//#include <headers/clientInput_manager.h>
//#include <headers/get_strMap.h> // trata o METHOD GET
//#include <headers/post_strMap.h> // trata o METHOD POST

// methods generics - the user only use these methods
bool CWeb_In_Init();
char *CWeb_In(const char *key);

// methods get - to implement interface above
extern bool CWeb_ClientInput_Get_Init();
extern char* ClientInput_Get_StrMap_Get(const char *get_key);

// methods post - to implement interface above
extern bool CWeb_ClientInput_Post_Init();
extern char* ClientInput_Post_StrMap_Get(const char *get_key);

////////////////////////////////////////////////////////////////////////////////
// Includes - Route
////////////////////////////////////////////////////////////////////////////////
//#include "route/route_easy/route_easy.h"
//#include <headers/route_easy.h>
//#include <headers/route_integrated.h>
// route integrade 
/**
 * os parâmetros são os mesmos.
 * @PARAM PATH = string que contém o caminho requisitado, ou seja, deve ser igual ao
 * retornado pela variável PATH_INFO que é preenchida pelo HTTP.
 * @PARAM FUNC = função que deve ser chamada, caso o endereço seja requisitado.
 * para utilizar mais de um código, apenas usar o parênteses ex: page_index(); page_index_2()
 */
#define CWeb_Route_Init(PATH, FUNC) \
	CWeb_In_Init();\
	CWeb_Cookie_Init();\
	char *cweb_route_path = getenv("PATH_INFO"); \
	if(cweb_route_path == NULL) {\
		FUNC;\
	} else if(strcmp(cweb_route_path, PATH) == 0) {\
		FUNC;\
	}

#define CWeb_Route(PATH, FUNC) \
	else if(strcmp(cweb_route_path, PATH) == 0) {\
		FUNC;\
	}

#define CWeb_Route_Error(FUNC) \
	else {\
		FUNC;\
	}

////////////////////////////////////////////////////////////////////////////////
// Includes - Cookies
////////////////////////////////////////////////////////////////////////////////
//#include "cookie/cookie_strMap/cookie_strMap.h"
//#include <headers/cookie_strMap.h>
extern bool CWeb_Cookie_Init();

/**
 * Retorna o valor do cookie.
 * @arg hasKey: se o cookie existe, pode não ter valor associado, apenas precisa existir
 * o valor é true. false caso o cookie não exista.
 * @return o valor do cookie ou retorna NULL se não existe o cookie.
 */
extern char *CWeb_Cookie_Get(const char *key,
							bool *hasKey);


/**
 * Retorna uma string contendo todo o conteúdo do cookie.
 * os valores key e value, não podem ser NULL e também não podem ser uma
 * string vazia.
 * Insert the string: " GMT" ao final do tempo designado.
 * O tempo é recuperado com a função localtime(time(NULL) + expires_sec)
 * Se os argumentos "domain" e "path" são NULLs, eles não são inseridos.
 * se os argumentos "isSecure" e "isHttpOnly" são false, eles não são inseridos.
 * Ao final da string do cookie é incluído o charactere '\n'
 * OBS: se o valor do expires_sec for -1, o tempo será setado para
 * "expires=Thu, 01 Jan 1970 00:00:00 GMT" este valor é usado para expirar o cookie
 */
extern char *CWeb_Cookie_Set(const char *key, const char *value,
								const long expires_sec,
								const char *domain, const char *path,
								const bool isSecure, const bool isHttpOnly);

/**
 * A saída é gravada pela função fprintf(stdout, ... );
 * A gravação é feita por partes, ou seja, não é criada toda a string e então
 * inserida no stdout.
 * os valores key e value, não podem ser NULL e também não podem ser uma
 * string vazia.
 * Insert the string: " GMT" ao final do tempo designado.
 * O tempo é recuperado com a função localtime(time(NULL) + expires_sec)
 * Se os argumentos "domain" e "path" são NULLs, eles não são inseridos.
 * se os argumentos "isSecure" e "isHttpOnly" são false, eles não são inseridos.
 * Ao final da string do cookie é incluído o charactere '\n'
 * OBS: se o valor do expires_sec for -1, o tempo será setado para
 * "expires=Thu, 01 Jan 1970 00:00:00 GMT" este valor é usado para expirar o cookie
 */
extern void CWeb_Cookie_Print(const char *key, const char *value,
								const long expires_sec,
								const char *domain, const char *path,
								const bool isSecure, const bool isHttpOnly);

////////////////////////////////////////////////////////////////////////////////
// Includes - Url Percent Encode/Decode
////////////////////////////////////////////////////////////////////////////////
//#include "percent/percent.h"
//#include <headers/percent.h>

extern bool
CWeb_Percent_Decode(char* out,
                    const char* in,
                    const int maxDecode);

/**
 * substituiu o character ' ' pelo character '+'.
 * @return retorna a string codificada em caso de acerto.
 * em caso de erro, retorna NULL.
 */
extern char*
CWeb_Percent_Encode(const char* in,
                    const int maxEncode);

////////////////////////////////////////////////////////////////////////////////
// Includes - Session
////////////////////////////////////////////////////////////////////////////////
//#include <headers/session_fileMap.h>
/**
 * @DirFileSession: diretório em que será criado o arquivo de Sessão.
 * @lifeSession: tempo que uma sessão dura - tempo em segundos.
 * @del: tempo (em segundos) para se deletar, dentro do diretório, todos os aruqivos de
 * sessão. A primeiro coisa que ele faz, é entrar dentro do diretório, e deletar
 * todos os arquivos de sessão que estão dentro do diretório que foram criados
 * a mais de 'del' segundos, onde del é esta variável.
 * 
 */
extern void*
CWeb_Session_Init(const char *DirFileSession,
				  const time_t lifeSession,
				  const time_t del);

extern bool
CWeb_Session_Load(const char *sid);

extern void*
CWeb_Session_Get(const char *key,
				size_t *size);

extern void
CWeb_Session_Set(const char *key,
				 const void *value,
				 const size_t size);

extern void*
CWeb_Session_Del(const char *key,
				size_t *size);

/**
 * Retorna o valor do cookie sid.
 */
extern char*
CWeb_Session_Save();

/**
 * Retorana todas as chave.
 * @arg numKey: tamanho do array char** retornado. representa o número de chaves.
 */
extern char**
CWeb_Session_Key(int *numKey);

/**
 * Realiza a operação "CWeb_Session_Del" para todas as key.
 * @return: o número de chaves deletadas.
 */
extern int
CWeb_Session_Clean();

extern void
CWeb_Session_End();
////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Enum
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Constructs
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions - private - to debug
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif // CWEB_H

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////





