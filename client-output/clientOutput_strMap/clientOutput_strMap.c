/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
//#include "clientOutput_strMap.h"
#include "../../cweb.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////

// para tratamento interno da variável type
enum Out_Type_t {
	string,
	file,
	file_name
};

// para tratamento interno da variável opt
enum Out_Opt_t {
	Root, // NÃO pode ser error - não é chamado pela tag <?cweb #include "NAME" ?>
	Leaf, // NÃO pode ser error - só é chamado pela tag <?cweb #include "NAME" ?>
	Error_Root, // só pode ser error - não é chamado pela tag <?cweb #include "NAME" ?>
	Error_Leaf // só pode ser error - só é chamado pela tag <?cweb #include "NAME" ?>
};


union Out_Var_o {
	FILE *file;
	char *file_name;
	char *str;
};


typedef struct {
	union Out_Var_o var;
	enum  Out_Type_t type;
	enum  Out_Opt_t opt;

	bool  hasPrinted;
	char  name[CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX]; // key do map para esta saída
} Out_o;

typedef Out_o* Out_t;



////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////
static Out_t
_Get_Root_COutput_To_Print (const bool isError);


static void
_Parser(Out_t cout);

static char*
_Parser_Tag(Out_t cout, char *tag);
										
////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////
static inline char*
_Get_New_Name_Tag_ADD()
{
	static int num = 0;
	
	char prefix[14] = "___tag_add___";
	
	char *name = MM_Malloc((strlen(prefix) + sizeof(int) +1)*sizeof(char));
	if(name == NULL) {
		Error("Allocating space for new name of tag add\n"
		"erro is %d\nstr erro is \"%s\"", errno, strerror(errno));
	}
	
	sprintf(name, "%s%d", prefix, num);
	++num; // update num for the next name
	
	return name;
}


static inline bool
_Has_Only_Alpha_Num(const char *str)
{
	for(int i=0; i < strlen(str); ++i) {
		if(isalnum(str[i]) == false &&
			str[i] != '_' &&
			str[i] != '-')
		{
				 return false;
		}
	}
	
	return true;
}


/**
 * Map com todos os "arquivos" que foram inseridos pelo usuário.
 * Guarda as saídas que serão enviadas para o cliente.
 * a chave do map, é a variável 'name'
 */
static inline map_t
_Get_LeafMap () 
{
	static map_t map = NULL;
	
	if(map == NULL) { // inicializa o map, se isto ainda não ocorreu
		map = Abstract_Factory_Common(Map);
		if(map == NULL) {
			Error("creating map");
		}
	}
	
	return map;
}

/**
 * Map com todos os "arquivos" que foram inseridos pelo usuário.
 * Guarda as saídas que serão enviadas para o cliente.
 * a chave do map, é a variável 'name'
 */
static inline map_t
_Get_LeafMap_Error () 
{
	static map_t mapError = NULL;
	
	if(mapError == NULL) { // inicializa o map, se isto ainda não ocorreu
		mapError = Abstract_Factory_Common(Map);
		if(mapError == NULL) {
			Error("creating map error");
		}
	}
	
	return mapError;
}



static inline void
_Check(const char *name, const void *output,
				    const char *type, const char *opt)
{
	// check name 
	if(name == NULL) { Error("name is NULL"); }
	
	if(strlen(name) < CWEB_CLIENTOUTPUT_STRMAP_NAME_MIN) { 
		Error("name length is lesser than %d.\nname length: %d", CWEB_CLIENTOUTPUT_STRMAP_NAME_MIN, strlen(name));
	}
	
	if(strlen(name) >= CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX) {
		Error("name length is greater than %d.\nname lenght: %d.",
			CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX -1, strlen(name));
	}
	
	if(_Has_Only_Alpha_Num(name) == false) {
		Error("not a valid name.\nall character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-').\nname: \"%s\"", name);
	}
	
	// verifica se já existe um nome igual no map
	map_t map = _Get_LeafMap();
	if(map->HasKey(map->self, name) == true) { Error("name already exists in ClientOutput Set.\n Name already inserted before.\nName: \"%s\"", name);}

	if(output == NULL) { Error("output is NULL"); } // check 

	if(strcmp(type, "file") != 0      &&
	   strcmp(type, "file_name") != 0 &&
	   strcmp(type, "string") != 0)
	{
		Error("type is not a valid type.type is \"%s\"", type);   
	}
	
	// check the name of the file
	if(strcmp(type, "file_name") == 0) {
		char *fn = (char*)output;
		
		if(strlen(fn) == 0) {
			Error("file name is a empty string.\nValid values for type is: \"string\" or \"file\" or \"file_name\".\nfor more details see documentation.", opt);
		}
	}

	if( strcmp(opt, "root") != 0 &&
		strcmp(opt, "leaf") != 0 &&
		strcmp(opt, "error_root") != 0 &&
		strcmp(opt, "error_leaf") != 0)
	{
		Error("opt is not a valid option.opt is \"%s\".\nValid values for opt is: \"root\" or \"leaf\" or \"error_root\" or \"error_leaf\".\nfor more details see documentation.", opt);   
	}
}


