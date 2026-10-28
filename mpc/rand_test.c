/* Offline test of the sub-page RANDOM buttons (p<page>_rand_<attr>). From the plugin folder:
 *   gcc -fsanitize=address,undefined -g -Impc -Impc/host src/stevequencer.c mpc/rand_test.c -lm -o /tmp/r && /tmp/r
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host/plugin_api_v1.h"
#include "host/midi_fx_api_v1.h"
midi_fx_api_v1_t *move_midi_fx_init(const host_api_v1_t *host);
static int fails;
#define CHECK(c, ...) do { printf("%s ", (c) ? "ok  " : "FAIL"); printf(__VA_ARGS__); printf("\n"); if (!(c)) fails++; } while (0)
static midi_fx_api_v1_t *api;
static void *q;
static void set(const char *k, int v) { char b[16]; snprintf(b, sizeof b, "%d", v); api->set_param(q, k, b); }
static int get(const char *k) { char b[64]; if (api->get_param(q, k, b, sizeof b) < 0) return -9999; return atoi(b); }
static const char *ATTRS[] = {"pitch", "length", "on", "velo", "chance", "ratchet", "moda", "modb"};
int main(void) {
    static host_api_v1_t host; api = move_midi_fx_init(&host); q = api->create_instance(NULL, NULL);
    char k[48];
    for (int a = 0; a < 8; a++) {                       /* press RANDOM of page 2 on each sub-page in turn */
        for (int s = 1; s <= 64; s++) for (int b = 0; b < 8; b++) {   /* a known state: every value 0 (an index) */
            snprintf(k, sizeof k, "s%d_%s", s, ATTRS[b]); set(k, 0);
        }
        int before[64][8];
        for (int s = 1; s <= 64; s++) for (int b = 0; b < 8; b++) { snprintf(k, sizeof k, "s%d_%s", s, ATTRS[b]); before[s-1][b] = get(k); }
        snprintf(k, sizeof k, "p2_rand_%s", ATTRS[a]); set(k, 1); set(k, 0);
        int changedA = 0, others = 0, outside = 0;
        for (int s = 1; s <= 64; s++) for (int b = 0; b < 8; b++) {
            char kk[48]; snprintf(kk, sizeof kk, "s%d_%s", s, ATTRS[b]);
            int now = get(kk), ch = now != before[s-1][b];
            if (s >= 17 && s <= 32 && b == a) changedA += ch;
            else if (s >= 17 && s <= 32) others += ch;
            else outside += ch;
        }
        CHECK(changedA >= 4, "%-7s random: %d of 16 steps of page 2 got a new %s", ATTRS[a], changedA, ATTRS[a]);
        CHECK(others == 0, "%-7s random: the other 7 values of page 2 are untouched (%d changed)", ATTRS[a], others);
        CHECK(outside == 0, "%-7s random: pages 1, 3 and 4 are untouched (%d changed)", ATTRS[a], outside);
    }
    /* ranges */
    set("p1_rand_chance", 1); set("p1_rand_chance", 0);
    int lo = 99, hi = 0; for (int s = 1; s <= 16; s++) { snprintf(k, sizeof k, "s%d_chance", s); int v = get(k) * 5; if (v < lo) lo = v; if (v > hi) hi = v; }
    CHECK(lo >= 20 && hi <= 100, "chance random stays in 20-100%% (%d-%d)", lo, hi);
    set("p1_rand_length", 1); set("p1_rand_length", 0);
    lo = 999; hi = 0; for (int s = 1; s <= 16; s++) { snprintf(k, sizeof k, "s%d_length", s); int v = 5 + get(k) * 5; if (v < lo) lo = v; if (v > hi) hi = v; }
    CHECK(lo >= 25 && hi <= 150, "length random stays in 25-150%% (%d-%d)", lo, hi);
    set("p1_rand_ratchet", 1); set("p1_rand_ratchet", 0);
    int ok = 1, x1 = 0; for (int s = 1; s <= 16; s++) { snprintf(k, sizeof k, "s%d_ratchet", s); int v = get(k); ok &= v >= 0 && v <= 3; x1 += v == 0; }
    CHECK(ok, "ratchet random gives x1-x4 (%d of 16 are x1)", x1);
    /* the old full RANDOM still works */
    for (int s = 1; s <= 16; s++) { snprintf(k, sizeof k, "s%d_on", s); set(k, 0); }
    set("p1_random", 1); set("p1_random", 0);
    int on = 0; for (int s = 1; s <= 16; s++) { snprintf(k, sizeof k, "s%d_on", s); on += get(k); }
    CHECK(on > 0, "the full p1_random still randomises the steps (%d of 16 on)", on);
    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}
