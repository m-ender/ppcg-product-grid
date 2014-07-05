#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#define MAX_SIZE 64

// Largest prime that is less than (2^64 / 20).
static const unsigned long long p = 922337203685477537;

// A precomputed list of powers of 10 modulo p
static unsigned long long p10mp[MAX_SIZE];

typedef struct {
    char symbol;
    int visited;
} cell;

typedef struct {
    int x;
    int y;
} coord;

static cell grid[MAX_SIZE][MAX_SIZE];
static int nEquals = 0;
static coord equals[MAX_SIZE*MAX_SIZE];

static coord dirs[8] = {
    { .x = -1, .y = -1 },
    { .x =  0, .y = -1 },
    { .x =  1, .y = -1 },
    { .x =  1, .y =  0 },
    { .x =  1, .y =  1 },
    { .x =  0, .y =  1 },
    { .x = -1, .y =  1 },
    { .x = -1, .y =  0 },
};

static char best_string[2*MAX_SIZE*MAX_SIZE+1];
static int best_length = 0;
static unsigned long long best_factor;

int main(int argc, char *argv[])
{
    // Precompute p10mp
    int i;
    p10mp[0] = 1;
    for (i = 1; i < MAX_SIZE; ++i)
        p10mp[i] = (10 * p10mp[i-1]) % p;

    //mpz_t lhs;
    //mpz_init_set_str(lhs, "3141592653589793238462643383279502884", 10);
    //gmp_printf("%Zd\n", lhs);

    FILE *fp = fopen("grid.txt", "r");
    if (!fp) perror("Memory allocation failed"), exit(1);

    fseek(fp, 0L, SEEK_END);
    long lSize = ftell(fp);
    rewind(fp);

    char *buffer = calloc(1, lSize+1);
    if (!buffer)
    {
        fclose(fp);
        perror("Memory allocation failed");
        exit(1);
    }

    if (1 != fread(buffer, lSize, 1, fp))
    {
        fclose(fp);
        free(buffer);
        perror("Reading file failed");
        exit(1);
    }

    fclose(fp);

    char *cp;
    cp = buffer;
    int width = (int)strtol(cp, &cp, 10);
    int height = (int)strtol(cp, &cp, 10);

    ++cp;
    int j;
    for (j = 0; j < height; ++j)
    {
        for (i = 0; i < width; ++i)
        {
            grid[j][i].symbol = *cp;
            grid[j][i].visited = 0;

            if (*cp == '=')
            {
                equals[nEquals].x = i;
                equals[nEquals].y = j;
                ++nEquals;
            }
            ++cp;
        }
        ++cp;
    }

    printf("Analysing %d x %d grid...\n", width, height);

    unsigned long long strings_checked = 0;

    clock_t start = clock();

    for (j = 0; j < nEquals; ++j)
    {
        char expression[2*MAX_SIZE*MAX_SIZE+1];
        coord lpos = equals[j];
        coord rpos = equals[j];

        // Set to spaces to ease parsing
        for (i = 0; i < 2*MAX_SIZE*MAX_SIZE+1; ++i)
            expression[i] = ' ';

        int center = MAX_SIZE*MAX_SIZE;
        expression[center] = '=';

        int lhs_size = 0;
        int lhs_n_factors = 0;


        int lhs_dirs[MAX_SIZE*MAX_SIZE];
        for (i = 0; i < MAX_SIZE*MAX_SIZE; ++i)
            lhs_dirs[i] = -2;
        int rhs_dirs[MAX_SIZE*MAX_SIZE];
        for (i = 0; i < MAX_SIZE*MAX_SIZE; ++i)
            rhs_dirs[i] = -2;

        unsigned long long lhs_factors[MAX_SIZE*MAX_SIZE];
        unsigned long long lhs_max_factors[MAX_SIZE*MAX_SIZE];
        unsigned long long lhs_products[MAX_SIZE*MAX_SIZE];
        lhs_max_factors[0] = 0;
        lhs_products[0] = 1;

        while (lhs_size >= 0)
        {
            int dir = ++lhs_dirs[lhs_size];
            char lmost_char = expression[center-lhs_size];

            if (dir == -1)
            {
                grid[lpos.y][lpos.x].visited = 1;

                if (lmost_char == 'x')
                {
                    unsigned long long factor = strtoll(expression+center-lhs_size + 1, NULL, 10);
                    lhs_factors[lhs_n_factors] = factor;
                    ++lhs_n_factors;
                    if (factor > lhs_max_factors[lhs_n_factors-1])
                        lhs_max_factors[lhs_n_factors] = factor;
                    else
                        lhs_max_factors[lhs_n_factors] = lhs_max_factors[lhs_n_factors-1];
                    lhs_products[lhs_n_factors] = lhs_products[lhs_n_factors-1] * factor;
                    continue;
                }
                else if (lmost_char == '=')
                    continue;
                else
                {
                    unsigned long long factor = strtoll(expression+center-lhs_size, NULL, 10);
                    unsigned long long max_lhs_factor;
                    if (factor > lhs_max_factors[lhs_n_factors])
                        max_lhs_factor = factor;
                    else
                        max_lhs_factor = lhs_max_factors[lhs_n_factors];

                    unsigned long long lhs_product = lhs_products[lhs_n_factors] * factor;

                    int rhs_size = 0;

                    int rhs_n_factors = 0;
                    unsigned long long max_rhs_factor = 0;

                    unsigned long long rhs_factors[MAX_SIZE*MAX_SIZE];
                    unsigned long long max_factors[MAX_SIZE*MAX_SIZE];
                    unsigned long long rhs_products[MAX_SIZE*MAX_SIZE];
                    max_factors[0] = max_lhs_factor;
                    rhs_products[0] = 1;

                    char *rhs_factor_pointers[MAX_SIZE*MAX_SIZE];
                    rhs_factor_pointers[0] = expression+center+1;

                    while (rhs_size >= 0)
                    {
                        int dir = ++rhs_dirs[rhs_size];
                        char rmost_char = expression[center+rhs_size];

                        if (dir == -1)
                        {
                            grid[rpos.y][rpos.x].visited = 1;

                            if (rmost_char == 'x')
                            {
                                factor = strtoll(rhs_factor_pointers[rhs_n_factors], NULL, 10);
                                rhs_factors[rhs_n_factors] = factor;
                                ++rhs_n_factors;
                                rhs_factor_pointers[rhs_n_factors] = expression + center + rhs_size + 1;
                                if (factor > max_factors[rhs_n_factors-1])
                                    max_factors[rhs_n_factors] = factor;
                                else
                                    max_factors[rhs_n_factors] = max_factors[rhs_n_factors-1];
                                rhs_products[rhs_n_factors] = rhs_products[rhs_n_factors-1] * factor;
                                continue;
                            }
                            else if (rmost_char == '=')
                                continue;
                            else
                            {
                                // check for solution
                                ++strings_checked;

                                factor = strtoll(rhs_factor_pointers[rhs_n_factors], NULL, 10);
                                unsigned long long max_factor;
                                if (factor > max_factors[rhs_n_factors])
                                    max_factor = factor;
                                else
                                    max_factor = max_factors[rhs_n_factors];

                                unsigned long long rhs_product = rhs_products[rhs_n_factors] * factor;

                                //printf("%.19s = %d\n", expression + center - 9, rhs_product);
                                //  printf("%.51s\n", expression + center - 25);
                                int length = rhs_size + lhs_size + 1;

                                if (lhs_product == rhs_product &&
                                    (length > best_length ||
                                     length == best_length && max_factor > best_factor))
                                {
                                    best_length = length;
                                    best_factor = max_factor;
                                    memcpy(best_string, expression + center - lhs_size, length);
                                    best_string[length] = 0;
                                }

                            }
                        }
                        else if (dir < 8)
                        {
                            coord step = dirs[dir];
                            int new_x = rpos.x + step.x;
                            int new_y = rpos.y + step.y;
                            char new_char;

                            if (new_x < 0 || new_x >= width ||
                                new_y < 0 || new_y >= height ||
                                grid[new_y][new_x].visited ||
                                (new_char = grid[new_y][new_x].symbol) == '=' ||
                                new_char == 'x' && (rmost_char == 'x' || rmost_char == '=')) continue;

                            // TODO: Should diagonally crossing paths be disallowed, too?

                            ++rhs_size;

                            rpos.x = new_x;
                            rpos.y = new_y;
                            expression[center+rhs_size] = new_char;
                        }
                        else
                        {
                            if (rmost_char == 'x')
                                --rhs_n_factors;

                            rhs_dirs[rhs_size] = -2;
                            if (rmost_char != '=') expression[center+rhs_size] = ' ';
                            grid[rpos.y][rpos.x].visited = 0;

                            if (--rhs_size >= 0)
                            {
                                coord step = dirs[rhs_dirs[rhs_size]];
                                rpos.x -= step.x;
                                rpos.y -= step.y;
                            }
                        }
                    }
                }
            }
            else if (dir < 8)
            {
                coord step = dirs[dir];
                int new_x = lpos.x + step.x;
                int new_y = lpos.y + step.y;
                char new_char;

                if (new_x < 0 || new_x >= width ||
                    new_y < 0 || new_y >= height ||
                    grid[new_y][new_x].visited ||
                    (new_char = grid[new_y][new_x].symbol) == '=' ||
                    new_char == 'x' && (lmost_char == 'x' || lmost_char == '=')) continue;

                // TODO: Should diagonally crossing paths be disallowed, too?

                ++lhs_size;

                lpos.x = new_x;
                lpos.y = new_y;
                expression[center-lhs_size] = new_char;
            }
            else
            {
                if (lmost_char == 'x')
                    --lhs_n_factors;

                lhs_dirs[lhs_size] = -2;
                expression[center-lhs_size] = ' ';
                grid[lpos.y][lpos.x].visited = 0;

                if (--lhs_size >= 0)
                {
                    coord step = dirs[lhs_dirs[lhs_size]];
                    lpos.x -= step.x;
                    lpos.y -= step.y;
                }
            }
        }
    }

    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    printf("Checked %llu paths.\n", strings_checked);

    printf("Took %f seconds.\n", seconds);

    printf("Result: %s\n", best_string);
}
