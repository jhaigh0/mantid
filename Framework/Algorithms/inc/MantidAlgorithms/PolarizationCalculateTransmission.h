// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/Algorithm.h"
#include "MantidAPI/WorkspaceGroup_fwd.h"
#include "MantidAlgorithms/DllConfig.h"

namespace Mantid {
namespace Algorithms {

/** PolarizationCalculateTransmission : TODO: DESCRIPTION
 */
class MANTID_ALGORITHMS_DLL PolarizationCalculateTransmission : public API::Algorithm {
public:
  const std::string name() const override;
  int version() const override;
  const std::string category() const override;
  const std::string summary() const override;

private:
  void init() override;
  void exec() override;

  Mantid::API::WorkspaceGroup_sptr loadTransmission(Mantid::API::WorkspaceGroup_sptr wsGroup, bool keepInADS = false);
  Mantid::API::MatrixWorkspace_sptr meanTransmission(Mantid::API::WorkspaceGroup_sptr wsGroup);
};

} // namespace Algorithms
} // namespace Mantid
