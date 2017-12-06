#!/usr/bin/env python
# -*- coding: utf-8 -*-

from scann.tests import test_features

import unittest

def suite():
    """The global test suite.
    """

    mysuite = unittest.TestSuite()
    mysuite.addTests(test_features.suite())

    return mysuite
