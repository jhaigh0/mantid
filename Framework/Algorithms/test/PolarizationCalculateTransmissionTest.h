// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include <cxxtest/TestSuite.h>

#include "MantidAlgorithms/PolarizationCalculateTransmission.h"

using Mantid::Algorithms::PolarizationCalculateTransmission;

class PolarizationCalculateTransmissionTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static PolarizationCalculateTransmissionTest *createSuite() { return new PolarizationCalculateTransmissionTest(); }
  static void destroySuite(PolarizationCalculateTransmissionTest *suite) { delete suite; }

  void test_Init() {
    PolarizationCalculateTransmission alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize())
    TS_ASSERT(alg.isInitialized())
  }

  void test_exec() {}
};
