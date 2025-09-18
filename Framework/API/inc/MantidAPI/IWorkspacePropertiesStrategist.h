// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/DllConfig.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/Workspace_fwd.h"

#include <memory>
#include <vector>

namespace Mantid {
namespace API {

using WorkspaceProperties = std::vector<IWorkspaceProperty *>;

struct WorkspaceInAndOutProperties {
  WorkspaceProperties workspacesIn;
  WorkspaceProperties workspacesOut;

  bool operator==(const WorkspaceInAndOutProperties &) const = default;
};

class MANTID_API_DLL IWorkspacePropertiesStrategist {
public:
  virtual std::vector<WorkspaceInAndOutProperties> createInputOutputStrategy() = 0;
  virtual void setupGroupOutputs() = 0;
};

} // namespace API
} // namespace Mantid
