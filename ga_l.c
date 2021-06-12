#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

// const による定数定義
const int width = 70;
const int height = 40;
const int max_cities = 1000;


// 町の構造体（今回は2次元座標）を定義
typedef struct
{
    int x;
    int y;
} City;

// 整数最大値をとる関数
int max(const int a, const int b)
{
    return (a > b) ? a : b;
}

// 以下は実装が長くなるのでプロトタイプ宣言
// draw_line: 町の間を線で結ぶ
// draw_route: routeでの巡回順を元に移動経路を線で結ぶ
// plot_cities: 描画する
// distance: 2地点間の距離を計算
// solove(): 巡回問題をといて距離を返す/ 引数route に巡回順を格納

void draw_line(char **map, City a, City b);
void draw_route(char **map, City *city, int n, const int *route);
void plot_cities(FILE* fp, char **map, City *city, int n, const int *route);
double distance( City a, City b);
double solve(City *city, int n, int *route, int **gen_routes);
double search(City *city, int n, int *route, double dist, int preshuffle1, int preshuffle2);

void ox(int i,int n,int *temp_route,int selected[20][n]);
void cx(int i,int n,int *temp_route,int selected[20][n]);
void px(int i,int n,int *temp_route,int selected[20][n]);
void move(int i,int n,int *temp_route);
void swap(int i,int n,int *temp_route);
void translocate(int i,int n,int *temp_route);

int main(int argc, char**argv)
{
    FILE *fp = stdout; // とりあえず描画先は標準出力としておく
    int n;
    if (argc != 2){ // デフォルトで都市数20
        n = 20;
    } else {
        n = atoi(argv[1]);
    }
    assert( n > 1 && n <= max_cities); // さすがに都市数1000は厳しいので

    City *city = (City*) malloc(n * sizeof(City));
    char **map = (char**) malloc(width * sizeof(char*));
    char *tmp = (char*)malloc(width*height*sizeof(char));
    for (int i = 0 ; i < width ; i++)
        map[i] = tmp + i * height;

    /* 町の座標をランダムに割り当て */
    for (int i = 0 ; i < n ; i++){
        city[i].x = rand() % (width - 5);
        city[i].y = rand() % height;
    }
    // 町の初期配置を表示
    plot_cities(fp, map, city, n, NULL);
    sleep(1);

    srand((unsigned) time(NULL));
    // 訪れる順序を記録する配列を設定
    int *route = (int*)calloc(n, sizeof(int));
    int **gen_routes = malloc(100*sizeof(int*));
    int *base_routes = (int*)malloc(100*n*sizeof(int));
    for (int i = 0; i < 100; i++) {
        gen_routes[i] = base_routes + n * i;
        for (int j = 0; j < n; j++) {
            gen_routes[i][j] = j;
        }
        for (int j = 1; j < n; j++) { // ランダムシャッフル(0番目は固定)
                int k = rand() % (n - 1) + 1;
                int t = gen_routes[i][j];
                gen_routes[i][j] = gen_routes[i][k];
                gen_routes[i][k] = t;
        }
    }
    double dold = INFINITY;

    long time1 = time(NULL);

    for (int i = 0; i < 10000; i++) {
        const double d = solve(city, n, route, gen_routes);
        if (i % 1000 == 0) {
            printf("generation = %d,total distance = %f\n", i,d);
        }
        long time2 = time(NULL);
        if (dold != d) {
            printf("generation = %d,total distance = %f,time = %ld\n", i,d,time2-time1);
        }
        dold = d;
        // for (int j = 0; j < n; j++) {
            // printf("%d",route[j]);
        // }
        // printf("\n");
        //plot_cities(fp, map, city, n, route);
    }
    
    // const double d = solve(city, n, route, gen_routes);
    // printf("total distance = %f\n", d);
    plot_cities(fp, map, city, n, route);

    // 動的確保した環境ではfreeをする
    free(route);
    free(map[0]);
    free(map);
    free(city);
    free(base_routes);
    free(gen_routes)
;    
    return 0;
}

int compare_int(const void *a, const void *b)
{
        return *(int*)a - *(int*)b;
}

