// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/DllConfig.h"
#include "MantidAPI/IWorkspacePropertiesStrategist.h"
#include "MantidAPI/WorkspaceProperty.h"

#include <optional>

namespace Mantid {
namespace API {

using WorkspaceVector = std::vector<std::shared_ptr<Workspace>>;

struct InputPropertyInfo {
  IWorkspaceProperty *inputProperty;
  WorkspaceVector unrolledWorkspaces;
  bool groupWorkspaceInSingleWorkspaceProperty;
};

/** WorkspacePropertiesStrategist : Class which takes the set input and output properties to
 * an algorithm and produces a strategy (a list of sets of input / output properties) by which
 * to execute over any workspace group inputs.
 */
class MANTID_API_DLL WorkspacePropertiesStrategist : public IWorkspacePropertiesStrategist {
public:
  WorkspacePropertiesStrategist(const WorkspaceInAndOutProperties properties) : m_inAndOutProperties(properties) {};

  std::vector<WorkspaceInAndOutProperties> createInputOutputStrategy() override;
  void setupGroupOutputs() override;

private:
  std::optional<std::string> checkGroupSizes();
  std::vector<InputPropertyInfo> collateInputWorkspaceInfo(const WorkspaceProperties &workspacesIn);
  std::vector<WorkspaceInAndOutProperties> createStrategy();
  bool areGroupsToUnpack();

  WorkspaceInAndOutProperties m_inAndOutProperties;
  size_t m_groupSizeFound{0};
  std::vector<InputPropertyInfo> m_inputWorkspaceProperties{};
  std::vector<WorkspaceInAndOutProperties> m_strategy{};
  bool m_inputGroupsHaveSimilarNames{true};
};

} // namespace API
} // namespace Mantid
