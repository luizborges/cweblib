CC              = gcc
DLIB_DIR_H      = /var/www/shared
DLIB_DIR        = $(DLIB_DIR_H)/dlibs
DLIB_DIR_LPATH  = $(foreach dir,$(DLIB_DIR),   -L$(dir)) # add prefix to all dir
DLIB_DIR_H_IPATH= $(foreach dir,$(DLIB_DIR_H), -I$(dir)) # add prefix to all dir
DLIB_DIR_RPATH  = $(foreach dir,$(DLIB_DIR),   -Wl,-rpath=$(dir)) # add prefix to all dir
DLIB_NAME       = # insert here all dynamics libraries in DLIB_DIR_H you want to use
# old -lclientOutput_strMap -lroute_easy -lclientInput_manager -lcookie_manager
# OLD -LIBCOMMON = -lerror -lmemoryManager -lstackTracer -lfileUtil -larrayList_noSync -lmap_ArrayList_noSync -labstractFactoryCommon
CFLAGS          = -Wall -g -O3 -DNDEBUG -Wno-variadic-macros -fPIC -Wl,--export-dynamic # Werror transforms warning in error
DLIB_STD        = -lm -lpthread -lfcgi -lgc
DLIB            = $(DLIB_STD) $(DLIB_NAME)
COMPILER_FLAGS  = $(CFLAGS) $(DLIB_DIR_LPATH) $(DLIB_DIR_H_IPATH)
LINK_FLAGS      = $(COMPILER_FLAGS) $(DLIB_DIR_RPATH) # use -Wl,-rpath= when the library is not in global environment
LINK_DLIB       = $(LINK_FLAGS) -shared -Wl,-soname,$(LIB)
# INCLUDE LIBRARIES OF THE LIBRARY
PERCENT         = percent/percent.c
ROUTE           = route/route_easy/route_easy.c
IN              = client-input/clientInput_manager/clientInput_manager.c client-input/get/get_strMap/get_strMap.c client-input/post/post_strMap/post_strMap.c
OUT             = client-output/clientOutput_strMap/clientOutput_strMap.c
COOKIE          = #cookie/cookie_strMap/cookie_strMap.c
# END
C_SRC_LIB       = cweb.c
C_SRC_MAIN      = main.c
C_SRC           = $(C_SRC_LIB) $(PERCENT) $(ROUTE) $(IN) $(OUT) $(COOKIE)
C_OBJ_ORI       = $(C_SRC:.c=.o)
C_SRC_NAME_ONLY = $(notdir $(C_SRC))
C_OBJ_NAME_ONLY = $(C_SRC_NAME_ONLY:.c=.o)
C_OBJ_DIR       = objs/
C_OBJ           = $(addprefix $(C_OBJ_DIR), $(C_OBJ_NAME_ONLY))
C_LIB_H         = $(C_SRC_LIB:.c=.h)
LIB             = lib$(C_SRC_LIB:.c=.so)
EXE             = exe

ARG1       = #-q input.dat
ARG2       = #-o output.dat
ARG3       = #-i list_log_files.dat
ARG4       = #-d date.dat
ARG5       = #-e str_end_block.dat

run: linker
	$(info $nrun: $(EXE))
	./$(EXE) $(ARG1) $(ARG2) $(ARG3) $(ARG4) $(ARG5)

lib: linker_lib export_lib export_lib_header
	$(info Dynamic Library created with success: $(LIB) $nDynamic Library Header exported with success: $(C_LIB_H)$nTo direct use: #include <headers/$(C_LIB_H)> $nFramework Library exported with success.$nTo use Framework CWEB: #include <headers/$(GLOBAL_LIB)>)
# can use to print: $(info your_text) $(warning your_text) or $(error your_text) # for new/break line use: $nYour_text - ex: my_text_line1 $nmy_text_line2

linker_lib: cscrean clean_lib $(C_SRC:.c=.o) mv_c_obj
	$(info $nlinkier objects to produce: $(LIB))
	$(CC) $(LINK_DLIB) $(C_OBJ) -o $(LIB) $(DLIB)

linker: cscrean clean add_c_src_main $(C_SRC:.c=.o) $(C_SRC_MAIN:.c=.o) mv_c_obj
	$(info $nlinker objects to produce: $(EXE))
	$(CC) $(LINK_FLAGS) $(C_OBJ) -o $(EXE) $(DLIB)

$(C_SRC:.c=.o): %.o : %.c
	$(info $ncompile: $<)
	$(CC) $(COMPILER_FLAGS) -c $< -o $@ $(DLIB)
	$(info $n)

$(C_SRC_MAIN:.c=.o): %.o : %.c
	$(info $ncompile: $<)
	$(CC) $(COMPILER_FLAGS) -c $< -o $@ $(DLIB)
	
export_lib:
	$(info $nexport lib and lib_header:)
	sudo cp $(LIB) $(DLIB_DIR)/

export_lib_header:
	sudo cp $(C_LIB_H) $(DLIB_DIR_H)/headers/

mv_c_obj: 
	mv $(C_OBJ_ORI) $(C_OBJ_DIR)

# only in C_SRC variable is necessary to concatenate whit C_SRC_MAIN, all others just loading them again
add_c_src_main:
	$(info add files from C_SRC_MAIN to global compile variables)
	$(eval C_SRC += $(C_SRC_MAIN))
	$(info new makefile variable: C_SRC = $(C_SRC))
	$(eval C_OBJ_ORI = $(C_SRC:.c=.o))
	$(info new makefile variable: C_OBJ_ORI = $(C_OBJ_ORI))
	$(eval C_SRC_NAME_ONLY = $(notdir $(C_SRC)))
	$(info new makefile variable: C_SRC_NAME_ONLY = $(C_SRC_NAME_ONLY))
	$(eval C_OBJ_NAME_ONLY = $(C_SRC_NAME_ONLY:.c=.o))
	$(info new makefile variable: C_OBJ_NAME_ONLY = $(C_OBJ_NAME_ONLY))
	$(eval C_OBJ = $(addprefix $(C_OBJ_DIR), $(C_OBJ_NAME_ONLY)))
	$(info new makefile variable: C_OBJ = $(C_OBJ))

clean_lib: clean
	sudo rm -rf $(LIB) $(DLIB_DIR)/$(LIB) $(DLIB_DIR_H)/headers/$(C_LIB_H)

clean:
	rm -rf $(C_OBJ_DIR)*.o *~ *.out *.key out.txt

cscrean:
	clear

valg: compile
	valgrind ./$(EXE) $(ARG1) $(ARG2)
	
define n # define a break line - new line to user's message


endef

























	
