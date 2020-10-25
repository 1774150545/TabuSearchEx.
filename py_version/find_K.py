import random
import time

INF = float("inf");

global N  # 节点数
global K  # 颜色数
global Graph  # 邻接图 graph[i][j] 节点i的第j个邻接节点
global n_edges  # 每个节点的相邻节点数量

# 禁忌搜索信息
global sol   # 每个节点的颜色
global tabuList   # 禁忌表 N*K
global f  # 目标函数
global best_f  # 历史最小f
global adj_color_v  # 冲突值表 N*K
global iterI
global iter_delt  # 每轮选择的更新对应delt值
tmp_delt = INF  # 临时delt
tabu_delt = INF
global solution   # 非禁忌移动待选表
global tabuSolution  # 禁忌移动待选表
global iterSolution  # 每轮选择的移动



def Init(filepath):
    # todo 这里要清空数组，必须把之前的数组定义为global 因为改变的是数组本身n_edges 而不是下标n_edges[0]
    # 在寻找K值的时候，注意每次数组一定要清零！
    global Graph,  n_edges, sol, tabuList, adj_color_v, solution, tabuSolution, iterSolution
    Graph  = [[0] * 1000 for i in range(1000)]  # 邻接图 graph[i][j] 节点i的第j个邻接节点
    n_edges = [0 for i in range(1000)]
    sol = [0 for i in range(1000)]
    tabuList = [[0] * 100 for i in range(1000)]
    adj_color_v = [[0] * 100 for i in range(1000)]
    solution = [[0, 0] for i in range(20000)]
    tabuSolution = [[0, 0] for i in range(20000)]
    iterSolution = [0, 0]

    global f, best_f, N
    start_flag = False
    f = 0
    best_f = 0
    with open(filepath) as file:
        for line in file:
            if (start_flag and line.startswith("e")):
                s_list = line.split()
                n1 = int(s_list[1]) - 1
                n2 = int(s_list[2]) - 1
                n_edges[n1] += 1
                tmpI = n_edges[n1]
                Graph[n1][tmpI - 1] = n2
                n_edges[n2] += 1
                tmpI = n_edges[n2]
                Graph[n2][tmpI - 1] = n1
            elif (line.startswith("p edge")):
                s_list = line.split()
                N = int(s_list[2])
                start_flag = True

    # 初始化adj_color_v 和方案sol
    for i in range(N):
        sol[i] = random.randint(0, K - 1)
    for i in range(N):  # 计算冲突表adj_color_v, 计算f
        cur_color = sol[i]
        for j in range(n_edges[i]):
            nei_color = sol[Graph[i][j]]
            adj_color_v[i][nei_color] += 1
            if cur_color == nei_color:
                f += 1
    f = f / 2
    best_f = f
    print("init f i is: ", f)


def FindMove():
    global iter_delt, iterSolution, f

    iterSolution = [0, 0]
    iter_delt = tabu_delt = tmp_delt = INF
    count = 0
    tabu_count = 0
    for i in range(N):  # adj_color_v 的行
        cur_color = sol[i]
        cur_value = adj_color_v[i][cur_color]
        if cur_value > 0:  # >0才考虑, 否则如果tmp_delt<0, 这种情况不会更新solution或者tabuSolution, 如果为tmp_delt==0，这种情况应该极少，而且换了没有意义
            # print("at least run", cur_value)
            for j in range(K):  # adj_color_v 的列
                if cur_color != j:
                    # 非禁忌移动
                    tmp_delt = adj_color_v[i][j] - cur_value  # 邻居节点更新完，所有邻居的冲突总数不变，delt来源是自己(节点i)的颜色改变
                    if tabuList[i][j] <= iterI: # todo 这里之前写错了，之前写的是tabuList[i][cur_color] <= iterI:
                        # 非禁忌移动
                        if tmp_delt < iter_delt:
                            iter_delt = tmp_delt
                            count = 1
                            solution[count - 1] = i, j
                        elif tmp_delt == iter_delt:
                            count += 1
                            solution[count - 1] = i, j
                    # 禁忌移动
                    else:
                        if tmp_delt < tabu_delt:
                            tabu_delt = tmp_delt
                            tabu_count = 1
                            tabuSolution[tabu_count - 1] = i, j
                        elif tmp_delt == tabu_delt:
                            tabu_count += 1
                            tabuSolution[tabu_count - 1] = i, j

    if tabu_delt + f < best_f and tabu_delt < iter_delt:  # 除了满足历史最优，还要比此轮非禁忌解优秀
        iter_delt = tabu_delt
        randI = random.randint(0, tabu_count - 1)
        iterSolution = tabuSolution[randI]
    else:  # 选择非禁忌解
        randI = random.randint(0, count - 1)
        iterSolution = solution[randI]

