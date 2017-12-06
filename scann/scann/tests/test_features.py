#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest
import numpy as np
from scann import features

class TestFeatures(unittest.TestCase):

    not_flashing_1 = np.zeros((5, 5), dtype=np.uint32)
    not_flashing_2 = np.array([[0, 0, 0, 0, 0],
                               [0, 0, 0, 1, 0],
                               [0, 0, 0, 0, 0],
                               [0, 1, 0, 0, 0],
                               [0, 0, 0, 0, 0]], dtype=np.uint32)
    flashing_1 = np.array([[0, 0, 0, 0, 0],
                           [0, 1, 1, 1, 0],
                           [0, 0, 0, 0, 0],
                           [0, 1, 1, 1, 0],
                           [0, 0, 0, 0, 0]], dtype=np.uint32)
    flashing_2 = np.array([[0, 0, 0, 0, 0],
                           [0, 0, 1, 0, 0],
                           [0, 0, 0, 0, 0],
                           [0, 0, 1, 0, 0],
                           [0, 0, 0, 0, 0]], dtype=np.uint32)
    flashing_3 = np.array([[0, 0, 0, 0, 0],
                           [0, 1, 0, 1, 0],
                           [0, 0, 0, 0, 0],
                           [0, 0, 1, 0, 0],
                           [0, 0, 0, 0, 0]], dtype=np.uint32)

    def test_flash_moved_up(self):
        pass

    def test_flash_moved_down(self):
        pass


    def test_flash(self):
        res = features.flash(None, None, self.not_flashing_1)
        self.assertFalse(np.any(res))
        res = features.flash(None, None, self.not_flashing_2)
        self.assertFalse(np.any(res))
        res = features.flash(None, None, self.flashing_1)
        self.assertEqual(np.sum(res), 6)
        res = features.flash(None, None, self.flashing_2)
        self.assertEqual(np.sum(res), 2)
        res = features.flash(None, None, self.flashing_3)
        self.assertEqual(np.sum(res), 3)


def suite():
    """The test suite for test_fetures.
    """
    loader = unittest.TestLoader()
    mysuite = unittest.TestSuite()
    mysuite.addTest(loader.loadTestsFromTestCase(TestFeatures))
 
