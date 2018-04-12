/**
 * <bitree.h>
 * @author: zhenhang<czhenhang@gmail.com>
 * @create_at:
 */

#ifndef BITREE_H_
#define BITREE_H_
#include "process.h"
#include "recursion_parallel.h"
TASK_NODE(bitree) {
    NODE_HEAD()
    int num;
    TASK_NODE(bitree) *lchild;
    TASK_NODE(bitree) *rchild;
} PBiTree;
extern int func_idx_bitree;

RECURSION_FUNCTION(bitree);
int root_func_bfs();
int end_func_bfs();
int bitree_create(TASK_NODE(bitree)** ppBiTree, int depth);
#endif /* BITREE_H_ */