/**
 * Lista que contém todos os nós que são root - que podem ser roots
 */
static inline arrayList_t
_Get_RootList ()
{
	static arrayList_t rootList = NULL;
	
	if(rootList == NULL) { // inicializa o array, se isto ainda não ocorreu
		rootList = Abstract_Factory_Common(ArrayList);
		if(rootList == NULL) {
			Error("Creating Root List");
		}
	}
	
	return rootList;
}

/**
 * Lista que contém todos os nós que são root - que podem ser roots
 */
static inline arrayList_t
_Get_RootList_Error ()
{
	static arrayList_t rootListError = NULL;
	
	if(rootListError == NULL) { // inicializa o array, se isto ainda não ocorreu
		rootListError = Abstract_Factory_Common(ArrayList);
		if(rootListError == NULL) {
			Error("Creating Root List");
		}
	}
	
	return rootListError;
}

/**
 * Fila que contém as strings que serão impressas
 * Queue strcut.
 * First in - First out
 * Embora a base seja um array, o modus operandi é de fila
 */
static inline arrayList_t
_Get_PrintQueue ()
{
	static arrayList_t printQueue = NULL;
	
	if(printQueue == NULL) { // inicializa o array, se isto ainda não ocorreu
		printQueue = Abstract_Factory_Common(ArrayList);
		if(printQueue == NULL) {
			Error("Creating Print List");
		}
	}
	
	return printQueue;
}

static inline void
_Parser_Inside_Tag_Check_Name(const Out_t cout,
										   const char *name)
{
	if(strlen(name) >= CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX) { // verifica se o nome é maior que o limite permitido
		Error("not a valid name.\nname is too long, it must to be equal or lesser than %d characters.\nin tag <?cweb #include \"\ngiven name: \"%s\"\n"
			"max length to name: %d.\nClient Output Name: \"%s\".",
			CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX, name, CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX, cout->name);
	}
	
	if(strlen(name) < CWEB_CLIENTOUTPUT_STRMAP_NAME_MIN) { // verifica se o nome é menor que o limite permitido
		Error("not a valid name.\nname is too short, it must to be equal or greater than %d characters.\nin tag <?cweb #include \"\ngiven name: \"%s\"\n"
			"min length to name: %d.\nClient Output Name: \"%s\".\nLine Number where is problem: %d.",
			CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX, name, CWEB_CLIENTOUTPUT_STRMAP_NAME_MAX, cout->name);
	}
	
	if(_Has_Only_Alpha_Num(name) == false) { // verifica se é apenas dígito ou alfa
		Error("not a valid name.\nall character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-').\nin tag <?cweb #include \"\ngiven name: \"%s\"\nClient Output Name: \"%s\".", name, cout->name);
	}
}

