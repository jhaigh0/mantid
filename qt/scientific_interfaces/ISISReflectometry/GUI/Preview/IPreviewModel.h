// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2021 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

namespace MantidQt::CustomInterfaces::ISISReflectometry {
class IPreviewModel {
public:
  virtual void loadWorkspace() = 0;
};
} // namespace MantidQt::CustomInterfaces::ISISReflectometry
