#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define P_EDGE 0.4 
#define P_LOOP 0.5  

#define CHECK_ALLOC(p)                                  \
    do {                                                \
        if ((p) == NULL) {                              \
            fprintf(stderr, "Ошибка: нет памяти\n");    \
            exit(EXIT_FAILURE);                         \
        }                                               \
    } while (0)

static double rnd01(void)
{
    return (double)rand() / ((double)RAND_MAX + 1.0);
}


void generate_adjacency(int n, int** a, int directed, double p_edge, double p_loop)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            a[i][j] = 0;

    for (int i = 0; i < n; i++)
        if (rnd01() < p_loop)
            a[i][i] = 1;

    if (directed) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (i != j && rnd01() < p_edge)
                    a[i][j] = 1;
    }
    else {
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (rnd01() < p_edge)
                    a[i][j] = a[j][i] = 1;
    }
}

void print_adjacency(int n, int** a)
{
    printf("     ");
    for (int j = 0; j < n; j++) printf("v%-3d", j + 1);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("v%-3d ", i + 1);
        for (int j = 0; j < n; j++) printf("%-4d", a[i][j]);
        printf("\n");
    }
}

int size_by_adjacency(int n, int** a, int directed)
{
    int s = 0;
    for (int i = 0; i < n; i++)
        for (int j = (directed ? 0 : i); j < n; j++)
            s += a[i][j];
    return s;
}

void degrees_by_adjacency(int n, int** a, int directed,
    int* din, int* dout, int* deg, int* loops)
{
    for (int i = 0; i < n; i++) {
        loops[i] = a[i][i];
        if (directed) {
            dout[i] = din[i] = 0;
            for (int j = 0; j < n; j++) {
                dout[i] += a[i][j];
                din[i] += a[j][i];
            }
            deg[i] = din[i] + dout[i];
        }
        else {
            din[i] = dout[i] = 0;
            deg[i] = 0;
            for (int j = 0; j < n; j++)
                if (j != i) deg[i] += a[i][j];
            deg[i] += 2 * a[i][i];
        }
    }
}


int** build_incidence(int n, int** a, int directed, int m, int* e1, int* e2)
{
    int cols = (m > 0) ? m : 1;

    int** b = (int**)malloc((size_t)n * sizeof(int*));
    CHECK_ALLOC(b);
    for (int i = 0; i < n; i++) {
        b[i] = (int*)calloc((size_t)cols, sizeof(int));
        CHECK_ALLOC(b[i]);
    }

    int k = 0;
    for (int i = 0; i < n; i++)
        for (int j = (directed ? 0 : i); j < n; j++)
            if (a[i][j] && k < m) {
                if (i == j) {
                    b[i][k] = 2;
                }
                else if (directed) {
                    b[i][k] = 1;
                    b[j][k] = -1;
                }
                else {
                    b[i][k] = 1;
                    b[j][k] = 1;
                }
                e1[k] = i;
                e2[k] = j;
                k++;
            }
    return b;
}

void print_incidence(int n, int m, int** b)
{
    if (m == 0) {
        printf("(матрица пуста: в графе нет рёбер)\n");
        return;
    }
    printf("     ");
    for (int k = 0; k < m; k++) printf("e%-3d", k + 1);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("v%-3d ", i + 1);
        for (int k = 0; k < m; k++) printf("%-4d", b[i][k]);
        printf("\n");
    }
}

int size_by_incidence(int m) { return m; }

void degrees_by_incidence(int n, int m, int** b, int directed,
    int* din, int* dout, int* deg, int* loops)
{
    for (int i = 0; i < n; i++) {
        din[i] = dout[i] = deg[i] = loops[i] = 0;
        for (int k = 0; k < m; k++) {
            int v = b[i][k];
            if (v == 2) loops[i]++;
            if (directed) {
                if (v == 1) dout[i]++;
                else if (v == -1) din[i]++;
                else if (v == 2) { dout[i]++; din[i]++; }
            }
            else {
                deg[i] += v;
            }
        }
        if (directed) deg[i] = din[i] + dout[i];
    }
}