static inline void
_Parser_Inside_Tag_Run_COutChild(Out_t cout,
											   const char *childName)
{
	map_t map = NULL;
	if(cout->opt == Error_Root ||
	   cout->opt == Error_Leaf)
	{
		map = _Get_LeafMap_Error();
	}
	else
	{
		map = _Get_LeafMap();
	}
	
	if(map == NULL) {
		Error("map is NULL. map = %p", map);
	}
	char *optStr[] = { "root", "leaf", "error_root", "error_leaf" };
	char *opt; // utilizado para imprimir a opção em caso de erro
	if(map->HasKey(map->self, childName) == true) {
		Out_t child = map->Del(map->self, childName);
		if(child == NULL) { // verifica se ocorreu algum erro
			Error("leaf is NULL.\nname = \"%s\".\nClient Output Name: \"%s\".",
				childName, cout->name);
		}
		if(child->opt != Leaf &&
		   child->opt != Error_Leaf) {
		   	opt = child->opt == Root ? optStr[0] : optStr[2];
			Error("child is not leaf option.\noption: %s\nname = \"%s\".\nClient Output Name: \"%s\"",
				opt, childName, cout->name);
		}
		if(child->opt == Leaf &&
		   cout->opt != Root &&
		   cout->opt != Leaf) {
		    opt = cout->opt == Error_Root ? optStr[2] : optStr[3];
			Error("child is Leaf but its parent not.\nParent opt is %s", opt);
		}
		if(child->opt == Error_Leaf &&
		   cout->opt != Error_Root &&
		   cout->opt != Error_Leaf) {
		    opt = cout->opt == Root ? optStr[0] : optStr[1];
		    Error("child is Error_Leaf but its parent not.\nParent opt is %s", cout->opt);
		}
		if(child->hasPrinted == true) {
			Error("leaf already printed.\nname = \"%s\".\nClient Output Name: \"%s\".",
				childName, cout->name);
		}
		
		_Parser(child); // inicia a recursão - para o pai e chama o filho
	}
	else // não tem o name no leaf map
	{
		int numKeys = -1;
		if(cout->opt == Root || cout->opt == Leaf) { opt = optStr[1]; }
		else { opt = optStr[3]; } // utiliza para imprimir o nome do map que deveria estar child
		char **key = map->Key(map->self, &numKeys);
		fprintf(stderr, "*********\nAll keys in %sleaf map\n**********\nnumber of keys: %d\n",
				opt, numKeys);
		for(int i=0; i < numKeys; ++i) { // imprime todas as chaves do leaf key
			fprintf(stderr,"key[%d]: \"%s\"\n", i, key[i]);
		}
		Error("No name as key in %s map.\nName could be not set or be already printed.\n"
			"name = \"%s\".\nClient Output Name: \"%s\".", opt, childName, cout->name);
	}
}

static inline void
_Parser_Main_Loop(Out_t cout,
								char *cout_data_)
{
	char *cout_data = cout_data_; // guarda essa posição para fins de debug
	while(strlen(cout_data) > 0)
	{
		/////////////////////////////////////////////////////////////////
		// guarda a string que será impressa e realiza a busca pela tag
		/////////////////////////////////////////////////////////////////			
		arrayList_t printQueue = _Get_PrintQueue ();
		printQueue->Add(printQueue->self, printQueue->Len(printQueue->self), cout_data); // guarda a string que será impressa
		char *tag = strstr(cout_data, "<?cweb"); // realiza a busca pela tag
		if(tag == NULL) { // termina o loop pois não há tag
			break;
		}
		
		/////////////////////////////////////////////////////////////////
		// verifica se não é comentário e se não for, chama o parser da tag
		/////////////////////////////////////////////////////////////////
		if(tag[6] == '@') // verifica se é o character de escape
		{	// o character de escape é sempre omitido
			tag[6] = '\0'; // finaliza a string que será impressa
			cout_data = &tag[7]; // atualiza o ponteiro, para continuar a verificação da string
		} else {
			cout_data = _Parser_Tag(cout, tag);
		}
	}
}

static inline void
_StrMap_Print()
{
	arrayList_t printQueue = _Get_PrintQueue();
	if(printQueue->Len(printQueue->self) < 0) {
		Error("NO Client Output to print.");
	}
	
	// imprime todas as strings armazenadas na fila de impressão.
	while(printQueue->Len(printQueue->self) > 0)
	{
		char *str = printQueue->Del(printQueue->self, 0);
		printf("%s", str);
	}
}

