/**
 *
 * @descripion: implementa 3 macros para tornar mais legível a escolha do caminho a se fazer.
 * Deve-se de preferência criar uma função exclusiva para gerenciar as routes da aplicação.
 * As macros devem ser utilizadas em ordem e em sequência, para não ocasionar errors, segue a ordem:
 * CWeb_Route_Init
 * CWeb_Route (quantas vezes for necessário para especificar todos os caminhos)
 * CWeb_Route_Error
 *
 *@IMPORTANT: após o nome do uso da maro, não pode usar o character ';' - erro de compilação, o certo é:
 *@exemple: exemplo de uma função que usa esta biblioteca:
 * void route()
 *{
 *	CWeb_Route_Init("/", page_index())
 *	CWeb_Route("/init", page_init())
 *	CWeb_Route_Error(page_error())
 *}
 */
#ifndef CWEB_ROUTE_INTEGRATED_H
#define CWEB_ROUTE_INTEGRATED_H

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
#include<stdarg.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

/**
 * os parâmetros são os mesmos.
 * @PARAM PATH = string que contém o caminho requisitado, ou seja, deve ser igual ao
 * retornado pela variável PATH_INFO que é preenchida pelo HTTP.
 * @PARAM FUNC = função que deve ser chamada, caso o endereço seja requisitado.
 * para utilizar mais de um código, apenas usar o parênteses ex: page_index(); page_index_2()
 */
#define CWeb_Route_Init(PATH, FUNC) \
	CWeb_ClientInput_Init();\
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
#endif // CWEB_ROUTE_INTEGRATED_H

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////





