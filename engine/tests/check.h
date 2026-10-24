/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SEURAT_TEST_CHECK_H
#define SEURAT_TEST_CHECK_H

#include <math.h>
#include <stdio.h>

static int check_failures;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, \
                    #cond);                                                  \
            check_failures++;                                                \
        }                                                                    \
    } while (0)

#define CHECK_NEAR(actual, expected, tol)                                    \
    do {                                                                     \
        double a_ = (actual), e_ = (expected);                               \
        if (!(fabs(a_ - e_) <= (tol))) {                                     \
            fprintf(stderr, "%s:%d: %s = %.6f, expected %.6f (+-%g)\n",      \
                    __FILE__, __LINE__, #actual, a_, e_, (double)(tol));     \
            check_failures++;                                                \
        }                                                                    \
    } while (0)

#endif