static inline void
_Check_Print_All_ClientOutput(const bool isError)
{
	map_t map = NULL;
	if(isError == true) {
		map = _Get_LeafMap_Error();
	}
	else
	{
		map = _Get_LeafMap();
	}
	
	int numKey;
	char **cout = map->Key(map->self, &numKey);
	
	if(numKey > 0) // check if any printable cout is not printed
	{	// se tem algum cout não impresso - exibe eles e lança um erro
		fprintf(stderr, "***********************************\n"
			"not all Client Output was printed.\nClient output name not printed listed below:\n");
		
		for(int i=0; i < numKey; ++i)
		{
			fprintf(stderr, "[%d]::Client Output Name::\"%s\"\n", i+1, cout[i]);
			map->Del(map->self, cout[i]);
		}
		
		Error("\n");
	}
}

////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////
static char*
_Parser_Tag(Out_t cout,
			char *tag)
{
	/////////////////////////////////////////////////////////////////
	// check the header of tag "<?cweb #in "" or <?cweb #add "
	/////////////////////////////////////////////////////////////////
	if(strncmp(tag, "<?cweb #in \"", 12) != 0 &&
		strncmp(tag, "<?cweb #add \"", 13) != 0)
	{
		char *endLine = strchr(tag, '\n'); // descobre o fim da linha onde ocorreu o erro
		if(endLine != NULL) {
			endLine[0] = '\0'; // posiciona um novo fim na string para impressão do erro.
		}
		
		Error("tag <?cweb is incorrect.\nTag found (until the end of line): \"%s\"\n"
			"Expected one of these sequences: \"<?cweb #include \"\" (with \' \') or"
			" \"<?cweb\n#add \"\" (with \' \') \nfor more details see documentation.\n"
			"Client Output Name: \"%s\"", tag, cout->name);
	}
	
	bool isTagADD = false;
	if(strncmp(tag, "<?cweb #add \"", 12) == 0) {
		isTagADD = true;
	}
	/////////////////////////////////////////////////////////////////
	// get the name in the include
	/////////////////////////////////////////////////////////////////
	char *name = strchr(tag, '\"'); // sempre terá o character '"' - devido a verificação anterior
	name = &name[1]; // atualiza o nome para a próxima possição do arranjo - sempre haverá tal posição - garantida pelo character '\0'
	char *nameEnd = strchr(name, '\"'); // descobre o fim de name
	if(nameEnd == NULL) {
		char *endLine = strchr(name, '\n'); // descobre o fim da linha onde ocorreu o erro
		if(endLine != NULL) {
			endLine[0] = '\0'; // posiciona um novo fim na string para impressão do erro.
		}
		Error("tag <?cweb is incorrect.\nNAME has always be terminated with character \'\"\'.\nNAME given is = \"%s\".\ntag struct is \"<?cweb #include \"NAME\" ?>\" or \"<?cweb #add \"NAME\" ?>\".\nfor more details see documentation.\nClient Output Name: \"%s\"", name, cout->name);
	}
	nameEnd[0] = '\0'; // seta o fim de name, onde era o character '"'
	
	
	if(isTagADD == true) // run tag <?cweb #add
	{
		// trata se for a tag <cweb #add \"
		/**
		 * para fins de deixar o código mais simples e harmonico.
		 * Ele le o nome do arquivo e adiciona o mesmo como sendo uma tag #in
		 * Em seguida manda executar a tag.
		*/
		char *tagAddName = _Get_New_Name_Tag_ADD();
		CWeb_Out_Set(tagAddName, name, "file_name", "leaf");
		_Parser_Inside_Tag_Check_Name(cout, tagAddName);
		_Parser_Inside_Tag_Run_COutChild(cout, tagAddName); // run cout child leaf - new cout
		MM_Free(tagAddName); // free memory
	} else { // run tag <?cweb #in
		_Parser_Inside_Tag_Check_Name(cout, name);
		_Parser_Inside_Tag_Run_COutChild(cout, name); // run cout child leaf - new cout run
	}
	
	
	/////////////////////////////////////////////////////////////////
	// get the new cout that name is called
	/////////////////////////////////////////////////////////////////
	
	
	/////////////////////////////////////////////////////////////////
	// treat the end of tag after NAME
	/////////////////////////////////////////////////////////////////
	char *tagEnd = nameEnd; // descobre o começo depois do fim do nome
	tagEnd = &tagEnd[1]; // seta o primeiro character depois de '"' - após o fim do nome
	
	if( strncmp(tagEnd, " ?>",    3) != 0)
	{
		char *endLine = strchr(tagEnd, '\n'); // descobre o fim da linha onde ocorreu o erro
		if(endLine != NULL) {
			endLine[0] = '\0'; // posiciona um novo fim na string para impressão do erro.
		}
		
		Error("tag <?cweb is incorrect.\nEnd of Tag found (until the end of line): \"%s\"\n"
			"Expected one of these sequences: \" ?>\" (with \' \')\nfor more details see"
			" documentation.\nClient Output Name: \"%s\"", tagEnd, cout->name);
	}
	
	/////////////////////////////////////////////////////////////////
	// treat the end of tag after NAME
	/////////////////////////////////////////////////////////////////
	tag[0] = '\0'; // finaliza a string passada
	char *new_cout_data = strchr(tagEnd, '>');
	new_cout_data = &new_cout_data[1]; // passa para o próximo character
	
	return new_cout_data;
}



