/*已知最小颜色数量的情况下算出来方案,并计时*/
#include<bits/stdc++.h>

using namespace std;
#define INF INT_MAX;

int N;//节点数
int Graph[1000][1000];//邻接图 ,Graph[i][j] 表示节点i的第j个邻接节点
int n_edges[1000];//每个节点的相邻节点数量

//禁忌搜索信息
int sol[1000];//每个节点的颜色
int f; //目标函数
int K; //颜色数
int adj_color_v[1000][100];//冲突值计算表,N*K 暂定颜色数量不超100种
int tabuList[1000][100]; //禁忌表,N*K
int iter_delt; //一轮选择的变化值
int tabu_delt; //存放可能成为禁忌解的变化值
int best_f; //历史最小f
int iter; //迭代轮数
int solution[20000][2]; //非禁忌的解
int tabuSolution[20000][2]; //禁忌的解
int iterSolution[2]; //每次迭代选择的解

// 分割字符串
void split(const string &s, vector<string> &elems, char delim = ' ') {
    elems.clear(); //注意加这句
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

//初始化
void Init(string filepath){
    srand(time(0));
    memset(tabuList,0,sizeof(tabuList));
    memset(adj_color_v,0,sizeof(adj_color_v));
    memset(n_edges,0,sizeof(n_edges));

    ifstream infile(filepath.c_str());
    if(infile.fail()) cout<<"Open File Failed!!"<<endl;
    vector<string> s_list;
    string line;
    int start_read = false;
    int n1,n2,tmpI;
    while(!infile.fail()) //读取图 
        {   
            getline(infile,line);
            if(start_read && line.find("e",0)!=string::npos){
                    split(line,s_list);
                    n1 = stoi(s_list[1])-1;
                    n2 = stoi(s_list[2])-1;
                    n_edges[n1]++;
                    tmpI = n_edges[n1];
                    Graph[n1][tmpI-1] = n2;
                    n_edges[n2]++;
                    tmpI = n_edges[n2];
                    Graph[n2][tmpI-1] = n1;
            }
            else if(line.find("p edge",0)!= string::npos){
                split(line,s_list);
                N =stoi(s_list[2]); 
                start_read = true;
            }
        }
    infile.close(); 

    //随机生成初始值,计算初始冲突值表
    for(int i=0;i<N;i++){
        sol[i] = rand() % K;
    }
    int cur_color;
    int nei_color;
    int nei_num;
    for(int i=0;i<N;i++){
        cur_color = sol[i];
        nei_num = n_edges[i];
        for(int j=0;j<nei_num;j++){
            nei_color = sol[Graph[i][j]];
            if(cur_color == nei_color) f++;
            adj_color_v[i][nei_color]++;
        }
    }
    f = f/2;
    best_f = f;
    cout<<"init f is: "<<f <<endl;
}


//找最佳禁忌或者非禁忌移动
void FindMove() {
    iter_delt = INF;
    int tabu_delt = INF;
    int tmp_delt;
    int count = 0, tabu_count = 0;
    int cur_color; //当前结点颜色
    int cur_value; //当前结点颜色表的值
    for (int i = 0; i<N; i++) {
        cur_color = sol[i];
        cur_value = adj_color_v[i][cur_color];
        if (adj_color_v[i][cur_color] > 0) { //当前节点当前颜色冲突值大于0才考虑
            for (int j = 0; j < K; j++) {
                if (cur_color != j) {
                    tmp_delt = adj_color_v[i][j] - cur_value;
                    if( tabuList[i][j] <= iter){    //非禁忌移动,取下一代最优
                        if(tmp_delt < iter_delt){
                            iter_delt = tmp_delt;
                            count = 1;
                            solution[count -1][0] = i;
                            solution[count -1][1] = j;
                        }else if(tmp_delt == iter_delt){
                            count++;
                            solution[count -1][0] = i;
                            solution[count -1][1] = j;
                        }
                    }
                    else { //禁忌步数不小于iter,但可能成为禁忌解,存放在禁忌候选表中。
                        if(tmp_delt < tabu_delt){
                            tabu_delt = tmp_delt;
                            tabu_count = 1;
                            tabuSolution[tabu_count-1][0] = i; 
                            tabuSolution[tabu_count-1][1] = j;
                        }else if(tmp_delt == tabu_delt){
                            tabu_count ++;
                            tabuSolution[tabu_count-1][0] = i;
                            tabuSolution[tabu_count-1][1] = j;
                        }
                    } 
                }
            }
        }
    }


    int random = 0;
    if((tabu_delt+f) < best_f && tabu_delt < iter_delt) {
        iter_delt = tabu_delt;
        random = rand() % tabu_count;//以1/n概率选择
        iterSolution[0] = tabuSolution[random][0];
        iterSolution[1] = tabuSolution[random][1];
    }
    else {
        random = rand() % count;//进行非禁忌移动
        iterSolution[0] = solution[random][0];
        iterSolution[1] = solution[random][1];
    }
}


//选择了一轮的最优后更新
void MakeMove(){
    f = iter_delt +f;
    if(f < best_f) {
        best_f = f;
    }
    int node = iterSolution[0], color = iterSolution[1];
    int old_color = sol[node];
    assert(old_color != color); 
    sol[node] = color;
    tabuList[node][old_color] = iter + f +rand() %10 +1 ;//禁忌步数公式
    int nei_num = n_edges[node];
    int adj_node;
    for(int i=0;i<nei_num;i++) { //相邻的节点更新冲突值
        adj_node = Graph[node][i];
        adj_color_v[adj_node][old_color]--;
        adj_color_v[adj_node][color]++;
    }
    
}


//禁忌搜索
void tabuSearch(){
    int Not_improve = 80000000,tmp_step = 0;
    double start,end,cost_time;
    iter = 0;
    start = clock();
    while(f>0){
        iter++;
        FindMove();
        if( best_f <= f){
            tmp_step++;
            if(tmp_step > Not_improve){
                cout<<"can not find a solution, please try again"<<endl;
                break;
            }   
        }
        MakeMove();
        if ((iter % 500000) == 0) cout << "iter: "<< iter << "  f:  " << f << "  bsetf:  " << best_f <<"  K:  "<<K<< endl;
    }
    end = clock();
    cout<<start<<endl<<end<<endl;
    cost_time = (double(end - start)) / CLOCKS_PER_SEC;
	cout<<" f "<<f<<" Iter "<<iter<<" cost_time "<<cost_time<<endl;
    cout<<"SOLUTION:"<<endl;
    for(int i=0;i<N;i++)
       cout<<sol[i]<<" ";
    iter = 0;
}


int main() {
    int CASE;
    int colorNum;
    string absolute="E:\\tool\\CodeBlocks16\\2020_algorithm\\tabu_search\\Graph-coloring\\tabucol-master-senior\\data\\";
    string filename[12] = {"DSJC125.1.col", "DSJC125.5.col", "DSJC125.9.col", "DSJC250.1.col", "DSJC250.5.col","DSJC250.9.col","DSJC500.1.col","DSJC500.5.col","DSJC500.9.col","DSJC1000.1.col","DSJC1000.5.col","DSJC1000.9.col"};
    cout<<"Enter the numbers as follows:"<<endl;
    cout<<"(0)DSJC125.1.col  (1)DSJC125.5.col   (2)DSJC125.9.col   (3)DSJC250.1.col  (4)DSJC250.5.col  (5)DSJC250.9.col  (6)DSJC500.1.col  (7)DSJC500.5.col"<<endl;
    cout<<"(8)DSJC500.9.col  (9)DSJC1000.1.col  (10)DSJC1000.5.col (11)DSJC1000.9.col "<<endl;
    while (cin >> CASE && CASE < 12 && CASE > -1)
    {
        string relative = filename[CASE];
        string filepath = absolute+relative;
        cout<<"Input sum of colors:"<<endl;
        cin >>colorNum;
        K = colorNum;
        Init(filepath);
        tabuSearch();
    }
    return 0;
}

