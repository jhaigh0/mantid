// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include <cxxtest/TestSuite.h>

#include "MantidAPI/WorkspaceGroup.h"
#include "MantidAlgorithms/DetermineSpinStateOrder.h"
#include "MantidFrameworkTestHelpers/WorkspaceCreationHelper.h"

using Mantid::Algorithms::DetermineSpinStateOrder;

class DetermineSpinStateOrderTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static DetermineSpinStateOrderTest *createSuite() { return new DetermineSpinStateOrderTest(); }
  static void destroySuite(DetermineSpinStateOrderTest *suite) { delete suite; }

  void test_Init() {
    DetermineSpinStateOrder alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize())
    TS_ASSERT(alg.isInitialized())
  }

  void test_validateInputs_inputWorkspaceSize() {
    const auto groupWsWithThreeItems = WorkspaceCreationHelper::createWorkspaceGroup(3, 1, 10, "three_items");

    DetermineSpinStateOrder alg;
    alg.initialize();
    alg.setProperty("InputWorkspace", groupWsWithThreeItems);

    auto errors = alg.validateInputs();
    TS_ASSERT(!errors.empty())
    TS_ASSERT_EQUALS(errors["InputWorkspace"], "Input workspace group must have 4 entries (PA data)")
    WorkspaceCreationHelper::removeWS("three_items");
  }

  void test_validateInputs_unsupportedInstrument() {
    auto wsGroupOsiris = std::make_shared<Mantid::API::WorkspaceGroup>();
    for (int i = 0; i < 4; ++i) {
      const auto ws = WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(1, 10, false, false, true, "OSIRIS");
      wsGroupOsiris->addWorkspace(ws);
    }

    DetermineSpinStateOrder alg;
    alg.initialize();
    alg.setProperty("InputWorkspace", wsGroupOsiris);

    auto errors = alg.validateInputs();
    TS_ASSERT(!errors.empty())
    TS_ASSERT_EQUALS(errors["InputWorkspace"], "Sub workspaces must be data from either LARMOR or ZOOM")
  }
};
