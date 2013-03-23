#include "build.h"
#include "merge.h"

static const gboolean build_debug = FALSE;

static void build_greedy(GRand *, Params * params, GPtrArray * trees, GSequence * merges);
static void build_add_merges(GRand *, Params * params, GSequence * merges, GPtrArray * trees, Tree * tkk);
static GSequence * build_init_merges(GRand *, Params * params, GPtrArray * trees);
static GPtrArray * build_init_trees(Params * params, GList * labels);
static void build_remove_tree(GPtrArray * trees, guint ii);

static GPtrArray * build_init_trees(Params * params, GList * labels) {
	GPtrArray * trees;

	trees = g_ptr_array_new_full(g_list_length(labels), (GDestroyNotify)tree_unref);
	for (labels = g_list_first(labels); labels != NULL; labels = g_list_next(labels)) {
		Tree * leaf = leaf_new(params, labels->data);
		g_ptr_array_add(trees, leaf);
	}
	return trees;
}


static GSequence * build_init_merges(GRand * rng, Params * params, GPtrArray * trees) {
	GSequence * merges;
	Merge * new_merge;
	Tree * aa;
	Tree * bb;
	guint ii;
	guint jj;

	merges = g_sequence_new(NULL);

	for (ii = 0; ii < trees->len; ii++) {
		aa = g_ptr_array_index(trees, ii);
		for (jj = ii + 1; jj < trees->len; jj++) {
			bb = g_ptr_array_index(trees, jj);
			new_merge = merge_join(rng, params, ii, aa, jj, bb);
			if (build_debug) {
				merge_println(new_merge, "\tadd init merge: ");
			}
			g_sequence_insert_sorted(merges, new_merge, merge_cmp_score, NULL);
		}
	}
	return merges;
}

static void build_remove_tree(GPtrArray * trees, guint ii) {
	gpointer * tii;

	tii = &g_ptr_array_index(trees, ii);
	tree_unref(*tii);
	*tii = NULL;
}

static void build_add_merges(GRand * rng, Params * params, GSequence * merges, GPtrArray * trees, Tree * tkk) {
	Tree *tll;
	Merge * new_merge;
	guint ll, kk;

	kk = trees->len;
	for (ll = 0; ll < trees->len; ll++) {
		if (g_ptr_array_index(trees, ll) == NULL) {
			continue;
		}

		tll = g_ptr_array_index(trees, ll);
		new_merge = merge_best(rng, params, kk, tkk, ll, tll);
		if (build_debug) {
			merge_println(new_merge, "\tadd merge: ");
		}
		g_sequence_insert_sorted(merges, new_merge, merge_cmp_score, NULL);
	}
}

static void build_greedy(GRand * rng, Params * params, GPtrArray * trees, GSequence * merges) {
	Merge * cur;
	GSequenceIter * head;
	guint live_trees;

	live_trees = trees->len;
	while (live_trees > 1) {
		g_assert(g_sequence_get_length(merges) > 0);
		head = g_sequence_get_begin_iter(merges);
		cur = g_sequence_get(head);
		g_sequence_remove(head);

		if (build_debug && g_sequence_get_length(merges) > 0) {
			Merge * cur_next = g_sequence_get(g_sequence_get_begin_iter(merges));
			g_assert(merge_cmp_score(cur, cur_next, NULL) != 0);
		}

		if (g_ptr_array_index(trees, cur->ii) == NULL ||
		    g_ptr_array_index(trees, cur->jj) == NULL) {
			goto again;
		}

		if (build_debug) {
			merge_println(cur, "best merge: ");
		}

		build_remove_tree(trees, cur->ii);
		build_remove_tree(trees, cur->jj);
		live_trees--;
		build_add_merges(rng, params, merges, trees, cur->tree);
		g_ptr_array_add(trees, cur->tree);
		tree_ref(cur->tree);
again:
		merge_free(cur);
	}
}

Tree * build(GRand * rng, Params * params) {
	GPtrArray * trees;
	GSequence * merges;
	Tree * root;
	GList * labels;

	labels = dataset_get_labels(params->dataset);
	trees = build_init_trees(params, labels);
	dataset_get_labels_free(labels);
	merges = build_init_merges(rng, params, trees);

	build_greedy(rng, params, trees, merges);

	root = g_ptr_array_index(trees, trees->len - 1);
	tree_ref(root);
	g_assert(root != NULL);
	g_ptr_array_free(trees, TRUE);
	g_sequence_foreach(merges, merge_free1, NULL);
	g_sequence_free(merges);
	return root;
}


Tree * build_repeat(GRand * rng, Params * params, guint num_repeats) {
	Tree * best;
	Tree * root;

	best = NULL;
	for (; num_repeats > 0; num_repeats--) {
		params_reset_cache(params);
		root = build(rng, params);
		if (best == NULL || tree_get_logprob(root) > tree_get_logprob(best)) {
			tree_unref(best);
			best = root;
			g_print("better(%d): ", num_repeats);
			tree_println(best, "");
		} else {
			tree_unref(root);
		}
	}
	return best;
}