def MakeMove():
    global f, best_f
    # 更新best_f
    f += iter_delt
    if f < best_f:
        best_f = f
    # 根据 iterSolution 更新 adj_color_v(做法：node相邻的节点，oldcolor行-1，color行+1)
    node = iterSolution[0]
    color = iterSolution[1]
    nei_num = n_edges[node]
    old_color = sol[node]
    for j in range(nei_num):
        nei_node = Graph[node][j]
        adj_color_v[nei_node][old_color] -= 1
        adj_color_v[nei_node][color] += 1
    # 更新tabuList , 注意是old_color(意思是禁止换回去)
    tabuList[node][old_color] = iterI + random.randint(1, 10) + f  # 禁忌步数公式，实际的禁忌步数是 f+rand(10)
    # 更新颜色
    sol[node] = color



def try_tabu():
    global iterI
    Not_imporve = 100000000
    tmp_step = 0
    iterI = 0
    while f > 0:
        iterI += 1
        FindMove()
        if best_f <= f + iter_delt:
            tmp_step += 1
            if tmp_step % 5000000 == 0:
                print("not improve")
            if tmp_step > Not_imporve:
                print("can not find a solution please try again!\n")
                return False
        else:
            tmp_step = 0
        MakeMove()
        if (iterI % 500000 == 0):
            print("iter: ", iterI, "  f:  ", f, "  best_f:  ", best_f, "  K:  ", K)
    return True


def tabuSearch(filepath):
    global K
    while True:
        start = time.time()
        Init(filepath)
        flag = try_tabu()
        if not flag:
            break
        print("Avaliable K:", K)
        end = time.time()
        print("cost time: ", end - start)
        print("sum iter:", iterI, '\n\n')
        K -= 1
        print("  Try K = ", K, "......")
    print("Avaliable K: ", (++K))


if __name__ == "__main__":
    CASE = 0
    active = True
    absolute = "E:\\tool\\CodeBlocks16\\2020_algorithm\\tabu_search\\Graph-coloring\\data\\";
    filename = ["DSJC125.1.col", "DSJC125.5.col", "DSJC125.9.col", "DSJC250.1.col", "DSJC250.5.col",
                "DSJC250.9.col", "DSJC500.1.col", "DSJC500.5.col", "DSJC500.9.col", "DSJC1000.1.col",
                "DSJC1000.5.col", "DSJC1000.9.col"];
    print("Enter the numbers as follows:              (enter 'quit' to quit)")
    print(
        "(0)DSJC125.1.col  (1)DSJC125.5.col   (2)DSJC125.9.col   (3)DSJC250.1.col  (4)DSJC250.5.col  (5)DSJC250.9.col  (6)DSJC500.1.col  (7)DSJC500.5.col")
    print("(8)DSJC500.9.col  (9)DSJC1000.1.col  (10)DSJC1000.5.col (11)DSJC1000.9.col ")
    while True:
        CASE = input()
        if CASE == 'quit':
            break
        else:
            CASE = int(CASE)
            if CASE > -1 and CASE < 12:
                relative = filename[CASE]
                filepath = absolute + relative
                if (CASE == 0 or CASE == 3 or CASE == 6):
                    start_num = 30
                elif (CASE == 1 or CASE == 4 or CASE == 7):
                    start_num = 55
                else:
                    start_num = 100

                K = start_num
                tabuSearch(filepath)
