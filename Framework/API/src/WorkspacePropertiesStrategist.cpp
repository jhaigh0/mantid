// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

#include "MantidAPI/WorkspacePropertiesStrategist.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidKernel/Property.h"

using namespace Mantid::Kernel;

namespace Mantid {
namespace API {

std::vector<WorkspaceInAndOutProperties> WorkspacePropertiesStrategist::createInputOutputStrategy() {
  m_inputWorkspaceProperties = collateInputWorkspaceInfo(m_inAndOutProperties.workspacesIn);
  if (const auto error = checkGroupSizes()) {
    throw std::invalid_argument(error.value());
  }
  return createStrategy();
}

/**
 * @brief If required, create group workspaces for each output workspace property.
 * Then add the output workspaces produced by createStrategy to the correct groups.
 * Must be called after the algorithm has been executed so that the workspaces have been
 * created.
 */
void WorkspacePropertiesStrategist::setupGroupOutputs() {
  // Going to be called after the algorithm(s) have been processed
  // So the child workspaces should now exist in the ADS

  bool unpackGroups = std::any_of(
      m_inputWorkspaceProperties.cbegin(), m_inputWorkspaceProperties.cend(),
      [](const auto &inputPropertyInfo) { return inputPropertyInfo.groupWorkspaceInSingleWorkspaceProperty; });
  if (!unpackGroups) {
    return;
  }

  std::map<std::string, WorkspaceGroup_sptr> outputWorkspaceGroups;
  for (const auto workspaceOutputProperty : m_inAndOutProperties.workspacesOut) {
    const auto prop = dynamic_cast<Property *>(workspaceOutputProperty);
    if (prop && !prop->value().empty()) {
      auto outputWorkspaceGroup = std::make_shared<WorkspaceGroup>();
      outputWorkspaceGroups[prop->name()] = outputWorkspaceGroup;
      AnalysisDataService::Instance().addOrReplace(prop->value(), outputWorkspaceGroup);
    }
  }

  for (const auto inAndOutSet : m_strategy) {
    const auto outProperties = inAndOutSet.workspacesOut;
    for (const auto outProperty : outProperties) {
      const auto prop = dynamic_cast<Property *>(outProperty);
      if (prop && !prop->value().empty()) {
        const auto outGroup = outputWorkspaceGroups[prop->name()];
        // TODO error handling if workspace doesn't exist.
        outGroup->add(prop->value());
      }
    }
  }
}

/**
 * @brief Create the sets of input and output workspace properties to be used to setup the algorithm
 * for execution or validation
 * @return vector of WorkspaceInAndOutProperties
 */
std::vector<WorkspaceInAndOutProperties> WorkspacePropertiesStrategist::createStrategy() {
  bool unpackGroups = std::any_of(
      m_inputWorkspaceProperties.cbegin(), m_inputWorkspaceProperties.cend(),
      [](const auto &inputPropertyInfo) { return inputPropertyInfo.groupWorkspaceInSingleWorkspaceProperty; });
  if (!unpackGroups) {
    // no group workspaces in single workspace properties
    m_strategy.push_back(m_inAndOutProperties);
    return m_strategy;
  }

  for (size_t i = 0; i < m_groupSizeFound; i++) {
    WorkspaceInAndOutProperties insAndOuts;
    std::string outputWsNameBase;
    // Inputs
    for (const auto &inputPropertyInfo : m_inputWorkspaceProperties) {
      const WorkspaceVector unrolledWorkspaces = inputPropertyInfo.unrolledWorkspaces;
      Workspace_sptr ws;
      if (unrolledWorkspaces.empty()) {
        insAndOuts.workspacesIn.push_back(inputPropertyInfo.inputProperty);
        continue;
      } else if (!inputPropertyInfo.groupWorkspaceInSingleWorkspaceProperty) {
        ws = unrolledWorkspaces[0];
        insAndOuts.workspacesIn.push_back(inputPropertyInfo.inputProperty);
      } else {
        const auto prop = dynamic_cast<Property *>(inputPropertyInfo.inputProperty);
        ws = unrolledWorkspaces[i];
        // Does this need to be specifically a matrix workspace, event workspace etc. property?
        const auto wsProp = new WorkspaceProperty<Workspace>(prop->name(), ws->getName(), Direction::Input);
        wsProp->setDataItem(ws);
        insAndOuts.workspacesIn.push_back(wsProp);
      }
      if (!outputWsNameBase.empty())
        outputWsNameBase += "_";
      outputWsNameBase += ws->getName();
    }

    for (const auto workspaceOutputProperty : m_inAndOutProperties.workspacesOut) {
      const auto prop = dynamic_cast<Property *>(workspaceOutputProperty);
      const std::string providedName = prop->value();
      if (providedName.empty()) {
        // TODO need to add an empty output prop???
        // could maybe just loop over the entries already in the map??
        continue;
      }
      std::string childWorkspaceName;
      if (m_inputGroupsHaveSimilarNames) {
        childWorkspaceName = providedName + "_" + std::to_string(i + 1);
      } else {
        childWorkspaceName = outputWsNameBase + "_" + providedName;
      }

      // TODO check for if the output propery is overiting an input (and work out how to handle that)
      // TODO make this neater, copy over things from algorithm.cpp? (if not used elsewhere?)
      const auto matchingInputPropertyInfo =
          std::find_if(m_inputWorkspaceProperties.cbegin(), m_inputWorkspaceProperties.cend(),
                       [&providedName](const auto &inputPropertyInfo) {
                         if (const auto prop = dynamic_cast<Property *>(inputPropertyInfo.inputProperty)) {
                           return prop->value() == providedName;
                         }
                         return false;
                       });
      if (matchingInputPropertyInfo != m_inputWorkspaceProperties.cend()) {
        const auto &unrolledWorkspaces = matchingInputPropertyInfo->unrolledWorkspaces;
        childWorkspaceName = unrolledWorkspaces[i]->getName();
      }

      const auto outputWsProp = new WorkspaceProperty<Workspace>(prop->name(), childWorkspaceName, Direction::Output);
      insAndOuts.workspacesOut.push_back(outputWsProp);
    }

    m_strategy.push_back(insAndOuts);
  }
  return m_strategy;
}

/**
 * @brief Check that any groups passed to single workspace inputs are the same size.
 * Also check that no empty groups have been passed to non-optional inputs.
 * Sets m_groupSizeFound to the size of the group inputs (or 1 by default).
 * @return An optional string with error message.
 */
std::optional<std::string> WorkspacePropertiesStrategist::checkGroupSizes() {
  m_groupSizeFound = 1;
  for (const auto &inputInfo : m_inputWorkspaceProperties) {
    const auto &workspaceVec = inputInfo.unrolledWorkspaces;
    if (workspaceVec.empty() && !inputInfo.inputProperty->isOptional() &&
        inputInfo.groupWorkspaceInSingleWorkspaceProperty) {
      return "Empty group passed to non-optional input.";
    } else if (!workspaceVec.empty() && inputInfo.groupWorkspaceInSingleWorkspaceProperty) {
      if (m_groupSizeFound > 1 && workspaceVec.size() != m_groupSizeFound) {
        return "Input WorkspaceGroups are not of the same size.";
      }
      m_groupSizeFound = workspaceVec.size();
    }
  }
  return std::nullopt;
}

/**
 * @brief Assemble a vector of InputPropertyInfo, in the process unrolling the input workspace groups
 * @param workspacesIn vector of workspace input properties
 * @return vector of InputPropertyInfo
 */
std::vector<InputPropertyInfo>
WorkspacePropertiesStrategist::collateInputWorkspaceInfo(const WorkspaceProperties &workspacesIn) {
  std::vector<InputPropertyInfo> inputWorkspaceProperties;
  const auto &ads = AnalysisDataService::Instance();
  for (const auto inputWorkspaceProperty : workspacesIn) {
    InputPropertyInfo inputInfo;
    inputInfo.inputProperty = inputWorkspaceProperty;

    const auto prop = dynamic_cast<Property *>(inputWorkspaceProperty);
    const auto wsGroupProp = dynamic_cast<WorkspaceProperty<WorkspaceGroup> *>(prop);
    const auto ws = inputWorkspaceProperty->getWorkspace();
    auto wsGroup = std::dynamic_pointer_cast<WorkspaceGroup>(ws);

    // Workspace groups are NOT returned by IWP->getWorkspace() most of the
    // time because WorkspaceProperty is templated by <MatrixWorkspace> and
    // WorkspaceGroup does not subclass <MatrixWorkspace>
    if (!wsGroup && prop && !prop->value().empty()) {
      // So try to use the name in the AnalysisDataService
      try {
        wsGroup = ads.retrieveWS<WorkspaceGroup>(prop->value());
      } catch (Exception::NotFoundError &) {
      }
    }

    // Found the group either directly or by name?
    // If the property is of type WorkspaceGroup then don't unroll
    if (wsGroup && !wsGroupProp) {
      inputInfo.unrolledWorkspaces = wsGroup->getAllItems();
      inputInfo.groupWorkspaceInSingleWorkspaceProperty = true;
      m_inputGroupsHaveSimilarNames = m_inputGroupsHaveSimilarNames && wsGroup->areNamesSimilar();
    } else {
      inputInfo.groupWorkspaceInSingleWorkspaceProperty = false;
      if (ws) {
        // Single Workspace. Treat it as a "group" with only one member
        inputInfo.unrolledWorkspaces = std::vector<std::shared_ptr<Workspace>>{ws};
      } else if (wsGroup) {
        inputInfo.unrolledWorkspaces = std::vector<std::shared_ptr<Workspace>>{wsGroup};
      } else {
        // should this be an error case?
        inputInfo.unrolledWorkspaces = std::vector<std::shared_ptr<Workspace>>{};
      }
    }

    inputWorkspaceProperties.push_back(inputInfo);
  }
  return inputWorkspaceProperties;
}

} // namespace API
} // namespace Mantid