// 繋がっている都市間に線を引く
void draw_line(char **map, City a, City b)
{
    const int n = max(abs(a.x - b.x), abs(a.y - b.y));
    for (int i = 1 ; i <= n ; i++){
        const int x = a.x + i * (b.x - a.x) / n;
        const int y = a.y + i * (b.y - a.y) / n;
        if (map[x][y] == ' ') map[x][y] = '*';
    }
}

void draw_route(char **map, City *city, int n, const int *route)
{
    if (route == NULL) return;

    for (int i = 0; i < n; i++) {
        const int c0 = route[i];
        const int c1 = route[(i+1)%n];// n は 0に戻る必要あり
        draw_line(map, city[c0], city[c1]);
    }
}

void plot_cities(FILE *fp, char **map, City *city, int n, const int *route)
{
    fprintf(fp, "----------\n");

    memset(map[0], ' ', width * height); // グローバルで定義されたconst を使用

    // 町のみ番号付きでプロットする
    for (int i = 0; i < n; i++) {
        char buf[100];
        sprintf(buf, "C_%d", i);
        for (int j = 0; j < strlen(buf); j++) {
            const int x = city[i].x + j;
            const int y = city[i].y;
            map[x][y] = buf[j];
        }
    }

    draw_route(map, city, n, route);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const char c = map[x][y];
            fputc(c, fp);
        }
        fputc('\n', fp);
    }
    fflush(fp);
}

double distance(City a, City b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

double solve(City *city, int n , int *route, int **gen_routes)
{
    double min_d = INFINITY; // 距離の最小値(仮)
    double valuesum = 0.0;
    double *roulette = (double*)calloc(100, sizeof(double));
    int temp_route[n];
    // 適合度計算
    for (int i = 0; i < 100; i++) { // 一世代当たりの標本数(初期解生成数)100
        // 距離計算
        double sum_d = 0;
        for (int j = 0 ; j < n ; j++){
            const int c0 = gen_routes[i][j];
            const int c1 = gen_routes[i][(j+1)%n]; // nは0に戻る
            sum_d += distance(city[c0],city[c1]);
            temp_route[j] = gen_routes[i][j];
        }
        sum_d = search(city,n,temp_route,sum_d,0,0);
        for (int j = 0 ; j < n ; j++){
            gen_routes[i][j] = temp_route[j];
        }
        double value = 1.0 / (sum_d * sum_d * sum_d); // 評価値は山登り法で求めた局所最短距離の逆数の3乗とする
        valuesum += value;
        roulette[i] = valuesum; // ルーレットの境界を設定
        if (min_d > sum_d) { // 最小値の更新処理
            min_d = sum_d;
            // routeを更新
            for (int l = 0; l < n; l++) {
                route[l] = gen_routes[i][l];
            }
        }
    }
    // 選択
    int selected[20][n];
    for (int i = 0; i < 20; i++) {
        double random = (double)rand() / RAND_MAX * valuesum;
        int num = 0;
        while (random > roulette[num]) {
            num++;
        }
        for (int j = 0; j < n; j++) {
            selected[i][j] = gen_routes[num][j];
            // printf("%d ",selected[i][j]);
        }
        // printf("\n");
    }
    // 生殖
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 5; j++) {
            // printf("%d %d starti\n",i,j);
            for (int k = 0; k < n; k++) {
                temp_route[k] = selected[i][k];
                // printf("%d\n",temp_route[k]);
            }
            // 突然変異(1%)　位置移動or交換or転座
            if (rand() % 100 == 0) {
                int y = rand();
                if (y % 3 == 0) {
                    move(i,n,temp_route);
                } else if (y % 3 == 1) {
                    swap(i,n,temp_route);
                } else {
                    translocate(i,n,temp_route);
                }
                
            }
            // 交叉(80%) 順序交叉or循環交叉or部分的交叉
            if (rand() % 10 < 8) {
                int x = rand();
                if (x % 3 == 0) {
                    // printf("cx\n");
                    cx(i,n,temp_route,selected);
                    // printf("end\n");
                } else if (x % 3 == 1) {
                    // printf("ox\n");
                    ox(i,n,temp_route,selected);
                    // printf("end\n");
                } else {
                    // printf("px\n");
                    px(i,n,temp_route,selected);
                    // printf("end\n");
                }
            }
            // printf("\n");
            for (int k = 0; k < n; k++) {
                gen_routes[i*5+j][k] = temp_route[k];
                // printf("%d\n",temp_route[k]);
            }
        }
    }
    // 最高のものを残す
    for (int k = 0; k < n; k++) {
        gen_routes[99][k] = route[k];
        // printf("%d\n",route[k]);
    }
    return min_d; // 最高成績(最短距離)
}

