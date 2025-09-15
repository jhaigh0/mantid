// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/DllConfig.h"
#include "MantidAPI/WorkspaceProperty.h"

#include <optional>

namespace Mantid {
namespace API {

using WorkspaceProperties = std::vector<std::shared_ptr<IWorkspaceProperty>>;
using WorkspaceVector = std::vector<std::shared_ptr<Workspace>>;

struct WorkspaceInAndOutProperties {
  WorkspaceProperties workspacesIn;
  WorkspaceProperties workspacesOut;

  bool operator==(const WorkspaceInAndOutProperties &) const = default;
};

struct InputPropertyInfo {
  std::shared_ptr<IWorkspaceProperty> inputProperty;
  WorkspaceVector unrolledWorkspaces;
  bool groupWorkspaceInSingleWorkspaceProperty;
};

/** WorkspacePropertiesStrategist : TODO: DESCRIPTION
 */
class MANTID_API_DLL WorkspacePropertiesStrategist {
public:
  WorkspacePropertiesStrategist(const WorkspaceInAndOutProperties properties) : m_inAndOutProperties(properties) {};
  std::vector<WorkspaceInAndOutProperties> createInputOutputStrategy();
  void setupGroupOutputs();

private:
  std::optional<std::string> checkGroupSizes();
  std::vector<InputPropertyInfo> collateInputWorkspaceInfo(const WorkspaceProperties &workspacesIn);
  std::vector<WorkspaceInAndOutProperties> createStrategy();

  WorkspaceInAndOutProperties m_inAndOutProperties;
  size_t m_groupSizeFound{0};
  std::vector<InputPropertyInfo> m_inputWorkspaceProperties{};
  std::vector<WorkspaceInAndOutProperties> m_strategy{};
  bool m_inputGroupsHaveSimilarNames{true};
};

} // namespace API
} // namespace Mantid
