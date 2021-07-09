// Solution to find minimum number of coins of some currency to achieve a target value
//
// Uses a queue to implement what is essentially a self-pruning breadth-first n-way tree search to find the total
// Is O(N * T) where N = number of coins in coin set, and T = total we are looking to minimise for
//
// Author: Stew Forster (stew675@gmail.com)
// Date: 9th July 2021

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//#define COUNT_COMPARES

#ifdef COUNT_COMPARES
static uint32_t compares = 0;
#define INC_COMPARES (compares++)
#define RESET_COMPARES (compares = 0)
#define PRINT_COMPARES printf("Solution took %u compares\n", compares)
#else
#define INC_COMPARES (0)
#define RESET_COMPARES (0)
#define PRINT_COMPARES (0)
#endif

static void
min_coins_to_total(uint32_t coins[], uint32_t n_coins, uint32_t target)
{
	// Use calloc 'cos using stack allocation can run us out of stack space easily
	uint32_t *totals = calloc(target + 1, sizeof(*totals));
	uint32_t *queue = calloc(target + 1, sizeof(*queue));

	if ((totals == NULL) || (queue == NULL)) {
		fprintf(stderr, "Line %d in %s:%s(): Out of memory\n", __LINE__, __FILE__, __func__);
		goto cleanup;
	}

	RESET_COMPARES;

	for (uint32_t queue_pos = 0, queue_max = 1; queue_pos < queue_max; queue_pos++) {
		for (uint32_t total, qpt = queue[queue_pos], c = 0; c < n_coins; c++) {
			INC_COMPARES;
			if ((total = qpt + coins[c]) <= target) {
				if (totals[total] == 0) {
					totals[total] = coins[c];
					queue[queue_max++] = total;
				}
				// Short-circuit out of the loops early if we've hit the target
				(total == target) && (queue_pos = queue_max) && (c = n_coins);
			}
		}
	}

	if (totals[target] == 0) {
		printf("No possible set of coins makes the target of %u\n", target);
	} else {
		for (uint32_t total = target; total > 0; total -= totals[total]) {
			printf("%u ", totals[total]);
		}
		printf("= %u\n", target);
	}

	PRINT_COMPARES;

cleanup:
	totals ? free(totals) : 0;
	queue ? free(queue) : 0;
} // min_coins_to_total


void
main(int argc, char *argv[])
{
	uint32_t target, coins[] = {1, 2, 5, 10, 20, 50, 100, 200};	// Australian coin currency

	if (argc != 2) {
		printf("Usage: %s target\n", argv[0]);
		return;
	}

	target = (uint32_t)atoi(argv[1]);

	if (target < 1) {
		fprintf(stderr, "Error: target must be a positive number\n");
		return;
	}

	min_coins_to_total(coins, sizeof(coins) / sizeof(*coins), target);
} // main