// 順序交叉(Order Crossover: OX)
void ox(int i,int n,int *temp_route,int selected[20][n]) {
    int temp2_route[n];
    int other = rand() % 20;
    while (i == other) {
        other = rand() % 20;
    }
    // printf("other = %d\n",other);
    int other_route[n];
    for (int k = 0; k < n; k++) {
        other_route[k] = selected[other][k];
        // printf("%d %d\n",temp_route[k],other_route[k]);
    }
    int point = rand() % (n - 1) + 1;
    int point2 = rand() % (n - 1) + 1;
    while (point2 == point) {
        point2 = rand() % (n - 1) + 1;
    }
    if (point > point2) {
        int t = point;
        point = point2;
        point2 = t;
    }
    int temp[point2-point+1];
    int pos = 1;
    int tpos = 0;
    while (pos < n) {
        bool unfilled = true;
        for (int k = 0; k < point2-point+1; k++) {
            if (other_route[pos] == temp_route[point+k]) {
                unfilled = false;
                break;
            }
        }
        if (unfilled) {
            temp[tpos] = other_route[pos];
            tpos++;
        }
        pos++;
    }
    // printf("checked\n");
    temp2_route[0] = 0;
    for (int k = 1; k < point; k++) {
        temp2_route[k] = temp[k-1];
    }
    for (int k = point; k < point2+1; k++) {
        temp2_route[k] = temp_route[k];
    }
    for (int k = 0; k < (n-point2-1); k++) {
        temp2_route[point2+k+1] = temp[point-1+k];
    }
    for (int k = 0; k < n; k++) {
        temp_route[k] = temp2_route[k];
        // printf("%d\n",temp_route[k]);
    }
}

// 循環交叉(Cycle Crossover: CX)
void cx(int i,int n,int *temp_route,int selected[20][n]) {
    int temp2_route[n];
    int other = rand() % 20;
    while (i == other) {
        other = rand() % 20;
    }
    // printf("other = %d\n",other);
    int other_route[n];
    for (int k = 0; k < n; k++) {
        other_route[k] = selected[other][k];
        // printf("%d %d\n",temp_route[k],other_route[k]);
    }
    for (int k = 0; k < n; k++) {
        temp2_route[k] = temp_route[k];
    }
    int point = rand() % (n - 1) + 1;
    int q[n];
    q[0] = other_route[point];
    temp2_route[point] = other_route[point];
    // printf("point = %d\n",point);
    for (int i = 1; i < n; i++) {
        int pos = 0;
        while (q[i-1] != temp_route[pos]) {
            pos++;
        }
        // printf("pos = %d\n",pos);
        q[i] = other_route[pos];
        temp2_route[pos] = other_route[pos];
        if (q[0] == q[i]) {
            break;
        }
    }
    for (int k = 0; k < n; k++) {
        temp_route[k] = temp2_route[k];
    }
}

// 部分的交叉(Partial Crossover: PX)
void px(int i,int n,int *temp_route,int selected[20][n]) {
    int temp2_route[n];
    int other = rand() % 20;
    while (i == other) {
        other = rand() % 20;
    }
    // printf("other = %d\n",other);
    int other_route[n];
    for (int k = 0; k < n; k++) {
        other_route[k] = selected[other][k];
        // printf("%d %d\n",temp_route[k],other_route[k]);
    }
    for (int k = 0; k < n; k++) {
        temp2_route[k] = temp_route[k];
    }
    int times = rand() % (n - 2) + 1;
    int a,b,point,pos;
    for (int i = 0; i < times; i++) {
        point = rand() % (n - 1) + 1;
        a = temp2_route[point];
        b = other_route[point];
        // printf("point = %d\n",point);
        pos = 0;
        while (temp2_route[pos] != b) {
            pos++;    
        }
        temp2_route[pos] = a;
        temp2_route[point] = b;
    }
    for (int k = 0; k < n; k++) {
        temp_route[k] = temp2_route[k];
    }
}

