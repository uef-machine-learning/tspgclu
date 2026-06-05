"""
Tests for the store_representatives feature in tspg clustering.

Each mergeOrder entry is [p1, p2, dist, size, rep] where rep is the
index of the point closest to the merged cluster's centroid (chosen
from the two old cluster representatives).

Run with:  python3 -m pytest tests/test_representatives.py -v
       or: python3 tests/test_representatives.py
"""

import sys
import os
import unittest
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import tspg


def _make_blobs(n_per_cluster, n_clusters, dim, spread=0.1, seed=42):
    rng = np.random.RandomState(seed)
    centers = rng.randn(n_clusters, dim) * 5
    parts = []
    for c in centers:
        parts.append(c + rng.randn(n_per_cluster, dim) * spread)
    return np.vstack(parts).astype(np.float32)


def _reconstruct_clusters(merge_order, N):
    """
    Walk the merge order and return a dict {step: (p1, p2, rep, members_set)}.
    members_set is the set of original point indices in the newly merged cluster
    AFTER the merge at that step.
    """
    members = {i: {i} for i in range(N)}
    steps = []
    for entry in merge_order:
        p1, p2, dist, size, rep = int(entry[0]), int(entry[1]), entry[2], int(entry[3]), int(entry[4])
        new_members = members[p1] | members[p2]
        del members[p2]
        members[p1] = new_members
        steps.append((p1, p2, rep, frozenset(new_members)))
    return steps


class TestMergeOrderShape(unittest.TestCase):
    """mergeOrder entries must always have exactly 5 elements."""

    def setUp(self):
        self.data = _make_blobs(20, 5, 8)
        self.N = len(self.data)

    def test_five_elements_with_store_true(self):
        _, mo = tspg.tspg(self.data, 5, num_tsp=3, distance="l2",
                          store_representatives=1)
        for i, entry in enumerate(mo):
            self.assertEqual(len(entry), 5,
                             f"Entry {i} has {len(entry)} elements, expected 5")

    def test_five_elements_with_store_false(self):
        _, mo = tspg.tspg(self.data, 5, num_tsp=3, distance="l2",
                          store_representatives=0)
        for i, entry in enumerate(mo):
            self.assertEqual(len(entry), 5,
                             f"Entry {i} has {len(entry)} elements, expected 5")

    def test_merge_count(self):
        _, mo = tspg.tspg(self.data, 5, num_tsp=3, distance="l2",
                          store_representatives=1)
        self.assertEqual(len(mo), self.N - 5,
                         "Expected N - k merge steps")


class TestRepresentativeDisabled(unittest.TestCase):
    """When store_representatives=0 every rep field must be -1."""

    def test_rep_is_minus_one_when_disabled(self):
        data = _make_blobs(30, 4, 4)
        _, mo = tspg.tspg(data, 4, num_tsp=3, distance="l2",
                          store_representatives=0)
        for i, entry in enumerate(mo):
            rep = int(entry[4])
            self.assertEqual(rep, -1,
                             f"Step {i}: rep={rep}, expected -1 when disabled")


class TestRepresentativeValidity(unittest.TestCase):
    """When enabled, each rep must be a valid point index in [0, N)."""

    def setUp(self):
        self.data = _make_blobs(40, 6, 16)
        self.N = len(self.data)
        _, self.mo = tspg.tspg(self.data, 6, num_tsp=4, distance="l2",
                               store_representatives=1)

    def test_rep_in_range(self):
        for i, entry in enumerate(self.mo):
            rep = int(entry[4])
            self.assertGreaterEqual(rep, 0,
                                    f"Step {i}: rep={rep} < 0")
            self.assertLess(rep, self.N,
                            f"Step {i}: rep={rep} >= N={self.N}")

    def test_rep_in_cluster(self):
        """Representative must be a member of the merged cluster."""
        steps = _reconstruct_clusters(self.mo, self.N)
        for i, (p1, p2, rep, members) in enumerate(steps):
            self.assertIn(rep, members,
                          f"Step {i}: rep={rep} not in cluster members")


