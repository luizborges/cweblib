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
#include <headers/clientInput_manager.h>

////////////////////////////////////////////////////////////////////////////////
// Includes - Route
////////////////////////////////////////////////////////////////////////////////
//#include "route/route_easy/route_easy.h"
//#include <headers/route_easy.h>
#include <headers/route_integrated.h>
////////////////////////////////////////////////////////////////////////////////
// Includes - Cookies
////////////////////////////////////////////////////////////////////////////////
//#include "cookie/cookie_strMap/cookie_strMap.h"
#include <headers/cookie_strMap.h>

////////////////////////////////////////////////////////////////////////////////
// Includes - Url Percent Encode/Decode
////////////////////////////////////////////////////////////////////////////////
//#include "percent/percent.h"
#include <headers/percent.h>

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





