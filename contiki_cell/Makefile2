CONTIKI_PROJECT = contiki_cell
all: $(CONTIKI_PROJECT)

#UIP_CONF_IPV6=1
TARGET_LIBFILES += -lpthread -lm
CFLAGS += -D FUNC_SWITCHER_matrix=false -D FUNC_SWITCHER_message=true -D FUNC_SWITCHER_som=false -D FUNC_SWITCHER_som_kohonenlayer_output=false -D FUNC_SWITCHER_som_train=false -D FUNC_SWITCHER_matrixcvm=false -D FUNC_SWITCHER_linearSearch=false -D FUNC_SWITCHER_SearchBST=false
CFLAGS +=-DFRAME_DEBUG
CONTIKI_SOURCEFILES += sem_com.c
CONTIKI_SOURCEFILES += matrix.c
CONTIKI_SOURCEFILES += matrixcvm.c
CONTIKI_SOURCEFILES += recursion_parallel.c
CONTIKI_SOURCEFILES += message.c
CONTIKI_SOURCEFILES += som.c
CONTIKI_SOURCEFILES += som_kohonenlayer_output.c
CONTIKI_SOURCEFILES += som_train.c
CONTIKI_SOURCEFILES += shm_com.c
CONTIKI_SOURCEFILES += cvm.c
CONTIKI_SOURCEFILES += cvm_api.c
CONTIKI_SOURCEFILES += sky.c
CONTIKI_SOURCEFILES += client.c
CONTIKI_SOURCEFILES += async_server.c
CONTIKI_SOURCEFILES += linearSearch.c
CONTIKI_SOURCEFILES += SearchBST.c
WITH_UIP=1
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