class TestRepresentativeIsFromOldReps(unittest.TestCase):
    """
    The rep must equal one of the two old representatives.
    And it must be the one closer to the merged centroid (computed in float64
    for the test, with a small tolerance for C float vs Python float64 differences).
    """

    def setUp(self):
        self.data = _make_blobs(50, 5, 10)
        self.N = len(self.data)
        _, self.mo = tspg.tspg(self.data, 5, num_tsp=5, distance="l2",
                               store_representatives=1)

    def test_rep_is_one_of_two_old_reps(self):
        reps = list(range(self.N))   # initial rep of each node = itself
        members = {i: {i} for i in range(self.N)}

        for i, entry in enumerate(self.mo):
            p1, p2, _, _, rep = int(entry[0]), int(entry[1]), entry[2], entry[3], int(entry[4])

            old_rep1 = reps[p1]
            old_rep2 = reps[p2]

            self.assertIn(rep, (old_rep1, old_rep2),
                          f"Step {i}: rep={rep} is neither old_rep1={old_rep1} "
                          f"nor old_rep2={old_rep2}")

            # Update tracking state
            new_members = members[p1] | members[p2]
            del members[p2]
            members[p1] = new_members
            reps[p1] = rep

    def test_rep_is_closer_to_centroid(self):
        """
        The chosen rep should be closer (or equal) to the merged centroid
        than the other old representative.
        Uses float64 arithmetic with a tolerance for C float rounding.
        """
        reps = list(range(self.N))
        members = {i: {i} for i in range(self.N)}
        data64 = self.data.astype(np.float64)
        TOLERANCE = 1e-3   # C uses float32 for means; small rounding differences allowed

        wrong = 0
        for i, entry in enumerate(self.mo):
            p1, p2, _, _, rep = int(entry[0]), int(entry[1]), entry[2], entry[3], int(entry[4])

            old_rep1 = reps[p1]
            old_rep2 = reps[p2]

            new_members = members[p1] | members[p2]
            centroid = data64[sorted(new_members)].mean(axis=0)

            d1 = float(np.sum((data64[old_rep1] - centroid) ** 2))
            d2 = float(np.sum((data64[old_rep2] - centroid) ** 2))

            expected = old_rep1 if d1 <= d2 else old_rep2
            other   = old_rep2  if d1 <= d2 else old_rep1
            d_chosen = d1 if rep == old_rep1 else d2
            d_other  = d2 if rep == old_rep1 else d1

            # Allow mismatch only when the two distances are within tolerance
            # (float32 rounding in C may flip a near-tie)
            if rep != expected and abs(d1 - d2) > TOLERANCE:
                wrong += 1

            del members[p2]
            members[p1] = new_members
            reps[p1] = rep

        # Allow at most 1% of steps to be affected by float32/float64 rounding
        max_allowed = max(1, len(self.mo) // 100)
        self.assertLessEqual(wrong, max_allowed,
                             f"{wrong} steps chose the wrong representative "
                             f"(tolerance={TOLERANCE})")


class TestRepresentativeQuality(unittest.TestCase):
    """
    Soft quality check: when clusters are well-separated, the representative
    should be closer to the centroid than a random member on average.
    """

    def test_rep_closer_than_random_average(self):
        rng = np.random.RandomState(7)
        data = _make_blobs(100, 8, 32, spread=0.05)
        N = len(data)
        data64 = data.astype(np.float64)

        _, mo = tspg.tspg(data, 8, num_tsp=5, distance="l2",
                          store_representatives=1)

        members = {i: {i} for i in range(N)}
        rep_dists = []
        avg_dists = []

        for entry in mo:
            p1, p2, _, _, rep = int(entry[0]), int(entry[1]), entry[2], entry[3], int(entry[4])
            new_members = members[p1] | members[p2]
            del members[p2]
            members[p1] = new_members

            cluster = sorted(new_members)
            if len(cluster) < 3:
                continue

            centroid = data64[cluster].mean(axis=0)
            rep_dists.append(np.linalg.norm(data64[rep] - centroid))
            avg_dists.append(np.mean(
                [np.linalg.norm(data64[m] - centroid) for m in cluster]
            ))

        mean_rep = np.mean(rep_dists)
        mean_avg = np.mean(avg_dists)
        # Representative should be closer than the average member
        self.assertLess(mean_rep, mean_avg,
                        f"mean rep dist {mean_rep:.4f} >= mean avg dist {mean_avg:.4f}")


if __name__ == "__main__":
    unittest.main(verbosity=2)
