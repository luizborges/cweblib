/**
 *
 * @descripion: como funciona o parser para encontrar a tag <?cweb CONTENT ?>
 * '<?cweb #include "name" ?>' -> inclui o objeto chamado name, que foi colocado na função ClientOuput_Se * t, onde name é o parâmetro 'name'.
 * A tag será então substituída pelo conteúdo do objeto name, pode ser um arquivo ou uma string.
 * Após a inserão, continuará a impressão do objeto de onde parou.
 * '<?cweb@' -> imprime normalemnte o valor '<?cweb' -> ou seja, o caracter '@' será omitido, porém isto  * somente ocorre com o primeiro caracter posterior. - deve-se utilizar tal valor nos comentários, pois o * parser não distingue se a linha está em um comentário ou não.
 * @ TODO - criar uma nova versão que verifica se a linha está dentro de um comentário ou não 
 *
 * NAME parameter in function ClientOuput_Set, have limits min and max.
 * all character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-')
 */
#ifndef CLIENT_OUTPUT_FILESTREAM_H
#define CLIENT_OUTPUT_FILESTREAM_H

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

#include <headers/abstractFactoryCommon.h>
#include <headers/fileUtil.h>

#include <headers/clientOutput.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define CLIENTOUTPUT_FILESTREAM_NAME_MIN 5 // tamanho mínimo para o name - não incluindo o character '\0'
#define CLIENTOUTPUT_FILESTREAM_NAME_MAX 256 // tamanho máximo é 255 | 256 inclui o caracter '\0'



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

extern clientOutput_t ClientOutput_FileStream_New_Interface();

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
extern int ClientOutput_FileStream_Set( void *self_,
										const char *name, const void *output,
			                            const char *type, const char *opt);

extern int ClientOutput_FileStream_Print(void *self_);

extern int ClientOutput_FileStream_Print_Error(void *self_);


#ifdef __cplusplus
}
#endif
#endif // CLIENT_OUTPUT_FILESTREAM_H

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////





