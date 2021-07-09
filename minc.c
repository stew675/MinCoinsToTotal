// Solution to find minimum number of coins of some currency to achieve a target value
//
// Uses a queue to implement what is essentially a self-pruning breadth-first n-way graph search to find the total
// Is worst-case O(N * T) where N = number of coins in coin set, and T = total we are looking to minimise for
// With the implemented pruning, the amortised case is typically much better than O(N * T)
//
// Author: Stew Forster (stew675@gmail.com)
// Date: 9th July 2021

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define COUNT_COMPARES

#ifdef COUNT_COMPARES
static uint32_t compares = 0;
#define INC_COMPARES (compares++)
#define RESET_COMPARES (compares = 0)
#define PRINT_COMPARES printf("\nSolution took %u compares\n", compares)
#else
#define INC_COMPARES (0)
#define RESET_COMPARES (0)
#define PRINT_COMPARES (0)
#endif

static int
int32_cmp(const void *a, const void *b)
{
	INC_COMPARES;
	return *((const int32_t *)a)  - *((const int32_t *)b);
} // int_cmp


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

	// Sorting the coin set into increasing order allows for search optimisations
	qsort(coins, n_coins, sizeof(coins[0]), int32_cmp);

	// Minimise the total search space where possible
	if (target < coins[n_coins - 1]) {
		// Prune the coin set if larger coins are not needed
		for (int i = 0; i < n_coins; i++) {
			INC_COMPARES;
			if (coins[i] > target) {
				n_coins = i;
				break;
			}
		}
	} else {
		uint32_t sum = 0, max_coin = coins[n_coins - 1];

		for (int i = 0; i < n_coins; i++) {
			sum += coins[i];
		}

		// Leap-forwards in search space as far as practicable
		INC_COMPARES;
		for (uint32_t rt = max_coin; (rt + sum) <= target; rt += max_coin) {
			INC_COMPARES;
			totals[rt] = max_coin;
			queue[0] = rt;
		}
	}

	// Now do the actual search algorithm
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
			} else {
				break;	// coins are sorted in order, no point in continuing this path
			}
		}
	}

	PRINT_COMPARES;

	// Print out the results in summarised sorted order
	if (totals[target] == 0) {
		printf("\nNo possible set of coins makes the target of %u\n", target);
	} else {
		uint32_t nr = 0, last_coin = 0, last_seq = 0;

		for (uint32_t total = target; total > 0; total -= totals[total], nr++);

		printf("\n%u coins needed to make the target of %u\n\n", nr, target);

		uint32_t res[nr];

		for (uint32_t pos = 0, total = target; total > 0; total -= totals[total], pos++) {
			res[pos] = totals[total];
		}

		qsort(res, nr, sizeof(res[0]), int32_cmp);

		for (uint32_t pos = 0; pos < nr; pos++) {
			if ((last_seq > 0) && (res[pos] != last_coin)) {
				printf("%ux%u + ", last_seq, last_coin);
				last_seq = 1;
			} else {
				last_seq++;
			}
			last_coin = res[pos];
		}
		printf("%ux%u", last_seq, last_coin);
		printf(" = %u\n", target);
	}

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