/**
 * Retorna os nós (roots ou defaults) que ainda faltam imprimir
 */
static Out_t
_Get_Root_COutput_To_Print (const bool isError)
{
	arrayList_t rootList = NULL;
	if(isError == true) {
		rootList = _Get_RootList_Error();
	} else {
		rootList = _Get_RootList();
	}
	
	if(rootList->Len(rootList->self) == 0) { // verifica se ainda tem node root ou default para enviar
		return NULL;
	}
	
	// recebe e retira o node root da lista de nós roots que ainda faltam imprimir
	Out_t root = (Out_t)rootList->Del(rootList->self, 0);
	if(root == NULL) {
		Error("Root Node is NULL.\nRootList Len: %d - After Del operation", rootList->Len(rootList->self));
	}
	
	if(root->hasPrinted == true) { // evita criação de loop e repetição na árvore de chamadas
		Error("Root Node already printed.\nRoot Name: \"%s\"\nRootList Len: %d - After Del operation",
			root->name, rootList->Len(rootList->self));
	}
	
	return root;
}

/**
 * parser o client_output.
 * abre o arquivo (se for o caso) e joga todo o seu conteúdo para o cliente.
 * caso seja necessário, chamar outro clientOutput - também faz isso.
*/
static void
_Parser(Out_t cout)
{

	if(cout->hasPrinted == true) {
		Error("Node already printed. - Client Output name: \"%s\"", cout->name);
	}
	cout->hasPrinted = true; // coloca antes para evitar chamadas recursivas do mesmo cout
	
	/////////////////////////////////////////////////////////////////
	// unifica o tratamento para todos os tipos
	/////////////////////////////////////////////////////////////////
	char *cout_data = NULL;
	switch(cout->type) { 
		case string:
			cout_data = cout->var.str;
			break;
		case file_name:
			cout_data = FileUtil_StrMap(cout->var.file_name, "file_name");
			if(cout_data == NULL) {
				Error("data file is NULL.\nType code: %d\nClient Output Name: \"%s\"",
					cout->type, cout->name);
			}
			break;
		case file:
			cout_data = FileUtil_StrMap(cout->var.file, "file");
			if(cout_data == NULL) {
				Error("data file is NULL.\nType code: %d\nClient Output Name: \"%s\"",
					cout->type, cout->name);
			}
			break;
		default:
			Error("Type wrong.\nType code: %d\nClient Output Name: \"%s\"", cout->type, cout->name);
	}
	
	_Parser_Main_Loop(cout, cout_data); // realiza o parser de fato - chama o main loop
}

////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////
/*
Out_t
Out_StrMap_New_Interface()
{
	Out_t co = (Out_t)MM_Malloc(sizeof(Out_o));
	if(co == NULL) {
		Error("Allocated Space for Client Output Interface");
	}
	
	// insere o self
	co->self = NULL;
	
	// insere as funções da interface
	co->Set			= Out_StrMap_Set;
	co->Print		= Out_StrMap_Print;
	co->Print_Error = Out_StrMap_Print_Error;
	
	return co;
}


Out_t
Out_StrMap_Singleton_Interface()
{
	static Out_t icout = NULL;
	
	if(icout == NULL)
	{
		icout = Out_StrMap_New_Interface();
		if(icout == NULL)
		{
			Error("could not create singleton instance of Client Output.");
		}
	}
	
	return icout;
}
*/

