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

//#define COUNT_COMPARES

#ifdef COUNT_COMPARES
static uint32_t compares = 0;
#define INC_COMPARES (compares++)
#define RESET_COMPARES (compares = 0)
#define PRINT_COMPARES printf("\nSolution took %u compares\n", compares)
#else
#define INC_COMPARES
#define RESET_COMPARES
#define PRINT_COMPARES
#endif


static int
int32_cmp(const void *a, const void *b)
{
	return *((const int32_t *)a)  - *((const int32_t *)b);
} // int_cmp


// Euclid's algorithm for greatest common divisor of 2 numbers
static uint32_t
find_gcd(uint32_t a, uint32_t b)
{
	if (a == 0) {
		return b;
	}
	while (b) {
		uint32_t rem = a % b;

		a = b;
		b = rem;
	}
	return a;
} // find_gcd


// Find least common multiple of 2 numbers.  Return 0 on a uint32_t overflow
static uint32_t
find_lcm(const uint32_t a, const uint32_t b)
{
	if ((a == 0) || (b == 0)) {
		return 0;
	}

	uint64_t lcm = a * b;

	lcm /= find_gcd(a, b);

	return ((lcm > UINT32_MAX) ? 0 : lcm);
} // find_lcm


// Find the least common multiple of all the coins
static uint32_t
get_coins_lcm(const uint32_t coins[], const uint32_t n_coins)
{
	if (n_coins < 1) {
		return 0;
	}

	uint32_t lcm = coins[0];
	for (int c = 1; c < n_coins; c++) {
		lcm = find_lcm(lcm, coins[c]);
	}
	return lcm;
} // get_coins_lcm


static void
min_coins_to_total(uint32_t coins[], uint32_t n_coins, const uint32_t target)
{
	// Use calloc 'cos using stack allocation can run us out of stack space easily
	uint32_t *totals = calloc(target + 1, sizeof(*totals));
	uint32_t *queue = calloc(target + 1, sizeof(*queue));
	uint32_t *res = NULL;

	if ((totals == NULL) || (queue == NULL)) {
		fprintf(stderr, "Line %d in %s:%s(): Out of memory\n", __LINE__, __FILE__, __func__);
		goto cleanup;
	}

	// Sorting the coin set into increasing order allows for search optimisations
	qsort(coins, n_coins, sizeof(coins[0]), int32_cmp);

	// Minimise the total search space where possible
	if (target < coins[n_coins - 1]) {
		// Prune the coin set if larger coins are not needed
		for (int i = 0; i < n_coins; i++) {
			if (coins[i] > target) {
				n_coins = i;
				break;
			}
		}
	} else if (n_coins > 2) {
		uint32_t max_coin = coins[n_coins - 1];
		uint32_t lcm = get_coins_lcm(coins, n_coins);

		// Leap-forwards in search space as far as practicable
		if (lcm > 0) {
			for (uint32_t rt = max_coin; (rt + lcm) <= target; rt += max_coin) {
				totals[rt] = max_coin;
				queue[0] = rt;
			}
		}
	}

	RESET_COMPARES;

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

		if ((res = calloc(nr, sizeof(*res))) != NULL) { 
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
	}

cleanup:
	totals ? free(totals) : 0;
	queue ? free(queue) : 0;
	res ? free(res) : 0;
} // min_coins_to_total


int
main(int argc, char *argv[])
{
	uint32_t target, coins[] = {1, 2, 5, 10, 20, 50, 100, 200};	// Australian coin currency

	if (argc != 2) {
		printf("Usage: %s target\n", argv[0]);
		return 1;
	}

	target = (uint32_t)atoi(argv[1]);

	if (target < 1) {
		fprintf(stderr, "Error: target must be a positive number\n");
		return 1;
	}

	min_coins_to_total(coins, sizeof(coins) / sizeof(*coins), target);

	return 0;
} // main
