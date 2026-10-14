#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define CHECK_ALLOC(p)                                  \
    do {                                                \
        if ((p) == NULL) {                              \
            fprintf(stderr, "Ошибка: нет памяти\n");    \
            exit(EXIT_FAILURE);                         \
        }                                               \
    } while (0)

void generate_adjacency(int n, int** a, double p)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            a[i][j] = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if ((double)rand() / RAND_MAX < p)
                a[i][j] = a[j][i] = 1;
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

int size_by_adjacency(int n, int** a)
{
    int s = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            s += a[i][j];
    return s / 2;
}

void degrees_by_adjacency(int n, int** a, int* deg)
{
    for (int i = 0; i < n; i++) {
        deg[i] = 0;
        for (int j = 0; j < n; j++) deg[i] += a[i][j];
    }
}

int** build_incidence(int n, int** a, int m, int* e1, int* e2)
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
        for (int j = i + 1; j < n; j++)
            if (a[i][j] && k < m) {
                b[i][k] = 1;
                b[j][k] = 1;
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

void degrees_by_incidence(int n, int m, int** b, int* deg)
{
    for (int i = 0; i < n; i++) {
        deg[i] = 0;
        for (int k = 0; k < m; k++) deg[i] += b[i][k];
    }
}

void report(int n, const int* deg)
{
    printf("Степени вершин:");
    for (int i = 0; i < n; i++) printf(" deg(v%d)=%d;", i + 1, deg[i]);
    printf("\n");

    const char* names[3] = {
        "Изолированные вершины (deg = 0):   ",
        "Концевые вершины      (deg = 1):   ",
        "Доминирующие вершины  (deg = n-1): "
    };
    for (int t = 0; t < 3; t++) {
        printf("%s", names[t]);
        int found = 0;
        for (int i = 0; i < n; i++) {
            int ok = (t == 0 && deg[i] == 0) ||
                (t == 1 && deg[i] == 1) ||
                (t == 2 && deg[i] == n - 1);
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

    int n = 0;
    if (argc > 1) n = atoi(argv[1]);
    else {
        printf("Число вершин n: ");
        if (scanf("%d", &n) != 1) return 1;
    }
    if (n < 1) {
        printf("n должно быть >= 1\n");
        return 1;
    }
    srand(argc > 2 ? (unsigned)atoi(argv[2]) : (unsigned)time(NULL));

    int** a = (int**)malloc((size_t)n * sizeof(int*));
    CHECK_ALLOC(a);
    for (int i = 0; i < n; i++) {
        a[i] = (int*)malloc((size_t)n * sizeof(int));
        CHECK_ALLOC(a[i]);
    }
    int* deg = (int*)malloc((size_t)n * sizeof(int));
    CHECK_ALLOC(deg);

    generate_adjacency(n, a, 0.4);

    printf("ЗАДАНИЕ 1. Матрица смежности\n");
    print_adjacency(n, a);
    int m = size_by_adjacency(n, a);
    printf("\nРазмер графа |E(G)| = %d\n", m);
    degrees_by_adjacency(n, a, deg);
    report(n, deg);

    size_t edge_cnt = (m > 0) ? (size_t)m : 1;
    int* e1 = (int*)malloc(edge_cnt * sizeof(int));
    CHECK_ALLOC(e1);
    int* e2 = (int*)malloc(edge_cnt * sizeof(int));
    CHECK_ALLOC(e2);
    int** b = build_incidence(n, a, m, e1, e2);

    printf("ЗАДАНИЕ 2. Матрица инцидентности\n");
    print_incidence(n, m, b);
    if (m > 0) {
        printf("Рёбра:");
        for (int k = 0; k < m; k++)
            printf(" e%d=(v%d,v%d);", k + 1, e1[k] + 1, e2[k] + 1);
        printf("\n");
    }
    printf("\nРазмер графа |E(G)| = %d\n", size_by_incidence(m));
    degrees_by_incidence(n, m, b, deg);
    report(n, deg);

    for (int i = 0; i < n; i++) {
        free(a[i]);
        free(b[i]);
    }
    free(a);
    free(b);
    free(deg);
    free(e1);
    free(e2);
    return 0;
}