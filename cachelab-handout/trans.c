/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int m, n, i, t0, t1, t2, t3, t4, t5, t6, t7;
    if (M == 32 && N == 32) {
        for (m = 0; m < M; m += 8) {
            for (n = 0; n < N; n += 8) {
                for (i = n; i < n + 8; i++) {
                    t0 = A[i][m];
                    t1 = A[i][m + 1];
                    t2 = A[i][m + 2];
                    t3 = A[i][m + 3];
                    t4 = A[i][m + 4];
                    t5 = A[i][m + 5];
                    t6 = A[i][m + 6];
                    t7 = A[i][m + 7];

                    B[m][i] = t0;
                    B[m + 1][i] = t1;
                    B[m + 2][i] = t2;
                    B[m + 3][i] = t3;
                    B[m + 4][i] = t4;
                    B[m + 5][i] = t5;
                    B[m + 6][i] = t6;
                    B[m + 7][i] = t7;
                }
            }
        }
    }

    if (M == 64 && N == 64) {
        for (m = 0; m < M; m += 8) {
            for (n = 0; n < N; n += 8) {
                for (i = n; i < n + 4; i++) {
                    t0 = A[i][m];
                    t1 = A[i][m + 1];
                    t2 = A[i][m + 2];
                    t3 = A[i][m + 3];
                    t4 = A[i][m + 4];
                    t5 = A[i][m + 5];
                    t6 = A[i][m + 6];
                    t7 = A[i][m + 7];

                    B[m][i] = t0;
                    B[m][i + 4] = t4;
                    B[m + 1][i] = t1;
                    B[m + 1][i + 4] = t5;
                    B[m + 2][i] = t2;
                    B[m + 2][i + 4] = t6;
                    B[m + 3][i] = t3;
                    B[m + 3][i + 4] = t7;
                }

                for(i = m; i < m + 4; i++) {
                    t0 = B[i][n + 4];
                    t1 = B[i][n + 5];
                    t2 = B[i][n + 6];
                    t3 = B[i][n + 7];

                    B[i][n + 4] = A[n + 4][i];
                    B[i][n + 5] = A[n + 5][i];
                    B[i][n + 6] = A[n + 6][i];
                    B[i][n + 7] = A[n + 7][i];

                    B[i + 4][n + 4] = A[n + 4][i + 4];
                    B[i + 4][n + 5] = A[n + 5][i + 4];
                    B[i + 4][n + 6] = A[n + 6][i + 4];
                    B[i + 4][n + 7] = A[n + 7][i + 4];

                    B[i + 4][n] = t0;
                    B[i + 4][n + 1] = t1;
                    B[i + 4][n + 2] = t2;
                    B[i + 4][n + 3] = t3;
                }
            }
        }
    }

    if (M == 61 && N == 67) {
        for (m = 0; m < M; m += 8) {
            for (n = 0; n < N; n += 8) {
                for (i = n; i < n + 8 && i < N; i++) {
                    for (int j = m; j < m + 8 && j < M; j++) {
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

