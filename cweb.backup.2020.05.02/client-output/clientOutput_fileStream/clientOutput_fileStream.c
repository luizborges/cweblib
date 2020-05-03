/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "clientOutput_fileStream.h"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////
// para tratamento interno da variável type
enum ClientOutput_Type_t {
	string,
	file,
	file_name
};

// para tratamento interno da variável opt
enum ClientOutput_Opt_t {
	Root, // NÃO pode ser error - não é chamado pela tag <?cweb #include "NAME" ?>
	Leaf, // NÃO pode ser error - só é chamado pela tag <?cweb #include "NAME" ?>
	Error_Root, // só pode ser error - não é chamado pela tag <?cweb #include "NAME" ?>
	Error_Leaf // só pode ser error - só é chamado pela tag <?cweb #include "NAME" ?>
};

// enum para controle do estado em que se encontra o parser
enum ClientOutput_ParserState_t {
	print,
	do_nothing,
	print_tag_header,
	inside_tag
};

union ClientOutput_Var_o {
	FILE *file;
	char *file_name;
	char *str;
};


typedef struct {
	union ClientOutput_Var_o var;
	enum  ClientOutput_Type_t type;
	enum  ClientOutput_Opt_t opt;

	bool  hasPrinted;
	char  name[CLIENTOUTPUT_FILESTREAM_NAME_MAX]; // key do map para esta saída
} ClientOutput_o;

typedef ClientOutput_o* ClientOutput_t;


////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////
static ClientOutput_t
_ClientOutput_Get_Root_COutput_To_Print (bool isError);


static void
_ClientOutput_Parser(ClientOutput_t cout);
										
////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////

