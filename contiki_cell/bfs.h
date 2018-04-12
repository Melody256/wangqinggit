/**
 * <bfs.h>
 * @author: zhenhang<czhenhang@gmail.com>
 * @create_at:
 */

#ifndef BFS_H_
#define BFS_H_
#define GRAPH_SIZE 7
#include "process.h"
#include "recursion_parallel.h"
TASK_NODE(bfs) {
    NODE_HEAD()
    int *graph_node;
    int *matrix;
    int *visited_node;// yi jing zhan kai guo de jie dian s
    int cur_node;
};
extern int func_idx_bfs;

RECURSION_FUNCTION(bfs);
int root_func_bfs();
int end_func_bfs();
#endif /* BFS_H_ */