/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "clientInput_manager.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////
static char *(*clientInput_get) (const char*);
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
bool
CWeb_ClientInput_Init()
{
	char *rm = getenv("REQUEST_METHOD");
	if(rm == NULL) return false;
	
	// descobre qual o REQUEST_METHOD passado
	if(strcmp(rm, "GET") == 0) {
		clientInput_get = ClientInput_Get_StrMap_Get;
		return CWeb_ClientInput_Get_Init();
	}
	else if(strcmp(rm, "POST") == 0) {
		clientInput_get = ClientInput_Post_StrMap_Get;
		return CWeb_ClientInput_Post_Init();
	}
	else {
		Error("REQUEST_METHOD do not recognize.\nREQUEST_METHOD = \"%s\"", rm);
	}
	
	return false;
}

char*
CWeb_ClientInput(const char *key)
{
	return clientInput_get(key);
}










