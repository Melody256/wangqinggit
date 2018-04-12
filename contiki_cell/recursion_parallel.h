/*
 * task_ctl.h
 *
 *  Created on: 2014-9-9
 *      Author: wintice
 */

#ifndef RECURSION_PARALLEL_H_
#define RECURSION_PARALLEL_H_

#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "shm_com.h"
#include "sem_com.h"
#include <process.h>
#include "sky.h"

#define RECURSION_ENABLE
#define MAXNUM_FUNCTIONS 32
#define MAXNUM_CALL 2

#define SHM_KEY_ARK 0x01

#define CALL_MODE_INIT 0
#define CALL_MODE_ROOT 1
#define CALL_MODE_ARK 2
#define CALL_MODE_CELL 3
#define CALL_MODE_SKY 4

#define CALL_NO_BLOCKING 0
#define CALL_BLOCKING 1

#define FUNC_LIST(...)                  \
        void *(*recursion_functions[MAXNUM_FUNCTIONS])(void *) = {__VA_ARGS__, NULL}

#ifdef FRAME_DEBUG
#define FRAME_PRINTF(...) \
        printf(__VA_ARGS__);
#define FRAME_PERROR(...) \
        perror(__VA_ARGS__);
#else
#define FRAME_PRINTF(...) \
        ;
#define FRAME_PERROR(...) \
        ;
#endif

#define TASK_NODE(funcname) struct task_node_##funcname

#define RECURSION_FUNCTION_NAME(funcname) recursion_func_##funcname

#define RECURSION_FUNCTION(funcname) \
void *RECURSION_FUNCTION_NAME(funcname)(void *frame_call_node)

#define INPUTPARA(x) (input_para->x)