////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////
/**
 *
 */
int
CWeb_Out_Set(const char *name, const void *output,
			 const char *type, const char *opt)
{
	_Check(name, output, type, opt);
	
	// cria um objeto de saída
	Out_t out = MM_Malloc(sizeof(Out_o));
	if(out == NULL) {
		Error("Not allocated space for out.");
	}
	
	// guarda o output de acordo com o tipo
	if(strcmp(type, "file") == 0)  {
		out->var.file = (FILE*)output;
	}
	else if(strcmp(type, "file_name") == 0 ||
			strcmp(type, "string") == 0) { // copia a string internamente 
		char *strOut  = (char*)output;
		char *str = MM_Malloc(strlen(strOut)+1);
		if(str == NULL) {
			Error("Allocated memory to keep output string.\noutput = \"%s\"\nouput len: %d",
				strOut, strlen(strOut));
		}
		
		strcpy(str, strOut); // copia a string
		
		// copia a string adequadamente com o tipo na union 
		if(strcmp(type, "file_name") == 0) {
			out->type = file_name;
			out->var.file_name = str;
		} else {
			out->type = string;
			out->var.str = str;
		}
	} else {
		Error("Not a valid type.");
	}
	
	// guarda os dados passados pelo usuário
	out->hasPrinted = false; // define que este tipo ainda não foi impresso no arquivo de saída
	strcpy(out->name, name); // guarda o nome para o caso de erro
	

	
	if(strcmp(opt, "root") == 0) // verifica e insere na lista de nós roots - insere sempre na última posição
	{
		out->opt = Root;
		arrayList_t rootList = _Get_RootList();
		rootList->Add(rootList->self, rootList->Len(rootList->self), (void*)out);
	} 
	else if(strcmp(opt, "leaf") == 0) // verifica e insere no map que contém todas as leafs
	{
		out->opt = Leaf;
		map_t leafMap = _Get_LeafMap();
		leafMap->Set(leafMap->self, name, (void*)out);
	}
	else if(strcmp(opt, "error_root") == 0)
	{
		out->opt = Error_Root;
		arrayList_t rootListError = _Get_RootList_Error();
		rootListError->Add(rootListError->self, rootListError->Len(rootListError->self), (void*)out);
	}
	else if(strcmp(opt, "error_leaf") == 0)
	{
		out->opt = Error_Leaf;
		map_t leafMapError = _Get_LeafMap_Error();
		leafMapError->Set(leafMapError->self, name, (void*)out);
	}
	
	return 0; // não houve erro
}

/**
 * Envia o arquivo gerado na saída para a saída padrão do fast-cgi.
 * NÃO É RECURSIVO - Se um arquivo já foi impresso, ele NÃO será novamente impresso.
 * Isto é feito para evitar entradas em loops infinitos
 */
int CWeb_Out_Print()
{
	Out_t cout = NULL;
	while((cout = _Get_Root_COutput_To_Print(false)) != NULL) {
		_Parser(cout);
		// free memory
/*		if(cout->type == file_name) {
			MM_Free(cout->var.file_name);
		}
		MM_Free(cout);*/
	}
	
	_StrMap_Print();
	
	_Check_Print_All_ClientOutput(false);
	
	return 0;
}

int CWeb_Out_Print_Error()
{
	Out_t cout = NULL;
	while((cout = _Get_Root_COutput_To_Print(true)) != NULL) {
		_Parser(cout);
		// free memory
/*		if(cout->type == file_name) {
			MM_Free(cout->var.file_name);
		}
		MM_Free(cout);*/
	}
	
	_StrMap_Print();
	
	_Check_Print_All_ClientOutput(true);
	
	return 0;
}


/*
//	fprintf(stderr, "***************************************");
//	fprintf(stderr, "********ENTER HERE         ************");
//	fprintf(stderr, "***************************************");
//	fprintf(stderr, "Cout name: \"%s\"\ncout path file: \"%s\"\n", cout->name, cout->var.file_name);
*/