static inline bool
_ClientOutput_Has_Only_Alpha_Num(const char *str)
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
_ClientOutput_Get_LeafMap () 
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
_ClientOutput_Get_LeafMap_Error () 
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
_ClientOutput_Check(const char *name, const void *output,
				    const char *type, const char *opt)
{
	// check name 
	if(name == NULL) { Error("name is NULL"); }
	
	if(strlen(name) == 0) { Error("name is a empty string."); }
	
	if(strlen(name) < CLIENTOUTPUT_FILESTREAM_NAME_MIN) { 
		Error("name length is lesser than %d.\nname length: %d", CLIENTOUTPUT_FILESTREAM_NAME_MIN, strlen(name));
	}
	
	if(strlen(name) >= CLIENTOUTPUT_FILESTREAM_NAME_MAX) {
		Error("name length is greater than %d.\nname lenght: %d.",
			CLIENTOUTPUT_FILESTREAM_NAME_MAX -1, strlen(name));
	}
	
	if(_ClientOutput_Has_Only_Alpha_Num(name) == false) {
		Error("not a valid name.\nall character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-').\nname: \"%s\"", name);
	}
	
	// verifica se já existe um nome igual no map
	map_t map = _ClientOutput_Get_LeafMap();
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
_ClientOutput_Get_RootList ()
{
	static arrayList_t rootList = NULL;
	
	if(rootList == NULL) { // inicializa o map, se isto ainda não ocorreu
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
_ClientOutput_Get_RootList_Error ()
{
	static arrayList_t rootListError = NULL;
	
	if(rootListError == NULL) { // inicializa o map, se isto ainda não ocorreu
		rootListError = Abstract_Factory_Common(ArrayList);
		if(rootListError == NULL) {
			Error("Creating Root List");
		}
	}
	
	return rootListError;
}



static inline int
_ClientOutput_Parser_Get_Char_By_Type(const ClientOutput_t cout,
									  FILE *file_,
									  int  *strIdx)
{
	return file_ != NULL ? fgetc(file_) : cout->var.str[++*strIdx];
}


/**
 * Verifica se a tag "<?cweb" foi alcançada.
 * retorna o estado atual do arquivo de entrada com base no enum ClientOutput_ParserState_t
 */
static inline enum ClientOutput_ParserState_t
_ClientOutput_Parser_Outside_Tag(const int c,
								 const ClientOutput_t cout,
								 const int lineNum)
{
	char tag_cweb[7] = "<?cweb";
	static int pos = 0; // controla se as sequências de caracteres estão batendo com a tag_cweb - indica a posição do último caratere - em sequência imediata, que está compatível com a tag_cweb. - feito para descobrir se a tag_cweb está escrita no arquivo
	
	// otimização para verificação do carater atual - já que a maioria dos carateres não se enquandram na busca - manda imprimir o caractere atual
	if(pos == 0 && tag_cweb[0] != c) { return print; }
	
	// faz a verificação para ver se entrou na tag
	if(pos < 6) { // ou seja, se ainda não foi alcançado a compatibilidade com a str tag_web - o número 6 significa o tamanho da string "<?cweb"
		if(tag_cweb[pos] == c) {
			++pos; // incrementa o valor de pos, ou seja, na próxima interação, irá conferir o próximo caracter da sequência.
			return do_nothing;
		} else {  
			// imprime até onde foi compatível com a sequência da tag_cweb e reseta a contagem
			// NÃO IMPRIME O CARACTER ATUAL
			for(int i=0; i < pos; ++i) {
				printf("%c", tag_cweb[i]);
			}
			pos = 0; // reseta a checagem para a tag
			return print; // ou seja, manda imprimir o caractere atual
		}
	} else if(pos == 6) { // a sequência da tag foi alcançada - verifica o próximo caratere depois da string tag_cweb
	
		if(c == '@') { // verificar se é apenas para imprimir a sequência "<?cweb" - o caracter '@' é omitido - a sequência "<?cweb@" -> imprime "<?cweb"
			printf("<?cweb"); // imprime a tag_cweb
			pos = 0; // reseta a posição de verificação para entrar na tag
			return do_nothing; // não se imprime o caractere de escape
		}
		else { // verifica se está realmente dentro da tag ou seja, se o carater é válido
			switch(c) { // depois de aberto a tag, somente pode haver estes próximos caracteres
			            // para qualquer outro caractere será emitido um erro
				case ' ':  break;
				case '\t': break;
				case '\n': break;
				default: Error("first character inside tag \"<?cweb\" is not valid.\ncharacter: \'%c\'."
					"\ninside tag \"<?cweb\" is expecting these character: \' \' (white space) " 
					"or \'\\t\' (tab) or \'\\n\' (new line).\nTo escape character use \'@\' - "
					"see documentation for more details about parser of tag \"<?cweb\".\n"
					"Client Output Name: \"%s\".\nLine Number where is problem: %d.",
					c, cout->name, lineNum);
			}
			
			++pos; // atualiza a posição da tag
			return do_nothing;
		}
		
	} else if(pos == 7) { // resetar é preciso, pois somente essa função será chamada novamente para procurar novamente outra tag cweb
		pos = 0; // reseta a posição
		return inside_tag;
		
	} else {
		Error("Parser error. state of position in parser is not valid.\n"
			"Pos = %d\nCharacter problem: \'%c\'"
			"Client Output Name: \"%s\".\nLine Number where is problem: %d",
			pos, c, cout->name, lineNum);
	}
	
	// deve necessariamente entrar em algum estado anterior - se não está com erro
	Error("Parser error to check tag init.\nstate of position in parser is not valid.\n"
			"Pos = %d\nCharacter problem: \'%c\'"
			"Client Output Name: \"%s\".\nLine Number where is problem: %d",
			pos, c, cout->name, lineNum);
	return do_nothing;
}


static inline void
_ClientOutput_Parser_Inside_Tag_Check_Name(const char *name,
										   const bool isValidName,
										   const int c,
										   const ClientOutput_t cout,
										   const int lineNum)
{
	if(isValidName == false) { // verifica se o nome é maior que o limite permitido
		Error("not a valid name.\nname not ended with a terminator character, that is \'\"\'.\n"
			"expected character: \'\"\'.\nfound character: \'%c\'\n"
			"in tag <?cweb #include \"\ngiven name (with correct max length): \"%s\"\n"
			"max length to name: %d.\nClient Output Name: \"%s\".\nLine Number where is problem: %d.",
			c, name, CLIENTOUTPUT_FILESTREAM_NAME_MAX -1, cout->name, lineNum);
	}
	
	if(strlen(name) < CLIENTOUTPUT_FILESTREAM_NAME_MIN) { // verifica se o nome é menor que o limite permitido
		Error("not a valid name.\nname is too short, it must to be equal or greater than %d characters.\nin tag <?cweb #include \"\ngiven name: \"%s\"\n"
			"min length to name: %d.\nClient Output Name: \"%s\".\nLine Number where is problem: %d.",
			CLIENTOUTPUT_FILESTREAM_NAME_MIN, name, CLIENTOUTPUT_FILESTREAM_NAME_MIN, cout->name, lineNum);
	}
	
	if(_ClientOutput_Has_Only_Alpha_Num(name) == false) { // verifica se é apenas dígito ou alfa
		Error("not a valid name.\nall character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-').\nin tag <?cweb #include \"\ngiven name: \"%s\"\nClient Output Name: \"%s\".\nLine Number where is problem: %d.", name, cout->name, lineNum);
	}
}

static inline int
_ClientOutput_Parser_Inside_Tag(const ClientOutput_t cout,
								const int lineNum_,
								FILE *file_,
								int  *strIdx)
{
	int lineNum = (int)lineNum_;
	int c;
	
	////////////////////////////////////////////////////////
	// verifica se a tag tagStr foi incluída corretamente
	////////////////////////////////////////////////////////
	char tagStr[11] = "#include \"";
	for(int i=0; i < 10; ++i) {
		c = _ClientOutput_Parser_Get_Char_By_Type(cout, file_, strIdx);
		if(c != tagStr[i]) {
			Error("tag cweb incorrect: \"%s\" macro error.\n"
				"character expected: \'%c\'.\ncharacter wrong: \'%c\'\nPosition in expected string: %d.\n"
				"Client Output Name: \"%s\".\nLine Number where is problem: %d.",
				tagStr, tagStr[i], c, i, cout->name, lineNum);
		}
	}
	
	////////////////////////////////////////////////////////
	// recebe o nome do arquivo
	////////////////////////////////////////////////////////
	char coutChildName[CLIENTOUTPUT_FILESTREAM_NAME_MAX];
	bool isValidName = false;
	for(int i=0; i < CLIENTOUTPUT_FILESTREAM_NAME_MAX; ++i) {
		c = _ClientOutput_Parser_Get_Char_By_Type(cout, file_, strIdx);
		if(c != '\"') {
			coutChildName[i] = c;
		} else {
			coutChildName[i] = '\0';
			isValidName = true;
			break;
		}
	}
	c = coutChildName[CLIENTOUTPUT_FILESTREAM_NAME_MAX-1]; // recolhe o último character para exibir erro
	coutChildName[CLIENTOUTPUT_FILESTREAM_NAME_MAX-1] = '\0'; // para garantir integridade da string
	_ClientOutput_Parser_Inside_Tag_Check_Name(coutChildName, isValidName, c, cout, lineNum);
	
	////////////////////////////////////////////////////////
	// chama o cout filho e coloca ele para rodar
	////////////////////////////////////////////////////////
	//check map type - opt option - se está imprimindo um error ou não.
	map_t map = NULL;
	if(cout->opt == Error_Root ||
	   cout->opt == Error_Leaf)
	{
		map = _ClientOutput_Get_LeafMap_Error();
	}
	else
	{
		map = _ClientOutput_Get_LeafMap();
	}
	
	if(map == NULL) {
		Error("map is NULL. map = %p", map);
	}
	if(map->HasKey(map->self, coutChildName) == true) {
		ClientOutput_t child = map->Del(map->self, coutChildName);
		if(child == NULL) { // verifica se ocorreu algum erro
			Error("leaf is NULL.\nname = \"%s\".\nClient Output Name: \"%s\".\n"
			"Line Number where is problem: %d.", coutChildName, cout->name, lineNum);
		}
		if(child->opt != Leaf &&
		   child->opt != Error_Leaf) {
			Error("child is not leaf option.\noption: %d\nname = \"%s\".\nClient Output Name: \"%s\".\nLine Number where is problem: %d.",child->opt, coutChildName, cout->name, lineNum);
		}
		if(child->opt == Leaf &&
		   (cout->opt != Root &&
		    cout->opt != Leaf)) {
			Error("child is Leaf but its parent not.\nParent opt is %d", cout->opt);
		}
		if(child->opt == Error_Leaf &&
		   (cout->opt != Error_Root &&
		    cout->opt != Error_Leaf)) {
		    Error("child is Error_Leaf but its parent not.\nParent opt is %d", cout->opt);
		}
		if(child->hasPrinted == true) {
			Error("leaf already printed.\nname = \"%s\".\nClient Output Name: \"%s\".\nLine Number where is problem: %d.", coutChildName, cout->name, lineNum);
		}
		
		_ClientOutput_Parser(child); // inicia a recursão - para o pai e chama o filho
	} else // não tem o name no leaf map
	{
		int numKeys = -1;
		char **key = map->Key(map->self, &numKeys);
		fprintf(stderr, "*********\nAll keys in leaf map\n**********\nnumber of keys: %d\n",numKeys);
		for(int i=0; i < numKeys; ++i) { // imprime todas as chaves do leaf key
			fprintf(stderr,"key[%d]: \"%s\"\n", i, key[i]);
		}
		Error("No name as key in leaf map.\nName could be not set or be already printed.\n"
			"name = \"%s\".\nClient Output Name: \"%s\".\nLine Number where is problem: %d.",
			coutChildName, cout->name, lineNum);
	}
	
	
	////////////////////////////////////////////////////////
	// verifica se o final da tag está correto e sai da tag
	////////////////////////////////////////////////////////
	c = _ClientOutput_Parser_Get_Char_By_Type(cout, file_, strIdx); // recebe o character após o fim do nome e do character '"'
	switch(c) { // depois de fechado o nome, somente pode haver estes próximos caracteres
			            // para qualquer outro caractere será emitido um erro
				case ' ':  break;
				case '\t': break;
				case '\n': ++lineNum; break; // atualiza a posição da liha
				default: Error("first character after tag \"<?cweb #include \"NAME_CHILD\" is not valid.\ncharacter: \'%c\'.\ninside tag \"<?cweb\" is expecting these character: \' \' (white space) " 
					"or \'\\t\' (tab) or \'\\n\' (new line).\n"
					"see documentation for more details about parser of tag \"<?cweb\".\n"
					"Client Output Name: \"%s\".\nLine Number where is problem: %d.",
					c, cout->name, lineNum);
	}
	
	char tagEnd[3]  = "?>";
	for(int i=0; i < 2; ++i) {
		c = _ClientOutput_Parser_Get_Char_By_Type(cout, file_, strIdx);
		if(c != tagEnd[i]) {
			Error("tag cweb incorrect: \'?>\' end tag error.\n"
				"expected: \"%s\".\ncharacter wrong: \'%c\'\nPosition in expected string: %d.\n"
				"Client Output Name: \"%s\".\nLine Number where is problem: %d.",
				tagEnd, c, i, cout->name, lineNum);
		}
	}
	
	return lineNum; // retorna a linha - que pode ter sido atualizada
}


static inline int
_ClientOutput_Parser_State_MainLoop(ClientOutput_t cout,
									char c,
									int lineNum,
									FILE *file_,
									int  *strIdx)
{
	enum ClientOutput_ParserState_t state = _ClientOutput_Parser_Outside_Tag(c, cout, lineNum);
			
	switch(state) {
		case print:	printf("%c", c); break;
		case do_nothing: break;
		case print_tag_header: printf("<?cweb"); break;
		case inside_tag:
			// repõe o caracter no stream para ser novamente lido e analisado na função _ClientOutput_Parser_Inside_Tag
			strIdx != NULL ? *strIdx -= 1 : ungetc(c, file_);
			
			lineNum = _ClientOutput_Parser_Inside_Tag(cout, lineNum, file_, strIdx); break;
		default: Error("no defined state - state_id: %d", state);
	}
			
	if(c == '\n') { ++lineNum; } // atualiza qual a linha que o arquivo se encontra
	
	return lineNum;
}
////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////

/**
 * Retorna os nós (roots ou defaults) que ainda faltam imprimir
 */
static ClientOutput_t
_ClientOutput_Get_Root_COutput_To_Print (bool isError)
{
	arrayList_t rootList = NULL;
	if(isError == true) {
		rootList = _ClientOutput_Get_RootList_Error();
	} else {
		rootList = _ClientOutput_Get_RootList();
	}
	
	if(rootList->Len(rootList->self) == 0) { // verifica se ainda tem node root ou default para enviar
		return NULL;
	}
	
	// recebe e retira o node root da lista de nós roots que ainda faltam imprimir
	ClientOutput_t root = (ClientOutput_t)rootList->Del(rootList->self, 0);
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
_ClientOutput_Parser(ClientOutput_t cout)
{
//	fprintf(stderr, "***************************************");
//	fprintf(stderr, "********ENTER HERE         ************");
//	fprintf(stderr, "***************************************");
//	fprintf(stderr, "Cout name: \"%s\"\ncout path file: \"%s\"\n", cout->name, cout->var.file_name);
	if(cout->hasPrinted == true) {
		Error("Node already printed. - Client Output name: \"%s\"", cout->name);
	}
	cout->hasPrinted = true; // coloca antes para evitar chamadas recursivas do mesmo cout
	int lineNum = 1; // guarda o número de linhas que o arquivo contém - para o caso de erro
	
	// separa o parser por tipos
	if(cout->type == string) { // parser a string
		int len = strlen(cout->var.str);
		for(int i=0; i < len; ++i) {
//			fprintf(stderr, "i:[%d of %d] - char: \'%c\'\n", i, len, cout->var.str[i]);
			lineNum = _ClientOutput_Parser_State_MainLoop(cout, cout->var.str[i], lineNum, NULL, &i);
		}
	} else if(cout->type == file ||
			  cout->type == file_name)
	{
		FILE *file_ = NULL;
		if(cout->type == file_name) // abre o arquivo, se cout for do tipo file_name
		{
			file_ = fopen(cout->var.file_name, "r");
			if(file_ == NULL) {
				Error("error in open file.\nfile_name: \"%s\"", cout->var.file_name);
			}
		} else {
			file_ = cout->var.file; // para unificar o parser dos tipos file_name e file
		}
		
		int c;
		while((c = fgetc(file_)) != EOF) {
			lineNum = _ClientOutput_Parser_State_MainLoop(cout, c, lineNum, file_, NULL);
		}
		
		if(cout->type == file_name) { // fecha o arquivo aberto caso é file_name
			fclose(file_);
		}
	} else {
		Error("No valid type for Client Output.\nType: %d\nClient Output Name: \"%s\"\n",
			cout->type, cout->name);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////
clientOutput_t
ClientOutput_FileStream_New_Interface()
{
	clientOutput_t co = (clientOutput_t)MM_Malloc(sizeof(clientOutput_o));
	if(co == NULL) {
		Error("Allocated Space for Client Output Interface");
	}
	
	// insere o self
	co->self = NULL;
	
	// insere as funções da interface
	co->Set			= ClientOutput_FileStream_Set;
	co->Print		= ClientOutput_FileStream_Print;
	co->Print_Error = ClientOutput_FileStream_Print_Error;
	
	return co;
}


////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////
/**
 *
 */
int
ClientOutput_FileStream_Set(void *self_,
							const char *name, const void *output,
			                const char *type, const char *opt)
{
	_ClientOutput_Check(name, output, type, opt);
	
	// cria um objeto de saída
	ClientOutput_t out = MM_Malloc(sizeof(ClientOutput_o));
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
		arrayList_t rootList = _ClientOutput_Get_RootList();
		rootList->Add(rootList->self, rootList->Len(rootList->self), (void*)out);
	} 
	else if(strcmp(opt, "leaf") == 0) // verifica e insere no map que contém todas as leafs
	{
		out->opt = Leaf;
		map_t leafMap = _ClientOutput_Get_LeafMap();
		leafMap->Set(leafMap->self, name, (void*)out);
	}
	else if(strcmp(opt, "error_root") == 0)
	{
		out->opt = Error_Root;
		arrayList_t rootListError = _ClientOutput_Get_RootList_Error();
		rootListError->Add(rootListError->self, rootListError->Len(rootListError->self), (void*)out);
	}
	else if(strcmp(opt, "error_leaf") == 0)
	{
		out->opt = Error_Leaf;
		map_t leafMapError = _ClientOutput_Get_LeafMap_Error();
		leafMapError->Set(leafMapError->self, name, (void*)out);
	}
	
	return 0; // não houve erro
}

/**
 * Envia o arquivo gerado na saída para a saída padrão do fast-cgi.
 * NÃO É RECURSIVO - Se um arquivo já foi impresso, ele NÃO será novamente impresso.
 * Isto é feito para evitar entradas em loops infinitos
 */
int ClientOutput_FileStream_Print(void *self_)
{
	ClientOutput_t cout = NULL;
	while((cout = _ClientOutput_Get_Root_COutput_To_Print(false)) != NULL) {
		_ClientOutput_Parser(cout);
	}
	
	return 0;
}

int ClientOutput_FileStream_Print_Error(void *self_)
{
	ClientOutput_t cout = NULL;
	while((cout = _ClientOutput_Get_Root_COutput_To_Print(true)) != NULL) {
		_ClientOutput_Parser(cout);
	}
	
	return 0;
}

