#define FUNCTION_INIT(funcname) \
                if(((struct str_func_call_node*)frame_call_node)->mode==CALL_MODE_INIT) {\
                    func_idx_##funcname = ((struct str_func_call_node*)frame_call_node)->shmid;\
                    g_strFunc_NameA[((struct str_func_call_node*)frame_call_node)->thread_idx] = #funcname;\
                    free(frame_call_node);\
                    FRAME_PRINTF("%s init!",#funcname);\
                    if(FUNC_SWITCHER_##funcname) {\
                        shm_base[func_idx_##funcname] = BKDRHash(#funcname);\
                        g_strFunc_Name[func_idx_##funcname] = #funcname;\
                        while ((gSemIdFunc[func_idx_##funcname] = semget(shm_base[func_idx_##funcname], 1, 0666|IPC_CREAT))== -1) perror("function init semget:");\
                        init_sem(gSemIdFunc[func_idx_##funcname], 1);\
                        pthread_mutex_init(&(frame_mutex[func_idx_##funcname]),NULL);\
                        while ((gShmIdFunc[func_idx_##funcname] = shmget(shm_base[func_idx_##funcname] , sizeof(struct task_head_str), 0666|IPC_CREAT))== -1) perror(" function init shmget:");\
                        while ((task_head_addr[func_idx_##funcname] = shmat(gShmIdFunc[func_idx_##funcname], (void*) 0, 0)) == (void*) -1) perror("function init shmmat:");\
                        task_head_ark[func_idx_##funcname] = (struct task_head_str*) task_head_addr[func_idx_##funcname];\
                        while((task_head_cell[func_idx_##funcname] = (struct task_head_str*)malloc(sizeof(struct task_head_str)))==NULL) perror("malloc");;\
                        task_head_cell[func_idx_##funcname]->stack_top.shmid = 0;\
                        task_head_cell[func_idx_##funcname]->stack_top.addr = NULL;\
                        task_head_cell[func_idx_##funcname]->task_undone  =0;\
                        task_head_cell[func_idx_##funcname]->task_finish = 0;\
                        return (void*)1;\
                    }\
                    else {\
                        func_idx_##funcname = -1;\
                        return NULL;\
                    }\
                }\
                else if(((struct str_func_call_node*)frame_call_node)->mode==CALL_MODE_ROOT) {\
                    root_func_##funcname();\
                    free(frame_call_node);\
                    return NULL;\
                }\
                else if(((struct str_func_call_node*)frame_call_node)->mode==CALL_MODE_SKY) {{\
                    int frame_node_finish_flag=0;\
                    struct task_node_##funcname* input_para;\
                    input_para = ((struct str_func_call_node*)frame_call_node)->pt_node;\
                    int frame_mode_current = ((struct str_func_call_node *)frame_call_node)->mode;\
                    int frame_shmid_current = ((struct str_func_call_node*)frame_call_node)->shmid;\
                    int frame_thread_idx =  ((struct str_func_call_node*)frame_call_node)->thread_idx;\
                    free(frame_call_node);\
                    if((INPUTPARA(node_head).SrcMark.nSrcArk == g_nArkIdx)) {\
                        if(INPUTPARA(node_head).node_finish==0) {\
                            INPUTPARA(node_head).SrcMark.shmid = frame_shmid_current;\
                            TaskMigrate(frame_shmid_current, sizeof(TASK_NODE(funcname)));\
                        }\
                        else {\
                            NODE_FINISH();\
                        }\
                    }\
                    FUNC_END()\
                }\
                else {\
                    int frame_node_finish_flag=0;\
                    struct task_node_##funcname* input_para;\
                    input_para = ((struct str_func_call_node*)frame_call_node)->pt_node;\
                    int frame_mode_current = ((struct str_func_call_node *)frame_call_node)->mode;\
                    int frame_shmid_current = ((struct str_func_call_node*)frame_call_node)->shmid;\
                    int frame_thread_idx =  ((struct str_func_call_node*)frame_call_node)->thread_idx;\
                    int frame_countera=0,frame_counterb = 0;\
                    int frame_call_counter=0;\
                    int frame_func_idx_current = input_para->node_head.funcidx;\
                    free(frame_call_node);\
\
                    char frame_mode_calling = 0;\
                    int frame_func_idx_calling = 0;\
                    int frame_node_shmid_temp = 0;\
                    int frame_node_shmid_pre = 0;\
                    void * frame_addr_temp;

#define IF_FIRST_CALL() \
        if(INPUTPARA(node_head).times_recall == 0)

#define SHMINPUTPARA(dest_pointer,type_pointer,shmpara) \
        while ((frame_addr_temp= shmat((shmpara).nShmId, (void*) 0,0)) == (void *) -1) {\
                    perror("SHMINPUTPARA shmat");\
        } \
        type_pointer* const dest_pointer = ((type_pointer*)frame_addr_temp)+(shmpara).offset;

#define DEL_SHM_PARA(pt_para,shmpara) \
        while(shmdt(((void *)(pt_para-(shmpara).offset)))==-1) perror("DEL_SHM_PARA shmdt");

#define ELSE else

#define CALL_SUBFUNCTION_PREPARE(aimfuncname,node,nodenum,call_mode) \
    if(func_idx_##aimfuncname != -1) {\
        frame_mode_calling = call_mode;\
        for (frame_countera=0;frame_countera<nodenum;frame_countera++) {\
            void *frame_addr_temp;\
            if(frame_mode_calling == CALL_MODE_ARK||frame_mode_calling == CALL_MODE_SKY) {\
                while ((frame_node_shmid_temp =  shmget(IPC_PRIVATE,sizeof(TASK_NODE(aimfuncname)), 0666|IPC_CREAT|IPC_EXCL))== -1) {\
                }\
                while ( (frame_addr_temp = shmat(frame_node_shmid_temp, (void*) 0,0)) == (void*) -1) {\
                    perror("CALL_SUBFUNCTION_PREPARE shmmat");\
                } \
                (node)[frame_countera] = (TASK_NODE(aimfuncname)*)frame_addr_temp;\
                (node)[frame_countera]->node_head.idx_node.shmid=frame_node_shmid_temp;\
                (node)[frame_countera]->node_head.parent.mode=CALL_MODE_ARK;\
                (node)[frame_countera]->node_head.parent.uNodeIdx.shmid=frame_shmid_current;\
            }\
            else if(frame_mode_calling == CALL_MODE_CELL) {\
                while((frame_addr_temp = malloc(sizeof(TASK_NODE(aimfuncname))))==NULL) perror("CALL_SUBFUNCTION_PREPARE malloc");;\
                (node)[frame_countera] = (TASK_NODE(aimfuncname)*)frame_addr_temp;\
                (node)[frame_countera]->node_head.idx_node.addr=frame_addr_temp;\
                if(frame_mode_current == CALL_MODE_CELL) {\
                    (node)[frame_countera]->node_head.parent.mode=CALL_MODE_CELL;\
                    (node)[frame_countera]->node_head.parent.uNodeIdx.addr=(void *)input_para;\
                }\
                else if(frame_mode_current == CALL_MODE_ARK) {\
                    (node)[frame_countera]->node_head.parent.mode=CALL_MODE_ARK;\
                    (node)[frame_countera]->node_head.parent.uNodeIdx.shmid=frame_shmid_current;\
                }\
            }\
            while(((node)[frame_countera]->node_head.semid_node = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT|IPC_EXCL)) == -1) { \
            } \
            init_sem((node)[frame_countera]->node_head.semid_node, 1);\
            (node)[frame_countera]->node_head.mode = frame_mode_calling;\
            strcpy((node)[frame_countera]->node_head.func, #aimfuncname);\
            (node)[frame_countera]->node_head.funcidx=func_idx_##aimfuncname;\
            (node)[frame_countera]->node_head.times_recall=0;\
            (node)[frame_countera]->node_head.counter_para = 0;\
            (node)[frame_countera]->node_head.need_para = 0;\
            (node)[frame_countera]->node_head.SrcMark.nSrcArk = g_nArkIdx;\
        } \
    }\
    else {\
        printf("Error: Try to call a subfunc %s not in the same cell.", #aimfuncname);\
        exit(-1);\
    }

#define CALL_SUBFUNCTION(node,node_num) \
        INPUTPARA(node_head).need_para += node_num;\
        int frame_startidx = 0;\
        frame_countera = 0;\
        frame_func_idx_calling = node[0]->node_head.funcidx;\
        frame_mode_calling = node[0]->node_head.mode;\
        for(frame_countera = 0;frame_countera<node_num;frame_countera++) {\
            if(node[frame_countera]->node_head.funcidx != frame_func_idx_calling||node[frame_countera]->node_head.mode != frame_mode_calling) {\
                if(frame_countera != 0) {\
                    if(frame_mode_calling == CALL_MODE_ARK||frame_mode_calling == CALL_MODE_SKY) {\
                        sem_p(gSemIdFunc[frame_func_idx_calling]);\
                        node[frame_startidx]->node_head.right.shmid = task_head_ark[frame_func_idx_calling]->stack_top.shmid;\
                        task_head_ark[frame_func_idx_calling]->stack_top.shmid = node[frame_countera-1]->node_head.idx_node.shmid;\
                        task_head_ark[frame_func_idx_calling]->task_undone += frame_countera-frame_startidx;\
                        for (frame_counterb = frame_startidx; frame_counterb<frame_countera ; frame_counterb++) {\
                            while (shmdt((void *) node[frame_counterb]) == -1) perror("CALL_SUBFUNCTION shmdt");\
                        }\
                        sem_v(gSemIdFunc[frame_func_idx_calling]);\
                    }\
                    else if(frame_mode_calling == CALL_MODE_CELL) {\
                        pthread_mutex_lock(&frame_mutex[frame_func_idx_calling]);\
                        node[frame_startidx]->node_head.right.addr = task_head_cell[frame_func_idx_calling]->stack_top.addr;\
                        task_head_cell[frame_func_idx_calling]->stack_top.addr=(void *)(node[frame_countera-1]);\
                        task_head_cell[frame_func_idx_calling]->task_undone += frame_countera-frame_startidx;\
                        pthread_mutex_unlock(&frame_mutex[frame_func_idx_calling]);\
                    }\
                }\
                frame_func_idx_calling = node[frame_countera]->node_head.funcidx;\
                frame_startidx = frame_countera;\
                frame_mode_calling = node[frame_countera]->node_head.mode;\
            }\
            else if(frame_countera>frame_startidx)\
            {\
                if(frame_mode_calling == CALL_MODE_ARK||frame_mode_calling == CALL_MODE_SKY) \
                {\
                    node[frame_countera]->node_head.right.shmid = node[frame_countera-1]->node_head.idx_node.shmid;\
                }\
                else if(frame_mode_calling == CALL_MODE_CELL)\
                {\
                    node[frame_countera]->node_head.right.addr = node[frame_countera-1]->node_head.idx_node.addr;\
                }\
            }\
        }\
        if(frame_mode_calling == CALL_MODE_ARK||frame_mode_calling == CALL_MODE_SKY) {\
            frame_node_shmid_temp = node[frame_startidx]->node_head.right.shmid;\
            sem_p(gSemIdFunc[frame_func_idx_calling]);\
            node[frame_startidx]->node_head.right.shmid = task_head_ark[frame_func_idx_calling]->stack_top.shmid;\
            task_head_ark[frame_func_idx_calling]->stack_top.shmid=node[node_num-1]->node_head.idx_node.shmid;\
            task_head_ark[frame_func_idx_calling]->task_undone += node_num-frame_startidx;\
            sem_v(gSemIdFunc[frame_func_idx_calling]);\
            for (frame_counterb = frame_startidx; frame_counterb<node_num ; frame_counterb++) {\
                while (shmdt((void *) node[frame_counterb]) == -1) perror("CALL_SUBFUNCTION shmdt");\
            }\
        }\
        else if(frame_mode_calling == CALL_MODE_CELL) {\
            pthread_mutex_lock(&frame_mutex[frame_func_idx_calling]);\
            node[frame_startidx]->node_head.right.addr = task_head_cell[frame_func_idx_calling]->stack_top.addr;\
            task_head_cell[frame_func_idx_calling]->stack_top.addr=(void *)(node[node_num-1]);\
            task_head_cell[frame_func_idx_calling]->task_undone += node_num-frame_startidx;\
            pthread_mutex_unlock(&frame_mutex[frame_func_idx_calling]);\
        }\

#define IF_WAITING_RETURN() \
        frame_call_counter++;\
        if(INPUTPARA(node_head).times_recall == frame_call_counter)

#define NODE_FINISH() {frame_node_finish_flag=1;INPUTPARA(node_head).node_finish=1;}

#define FUNC_END() \
    FUNC_RETURN()\
    END_NODE()\

#define FUNC_RETURN() \
        if (frame_node_finish_flag == 1){\
            if(INPUTPARA(node_head).SrcMark.nSrcArk != g_nArkIdx) {\
                SkyFuncReturn(INPUTPARA(node_head).SrcMark.shmid, INPUTPARA(node_head).SrcMark.nSrcArk);\
                            FRAME_PRINTF("Cell %d:Sky returning!\n",nCellIdx );\
            }\
            else {\
                if(input_para->node_head.parent.mode == CALL_MODE_ROOT) {\
                } else {\
                    struct node_head_str  *func_parrent;\
                    if(input_para->node_head.parent.mode == CALL_MODE_ARK) {\
                        while ((func_parrent = (struct node_head_str *)shmat(input_para->node_head.parent.uNodeIdx.shmid, (void*) 0,0)) == ((struct node_head_str *) -1)) {\
                            perror("FUNC_RETURN shmat");\
                        }\
                        while (sem_p(func_parrent->semid_node) != 0) ;\
                        func_parrent->counter_para++;\
                        if (func_parrent->counter_para == func_parrent->need_para) {\
                            func_parrent->times_recall++;\
                            int nFrameSemTemp = semget(BKDRHash(func_parrent->func),1,0666|IPC_CREAT);\
                            int nFrameShmTemp = shmget(BKDRHash(func_parrent->func),0,0666|IPC_CREAT);\
                            struct task_head_str* pFrameShmatTemp = (struct task_head_str*)shmat(nFrameShmTemp,NULL,0);\
                            while (sem_p(nFrameSemTemp) != 0) ;\
                            func_parrent->right.shmid =pFrameShmatTemp->stack_top.shmid;\
                            while(sem_v(func_parrent->semid_node)!=0);\
                            pFrameShmatTemp->stack_top.shmid =  input_para->node_head.parent.uNodeIdx.shmid;\
                            pFrameShmatTemp->task_undone++;\
                            while (sem_v(nFrameSemTemp) != 0) ;\
                            shmdt((void*)pFrameShmatTemp);\
                            FRAME_PRINTF("Cell %d:Successful to call father function\n",nCellIdx);\
                        } else {\
                            while(sem_v(func_parrent->semid_node)!=0);\
                        }\
                        while(shmdt((void*)func_parrent)==-1) perror("FUNC_RETURN shmdt");\
                    }\
                    else if(input_para->node_head.parent.mode == CALL_MODE_CELL){\
                        func_parrent = (struct node_head_str *)input_para->node_head.parent.uNodeIdx.addr;\
                        while (sem_p(func_parrent->semid_node) != 0) ;\
                        func_parrent->counter_para++;\
                        FRAME_PRINTF("Cell %d:CELL return.Now %d/%d\n",nCellIdx,func_parrent->counter_para,func_parrent->need_para);\
                        if (func_parrent->counter_para == func_parrent->need_para) {\
                            FRAME_PRINTF("Cell %d:Call father function\n",nCellIdx);\
                            func_parrent->times_recall++;\
                            pthread_mutex_lock(&frame_mutex[func_parrent->funcidx]);\
                            func_parrent->right.addr =task_head_cell[func_parrent->funcidx]->stack_top.addr;\
                            while(sem_v(func_parrent->semid_node)!=0);\
                            task_head_cell[func_parrent->funcidx]->stack_top.addr = (void *)func_parrent;\
                            task_head_cell[func_parrent->funcidx]->task_undone++;\
                            pthread_mutex_unlock(&frame_mutex[func_parrent->funcidx]);\
                            FRAME_PRINTF("Cell %d:Successful to call father function\n",nCellIdx);\
                        } else{\
                            while(sem_v(func_parrent->semid_node)!=0);\
                        }\
                    }\
                }\
            }\
        }

#define END_NODE() \
        if(frame_node_finish_flag == 1) {while(del_sem(input_para->node_head.semid_node)!=0) perror("delsem");}\
        if(frame_mode_current == CALL_MODE_ARK||frame_mode_current == CALL_MODE_SKY) {\
            while(shmdt((void*) input_para) == -1)\
            perror("END_NODE shmdt");\
        }\
        if (frame_node_finish_flag == 1) {\
                if(frame_mode_current == CALL_MODE_ARK||frame_mode_current == CALL_MODE_SKY) {\
                    shmctl(frame_shmid_current, IPC_RMID, NULL);\
                }\
                else if(frame_mode_current == CALL_MODE_CELL){\
                    free((void*)input_para);\
                }\
            }\
            frame_func_return_flag[frame_thread_idx] = 1;\
            pthread_exit(NULL);\
    }\

#define CALL_FUNCTION_PREPARE(aimfuncname,node,call_mode) \
            void *frame_pt;\
            SHM_PARA frame_task_node;\
            node = NULL;\
            frame_task_node.nShmId = smalloc(IPC_PRIVATE,sizeof(TASK_NODE(aimfuncname)));\
            if (frame_task_node.nShmId != -1) {\
                frame_pt = smload(frame_task_node);\
                if(frame_pt!=NULL) {\
                    node = (TASK_NODE(aimfuncname)*)frame_pt;\
                    node->node_head.parent.mode=CALL_MODE_ROOT;\
                    while((node->node_head.semid_node = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT|IPC_EXCL)) == -1) { \
                        perror("call_function_prepare semget:");\
                    } \
                    init_sem(node->node_head.semid_node, 1);\
                    node->node_head.mode = call_mode;\
                    strcpy(node->node_head.func, #aimfuncname);\
                    node->node_head.funcidx=func_idx_##aimfuncname;\
                    node->node_head.times_recall = 0;\
                    node->node_head.counter_para = 0;\
                    node->node_head.node_finish = 0;\
                    node->node_head.need_para = 0;\
                    node->node_head.SrcMark.nSrcArk = g_nArkIdx;\
                    node->node_head.SrcMark.shmid = frame_task_node.nShmId;\
                }\
            }

#define CALL_FUNCTION(node,needblocking) \
        int nFrameKeyTemp = BKDRHash(node->node_head.func);\
        int nFrameSemTemp = semget(nFrameKeyTemp,1,0666|IPC_CREAT);\
        int nFrameShmTemp = shmget(nFrameKeyTemp,0,0666|IPC_CREAT);\
        struct task_head_str* pFrameShmatTemp = (struct task_head_str*)shmat(nFrameShmTemp,NULL,0);\
        sem_p(nFrameSemTemp);\
        node->node_head.right.shmid = pFrameShmatTemp->stack_top.shmid;\
        pFrameShmatTemp->stack_top.shmid=frame_task_node.nShmId;\
        pFrameShmatTemp->task_undone ++;\
        printf("Cell %d:Now %d task.semid:%d\n", nCellIdx, pFrameShmatTemp->task_undone, nFrameSemTemp);\
        sem_v(nFrameSemTemp);\
        shmdt((void *)pFrameShmatTemp);\
        if(needblocking) {\
            while(node->node_head.node_finish == 0){}\
        }\
        smunload(frame_task_node);\

struct str_ark_status
{
    volatile int num_cells;
    volatile int num_available;
};

typedef union
{
    int shmid;
    void* addr;
} NODEIDX;

typedef struct
{
    NODEIDX uNodeIdx;
    char mode;
} NODEMARK;

typedef struct
{
    int nSrcArk;
    int shmid;
} SRCNODEMARK;

struct task_head_str
{
    NODEIDX stack_top;
    volatile int task_undone;
    volatile int task_finish;
};

struct node_head_str
{
    NODEMARK parent;
    NODEIDX right;
    SRCNODEMARK SrcMark;
    char func[32];
    int funcidx;
    NODEIDX idx_node;
    int semid_node;
    int times_recall;
    char mode;
    int counter_para;
    int need_para;
    volatile char node_finish;
};

#define SHM_PARA SMPT
#define CVM_PARA CVMPT
#define NODE_HEAD() \
        struct node_head_str node_head;

struct str_func_call_node
{
    char mode;\
    int shmid;\
    int thread_idx;\
    void* pt_node;\
};

extern struct process recursion_parallel_process;
extern process_event_t event_recursion_calling;
extern process_event_t event_recursion_return;
extern process_event_t event_recursion_exit;
extern int busy_flag;
extern int g_FuncEnable;
extern int shm_id_ark;
extern volatile struct str_ark_status* pt_ark_status;
extern int shm_base[MAXNUM_FUNCTIONS];
extern int gSemIdFunc[MAXNUM_FUNCTIONS];
extern char frame_func_return_flag[MAXNUM_CALL];
extern pthread_mutex_t frame_mutex[MAXNUM_FUNCTIONS];
extern int gShmIdFunc[MAXNUM_FUNCTIONS];
extern void * task_head_addr[MAXNUM_FUNCTIONS];
extern struct task_head_str* task_head_ark[MAXNUM_FUNCTIONS];
extern struct task_head_str* task_head_cell[MAXNUM_FUNCTIONS];
extern void *(*recursion_functions[MAXNUM_FUNCTIONS])(void *);
extern char *g_strFunc_Name[MAXNUM_FUNCTIONS];
extern char *g_strFunc_NameA[MAXNUM_FUNCTIONS];
extern int recursion_funcitons_running[MAXNUM_CALL];

int GetFuncListIdx(const char * strFuncName);
int BKDRHash(char *str);
int GetAvailArk();
#endif /* TASK_CTL_H_ */