void report(int n, int directed, const int* din, const int* dout,
    const int* deg, const int* loops)
{
    printf("Степени вершин:\n");
    for (int i = 0; i < n; i++) {
        if (directed)
            printf("  v%d: deg+ (исх.)=%d, deg- (вх.)=%d, deg=%d\n",
                i + 1, dout[i], din[i], deg[i]);
        else
            printf("  deg(v%d)=%d\n", i + 1, deg[i]);
    }

    int has_loops = 0;
    printf("Вершины с петлями:                  ");
    for (int i = 0; i < n; i++)
        if (loops[i]) {
            printf(has_loops ? ", v%d" : "v%d", i + 1);
            has_loops = 1;
        }
    if (!has_loops) printf("нет");
    printf("\n");

    const char* names[3] = {
        "Изолированные вершины (deg = 0):   ",
        "Концевые вершины      (deg = 1):   ",
        "Доминирующие вершины (смежны со всеми остальными): "
    };
    for (int t = 0; t < 3; t++) {
        printf("%s", names[t]);
        int found = 0;
        for (int i = 0; i < n; i++) {
            int ok = 0;
            if (t == 0) ok = (deg[i] == 0);
            else if (t == 1) ok = (deg[i] == 1);
            else {
                if (directed) ok = (deg[i] - 2 * loops[i] == 2 * (n - 1));
                else          ok = (deg[i] - 2 * loops[i] == n - 1);
            }
            if (ok) {
                printf(found ? ", v%d" : "v%d", i + 1);
                found = 1;
            }
        }
        if (!found) printf("нет");
        printf("\n");
    }
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    int type = 0;
    if (argc > 1) type = atoi(argv[1]);
    else {
        printf("Тип графа (1 - неориентированный, 2 - ориентированный): ");
        if (scanf("%d", &type) != 1) return 1;
    }
    if (type != 1 && type != 2) {
        printf("Тип должен быть 1 или 2\n");
        return 1;
    }
    int directed = (type == 2);

    int n = 0;
    if (argc > 2) n = atoi(argv[2]);
    else {
        printf("Число вершин n: ");
        if (scanf("%d", &n) != 1) return 1;
    }
    if (n < 1) {
        printf("n должно быть >= 1\n");
        return 1;
    }
    srand(argc > 3 ? (unsigned)atoi(argv[3]) : (unsigned)time(NULL));

    printf("\n %s граф, n = %d \n",
        directed ? "Ориентированный" : "Неориентированный", n, P_EDGE, P_LOOP);

    int** a = (int**)malloc((size_t)n * sizeof(int*));
    CHECK_ALLOC(a);
    for (int i = 0; i < n; i++) {
        a[i] = (int*)malloc((size_t)n * sizeof(int));
        CHECK_ALLOC(a[i]);
    }
    int* deg = (int*)malloc((size_t)n * sizeof(int));   CHECK_ALLOC(deg);
    int* din = (int*)malloc((size_t)n * sizeof(int));   CHECK_ALLOC(din);
    int* dout = (int*)malloc((size_t)n * sizeof(int));  CHECK_ALLOC(dout);
    int* loops = (int*)malloc((size_t)n * sizeof(int)); CHECK_ALLOC(loops);

    generate_adjacency(n, a, directed, P_EDGE, P_LOOP);

    printf("ЗАДАНИЕ 1. Матрица смежности\n");
    print_adjacency(n, a);
    int m = size_by_adjacency(n, a, directed);
    printf("\nРазмер графа |E(G)| = %d\n", m);
    degrees_by_adjacency(n, a, directed, din, dout, deg, loops);
    report(n, directed, din, dout, deg, loops);

    size_t edge_cnt = (m > 0) ? (size_t)m : 1;
    int* e1 = (int*)malloc(edge_cnt * sizeof(int)); CHECK_ALLOC(e1);
    int* e2 = (int*)malloc(edge_cnt * sizeof(int)); CHECK_ALLOC(e2);
    int** b = build_incidence(n, a, directed, m, e1, e2);

    printf("\nЗАДАНИЕ 2. Матрица инцидентности\n");
    print_incidence(n, m, b);
    if (m > 0) {
        printf("%s:", directed ? "Дуги" : "Рёбра");
        for (int k = 0; k < m; k++) {
            if (e1[k] == e2[k])
                printf(" e%d=(v%d,v%d)[петля];", k + 1, e1[k] + 1, e2[k] + 1);
            else if (directed)
                printf(" e%d=(v%d->v%d);", k + 1, e1[k] + 1, e2[k] + 1);
            else
                printf(" e%d=(v%d,v%d);", k + 1, e1[k] + 1, e2[k] + 1);
        }
        printf("\n");
    }
    printf("\nРазмер графа |E(G)| = %d\n", size_by_incidence(m));
    degrees_by_incidence(n, m, b, directed, din, dout, deg, loops);
    report(n, directed, din, dout, deg, loops);

    for (int i = 0; i < n; i++) {
        free(a[i]);
        free(b[i]);
    }
    free(a);
    free(b);
    free(deg);
    free(din);
    free(dout);
    free(loops);
    free(e1);
    free(e2);
    return 0;
}