// 位置移動
void move(int i,int n,int *temp_route) {
    int temp2_route[n];
    int point = rand() % (n - 1) + 1;
    int point2 = rand() % (n - 1) + 1;
    while (point2 == point) {
        point2 = rand() % (n - 1) + 1;
    }
    int t = temp_route[point];
    int pos = 0;
    while (pos < n) {
        if (point2 > point) {
            if (point <= pos && pos < point2-1) {
                temp2_route[pos] = temp_route[pos+1];
            } else if (point2 -1 == pos) {
                temp2_route[pos] = t;
            } else {
                temp2_route[pos] = temp_route[pos];
            }
        } else {
            if (point2 == pos) {
                temp2_route[pos] = t;
            } else if (point2 < pos && pos <= point) {
                temp2_route[pos] = temp_route[pos-1];
            } else {
                temp2_route[pos] = temp_route[pos];
            }
        }
        pos++;
    }
    for (int k = 0; k < n; k++) {
        temp_route[k] = temp2_route[k];
    }
}

// 交換
void swap(int i,int n,int *temp_route) {
    int temp2_route[n];
    int point = rand() % (n - 4) + 1;
    int point2 = rand() % (n - point - 3) + point + 1;
    int point3 = rand() % (n - point2 - 2) + point2 + 1;
    int point4 = rand() % (n - point3 - 1) + point3 + 1;
    // printf("%d %d %d %d\n",point,point2,point3,point4);
    for (int k = 0; k < n; k++) {
        temp2_route[k] = temp_route[k];
    }
    int front = point2 - point + 1;
    int back = point4 - point3 + 1;
    for (int k = 0; k < back; k++) {
        temp2_route[point+k] = temp_route[point3+k];
    }
    for (int k = 0; k < point3 - point2 - 1; k++) {
        temp2_route[point+back+k] = temp_route[point2+k+1];
    }
    for (int k = 0; k < front; k++) {
        temp2_route[point3+back-front+k] = temp_route[point+k];
    }
    for (int k = 0; k < n; k++) {
        temp_route[k] = temp2_route[k];
    }
}

// 転座
void translocate(int i,int n,int *temp_route) {
    int temp2_route[n];
    int point = rand() % (n - 2) + 1;
    int point2 = rand() % (n - point - 1) + point + 1;
    int size = point2 - point + 1;
    int point3 = rand() % (n - size) + 1;
    // printf("before");
    // while (point == point3) {
    //     point3 = rand() % (n - size) + 1;
    // }
    // printf("%d %d %d\n",point,point2,point3);
    for (int k = 0; k < n; k++) {
        temp2_route[k] = temp_route[k];
    }
    for (int k = 0; k < size; k++) {
        temp2_route[point3+k] = temp_route[point+k];
    }
    for (int k = 1; k < point3; k++) {
        if (size + k < n && point <= k) {
            temp2_route[k] = temp_route[k+size];
        }
    }
    for (int k = point3 + size; k < n; k++) {
        if (k - size < point2 && point2 >= k) {
            temp2_route[k] = temp_route[k-size];
        }
    }
    for (int k = 0; k < n; k++) {
        temp_route[k] = temp2_route[k];
    }
}

// 山登り法探索
double search(City *city, int n , int *route, double dist, int preshuffle1, int preshuffle2) {
  for (int i = 1; i < n; i++) { // 次入れ替えるもの(0番目は不変にするので1番目から)
    for (int j = 1; j < i; j++) { // 次入れ替えるもの(0番目は不変にするので1番目から)
      if (preshuffle1 != i || preshuffle2 != j) { // 前交換した組ではないなら
        // 交換
        int t = route[i];
        route[i] = route[j];
        route[j] = t;
        // 距離計算
        double sum_d = 0;
        for (int i = 0 ; i < n ; i++){
            const int c0 = route[i];
            const int c1 = route[(i+1)%n]; // nは0に戻る
            sum_d += distance(city[c0],city[c1]);
        }
        if (dist > sum_d) { // 今計算したほうの距離が短いなら
            dist = sum_d; // 最短距離を更新
            return search(city,n,route,dist,i,j); // また探す
        } else { // 等しいもしくは長いなら(更新できないなら)
            // 交換(元に戻す)
            int t = route[j];
            route[j] = route[i];
            route[i] = t;
        }
      }
    }
  }
  return dist; // ここまでくると更新できなかったので現時点の最短距離を返す
}