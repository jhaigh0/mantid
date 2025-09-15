// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include <cxxtest/TestSuite.h>

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidAPI/WorkspacePropertiesStrategist.h"
#include "MantidFrameworkTestHelpers/WorkspaceCreationHelper.h"

using Mantid::API::WorkspacePropertiesStrategist;
using namespace Mantid::API;
using namespace Mantid::Kernel;

class WorkspacePropertiesStrategistTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static WorkspacePropertiesStrategistTest *createSuite() { return new WorkspacePropertiesStrategistTest(); }
  static void destroySuite(WorkspacePropertiesStrategistTest *suite) { delete suite; }

  void setUp() override {
    auto &ads = AnalysisDataService::Instance();
    ads.clear();

    m_matrixWs1 = WorkspaceCreationHelper::create2DWorkspace(10, 10);
    m_matrixWs2 = WorkspaceCreationHelper::create2DWorkspace(10, 10);
    m_matrixWs3 = WorkspaceCreationHelper::create2DWorkspace(10, 10);
    m_matrixWs4 = WorkspaceCreationHelper::create2DWorkspace(10, 10);
    ads.add("myWorkspace_1", m_matrixWs1);
    ads.add("myWorkspace_2", m_matrixWs2);
    ads.add("matrixWs_3", m_matrixWs3);
    ads.add("matrixWs_4", m_matrixWs4);
    auto wsGroupSimilarNames = std::make_shared<WorkspaceGroup>();
    wsGroupSimilarNames->addWorkspace(m_matrixWs1);
    wsGroupSimilarNames->addWorkspace(m_matrixWs2);
    ads.add("myWorkspace", wsGroupSimilarNames);

    auto wsGroupUnsimilarNames = std::make_shared<WorkspaceGroup>();
    wsGroupUnsimilarNames->addWorkspace(m_matrixWs3);
    wsGroupUnsimilarNames->addWorkspace(m_matrixWs4);
    ads.add("wsGroup", wsGroupUnsimilarNames);

    m_matrixInputProp1 =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("InputWorkspace", "myWorkspace_1", Direction::Input);
    m_matrixInputProp2 =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("InputWorkspace2", "myWorkspace_2", Direction::Input);
    m_matrixInputSetWithGroupSimilar =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("InputWorkspace", "myWorkspace", Direction::Input);
    m_matrixInputSetWithGroupUnsimilar =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("InputWorkspace2", "wsGroup", Direction::Input);
    m_matrixOutputProp1 =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("OutputWorksapce", "outWs1", Direction::Output);
    m_matrixOutputProp2 =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("OutputWorksapce2", "outWs2", Direction::Output);
    m_groupInput = std::make_shared<WorkspaceProperty<WorkspaceGroup>>("InputGroup", "myWorkspace", Direction::Input);

    m_matrixInputProp1->setDataItem(m_matrixWs1);
    m_matrixInputProp2->setDataItem(m_matrixWs2);
  }

  void test_noGroupWorkspacesOneInput() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputProp1}, {m_matrixOutputProp1}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    TS_ASSERT_EQUALS(propertySets, std::vector<WorkspaceInAndOutProperties>{insAndOuts})
  }

  void test_noGroupWorkspacesTwoInputs() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputProp1, m_matrixInputProp2}, {m_matrixOutputProp1}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    TS_ASSERT_EQUALS(propertySets, std::vector<WorkspaceInAndOutProperties>{insAndOuts})
  }

  void test_oneGroupInput() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupSimilar}, {}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<WorkspaceVector> expectedWorkspaceInputSets = {{m_matrixWs1}, {m_matrixWs2}};
    checkInputSets(propertySets, expectedWorkspaceInputSets);
  }

  void test_twoGroupInputs() {
    const WorkspaceInAndOutProperties insAndOuts = {
        {m_matrixInputSetWithGroupSimilar, m_matrixInputSetWithGroupUnsimilar}, {}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<WorkspaceVector> expectedWorkspaceInputSets = {{m_matrixWs1, m_matrixWs3}, {m_matrixWs2, m_matrixWs4}};
    checkInputSets(propertySets, expectedWorkspaceInputSets);
  }

  void test_oneGroupInputAndOneSingleWorkspace() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupUnsimilar, m_matrixInputProp1}, {}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<WorkspaceVector> expectedWorkspaceInputSets = {{m_matrixWs3, m_matrixWs1}, {m_matrixWs4, m_matrixWs1}};
    checkInputSets(propertySets, expectedWorkspaceInputSets);
  }

  void test_groupWorkspacePropertyIsNotSplitUp() {
    // also tests that an empty group in a group property, does not throw an error
    const WorkspaceInAndOutProperties insAndOuts = {{m_groupInput}, {}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    TS_ASSERT_EQUALS(propertySets, std::vector<WorkspaceInAndOutProperties>{insAndOuts})
  }

  void test_groupInputLeadsToGroupOutputWithSimilarNames() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupSimilar}, {m_matrixOutputProp1}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<std::vector<std::string>> expectedWorkspaceNames = {{"outWs1_1"}, {"outWs1_2"}};
    checkOutputSets(propertySets, expectedWorkspaceNames);

    // in place of the algorithm running and creating the output workspaces
    createOutputSingleWorkspaces(flatten(expectedWorkspaceNames));

    strategist.setupGroupOutputs();
    checkGroupOutputsSetupCorrectly({"outWs1"}, {{"outWs1_1", "outWs1_2"}});
  }

  void test_groupInputLeadsToGroupOutputWithUnsimilarNames() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupUnsimilar}, {m_matrixOutputProp1}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<std::vector<std::string>> expectedWorkspaceNames = {{"matrixWs_3_outWs1"}, {"matrixWs_4_outWs1"}};
    checkOutputSets(propertySets, expectedWorkspaceNames);

    // in place of the algorithm running and creating the output workspaces
    createOutputSingleWorkspaces(flatten(expectedWorkspaceNames));

    strategist.setupGroupOutputs();
    checkGroupOutputsSetupCorrectly({"outWs1"}, {{"matrixWs_3_outWs1", "matrixWs_4_outWs1"}});
  }

  void test_twoGroupInputsCombineOutputNamesWhenUnsimilar() {
    const WorkspaceInAndOutProperties insAndOuts = {
        {m_matrixInputSetWithGroupSimilar, m_matrixInputSetWithGroupUnsimilar}, {m_matrixOutputProp1}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<std::vector<std::string>> expectedWorkspaceNames = {{"myWorkspace_1_matrixWs_3_outWs1"},
                                                                    {"myWorkspace_2_matrixWs_4_outWs1"}};
    checkOutputSets(propertySets, expectedWorkspaceNames);

    // in place of the algorithm running and creating the output workspaces
    createOutputSingleWorkspaces(flatten(expectedWorkspaceNames));

    strategist.setupGroupOutputs();
    checkGroupOutputsSetupCorrectly({"outWs1"},
                                    {{"myWorkspace_1_matrixWs_3_outWs1", "myWorkspace_2_matrixWs_4_outWs1"}});
  }

  void test_groupInputWithTwoOutputs() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupSimilar},
                                                    {m_matrixOutputProp1, m_matrixOutputProp2}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    std::vector<std::vector<std::string>> expectedWorkspaceNames = {{"outWs1_1", "outWs2_1"}, {"outWs1_2", "outWs2_2"}};
    checkOutputSets(propertySets, expectedWorkspaceNames);

    // in place of the algorithm running and creating the output workspaces
    createOutputSingleWorkspaces(flatten(expectedWorkspaceNames));

    strategist.setupGroupOutputs();
    checkGroupOutputsSetupCorrectly({"outWs1", "outWs2"}, {{"outWs1_1", "outWs1_2"}, {"outWs2_1", "outWs2_2"}});
  }

  void test_outputOverwritingInputGroup() {
    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupSimilar},
                                                    {m_matrixInputSetWithGroupSimilar}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    const auto propertySets = strategist.createInputOutputStrategy();

    checkInputSets(propertySets, {{m_matrixWs1}, {m_matrixWs2}});
    checkOutputSets(propertySets, {{"myWorkspace_1"}, {"myWorkspace_2"}});
  }

  void test_inputGroupsWithDifferentSizes() {
    auto &ads = AnalysisDataService::Instance();
    const auto ws = WorkspaceCreationHelper::create2DWorkspace(10, 10);
    ads.add("ws", ws);
    auto wsGroupWithOneEntry = std::make_shared<WorkspaceGroup>();
    ads.add("wsGroupWithOneEntry", wsGroupWithOneEntry);
    wsGroupWithOneEntry->addWorkspace(ws);
    const auto matrixInputSetWithSingleGroup = std::make_shared<WorkspaceProperty<MatrixWorkspace>>(
        "InputWorkspace2", "wsGroupWithOneEntry", Direction::Input);

    const WorkspaceInAndOutProperties insAndOuts = {{m_matrixInputSetWithGroupSimilar, matrixInputSetWithSingleGroup},
                                                    {}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    TS_ASSERT_THROWS_EQUALS(strategist.createInputOutputStrategy(), const std::invalid_argument &e,
                            std::string(e.what()), "Input WorkspaceGroups are not of the same size.");
  }

  void test_emptyGroupPassedToASingleProperty() {
    auto wsGroupNoEntries = std::make_shared<WorkspaceGroup>();
    AnalysisDataService::Instance().add("wsGroupNoEntries", wsGroupNoEntries);
    const auto matrixInputSetWithEmptyGroup =
        std::make_shared<WorkspaceProperty<MatrixWorkspace>>("InputWorkspace", "wsGroupNoEntries", Direction::Input);

    const WorkspaceInAndOutProperties insAndOuts = {{matrixInputSetWithEmptyGroup}, {}};

    WorkspacePropertiesStrategist strategist(insAndOuts);
    TS_ASSERT_THROWS_EQUALS(strategist.createInputOutputStrategy(), const std::invalid_argument &e,
                            std::string(e.what()), "Empty group passed to non-optional input.");
  }

private:
  void checkInputSets(const std::vector<WorkspaceInAndOutProperties> &propertySets,
                      const std::vector<WorkspaceVector> &expectedWorkspaceInputSets) {
    TS_ASSERT_EQUALS(propertySets.size(), expectedWorkspaceInputSets.size());
    for (size_t i = 0; i < propertySets.size(); i++) {
      const auto inAndOutProperties = propertySets[i];
      const auto inputProperties = inAndOutProperties.workspacesIn;
      const auto inputWorkspaces = expectedWorkspaceInputSets[i];
      TS_ASSERT_EQUALS(inputProperties.size(), inputWorkspaces.size());
      for (size_t j = 0; j < inputProperties.size(); j++) {
        const auto inputProperty = inputProperties[j];
        TS_ASSERT_EQUALS(inputProperty->getWorkspace(), inputWorkspaces[j]);
      }
    }
  }

  void checkOutputSets(const std::vector<WorkspaceInAndOutProperties> &propertySets,
                       const std::vector<std::vector<std::string>> &expectedWorkspaceNames) {
    TS_ASSERT_EQUALS(propertySets.size(), expectedWorkspaceNames.size());
    for (size_t i = 0; i < propertySets.size(); i++) {
      const auto inAndOutProperties = propertySets[i];
      const auto outputProperties = inAndOutProperties.workspacesOut;
      const auto outputWorkspaceNames = expectedWorkspaceNames[i];
      TS_ASSERT_EQUALS(outputProperties.size(), outputWorkspaceNames.size());
      for (size_t j = 0; j < outputProperties.size(); j++) {
        const auto outputProperty = outputProperties[j];
        const auto prop = std::dynamic_pointer_cast<Property>(outputProperty);
        TS_ASSERT_EQUALS(prop->value(), outputWorkspaceNames[j]);
      }
    }
  }

  void checkGroupOutputsSetupCorrectly(const std::vector<std::string> &expectedGroupNames,
                                       const std::vector<std::vector<std::string>> &expectedWorkspaceNames) {
    auto &ads = AnalysisDataService::Instance();
    TS_ASSERT_EQUALS(expectedGroupNames.size(), expectedWorkspaceNames.size());
    for (size_t i = 0; i < expectedGroupNames.size(); i++) {
      const auto groupName = expectedGroupNames[i];
      TS_ASSERT(ads.doesExist(groupName));
      const auto ws = ads.retrieve(groupName);
      const auto wsGroup = std::dynamic_pointer_cast<WorkspaceGroup>(ws);
      TS_ASSERT(wsGroup);
      TS_ASSERT_EQUALS(wsGroup->getNames(), expectedWorkspaceNames[i]);
    }
  }

  void createOutputSingleWorkspaces(const std::vector<std::string> &workspaceNames) {
    for (const auto name : workspaceNames) {
      const auto ws = WorkspaceCreationHelper::create2DWorkspace(10, 10);
      AnalysisDataService::Instance().addOrReplace(name, ws);
    }
  }

  template <typename T> std::vector<T> flatten(const std::vector<std::vector<T>> &original) {
    std::vector<T> flattened;
    for (const auto &v : original) {
      flattened.insert(flattened.end(), v.begin(), v.end());
    }
    return flattened;
  }

  std::shared_ptr<Workspace> m_matrixWs1;
  std::shared_ptr<Workspace> m_matrixWs2;
  std::shared_ptr<Workspace> m_matrixWs3;
  std::shared_ptr<Workspace> m_matrixWs4;
  std::shared_ptr<WorkspaceProperty<MatrixWorkspace>> m_matrixInputProp1;
  std::shared_ptr<WorkspaceProperty<MatrixWorkspace>> m_matrixInputProp2;
  std::shared_ptr<WorkspaceProperty<MatrixWorkspace>> m_matrixInputSetWithGroupSimilar;
  std::shared_ptr<WorkspaceProperty<MatrixWorkspace>> m_matrixInputSetWithGroupUnsimilar;
  std::shared_ptr<WorkspaceProperty<WorkspaceGroup>> m_groupInput;
  std::shared_ptr<WorkspaceProperty<MatrixWorkspace>> m_matrixOutputProp1;
  std::shared_ptr<WorkspaceProperty<MatrixWorkspace>> m_matrixOutputProp2;
};
