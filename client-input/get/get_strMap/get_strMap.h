/**
 *
 * @descripion: faz um parser dos valores passados ao servidor por meio do HTTP REQUEST METHOD GET
 * a string, é quebrada em valor e conteúdo, e inserida em um map, para facilitar a busca dos
 * valores pelo usuários.
 * @warning: OS VALORES SÃO APENAS PARA LEITURA, NUNCA MODIFIQUE OS VALORES PASSADOS.
 * PARA TANTO, COPIE O VALOR PARA OUTRA STRING.
 */
#ifndef CLIENTINPUT_GET_STRMAP_H
#define CLIENTINPUT_GET_STRMAP_H

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
// Interface
////////////////////////////////////////////////////////////////////////////////
extern bool CWeb_ClientInput_Get_Init();

extern char* ClientInput_Get_StrMap_Get(const char *get_key);

////////////////////////////////////////////////////////////////////////////////
// Functions - private - to debug
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif // CLIENTINPUT_GET_STRMAP_H

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////





