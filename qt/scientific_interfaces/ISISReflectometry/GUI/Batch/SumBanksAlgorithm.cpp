// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2021 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "SumBanksAlgorithm.h"
#include "BatchJobAlgorithm.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AlgorithmProperties.h"
#include "MantidAPI/AlgorithmRuntimeProps.h"
#include "MantidAPI/IAlgorithm.h"
#include "MantidAPI/IAlgorithmRuntimeProps.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidQtWidgets/Common/BatchAlgorithmRunner.h"
#include "Reduction/IBatch.h"
#include "Reduction/Item.h"
#include "Reduction/PreviewRow.h"

#include <string>
#include <vector>

using namespace MantidQt::CustomInterfaces::ISISReflectometry;
using namespace Mantid::API;
using MantidQt::API::IConfiguredAlgorithm;
using MantidQt::API::IConfiguredAlgorithm_sptr;

namespace {
void updateInputProperties(Mantid::API::IAlgorithmRuntimeProps &properties, MatrixWorkspace_sptr const &workspace,
                           std::optional<ProcessingInstructions> const &detIDsStr) {
  properties.setProperty("InputWorkspace", workspace);
  if (detIDsStr) {
    AlgorithmProperties::update("ROIDetectorIDs", *detIDsStr, properties);
  }
}
} // namespace

namespace MantidQt::CustomInterfaces::ISISReflectometry::SumBanks {

/** Create a configured algorithm for summing banks. The algorithm
 * properties are set from the reduction configuration model and the
 * given row.
 * @param model : the reduction configuration model
 * @param row : the row from the preview tab
 * @param alg : this param allows the caller to override the default algorithm type e.g. for injection of a mock;
 * in normal usage this should be left as the default nullptr
 */
IConfiguredAlgorithm_sptr createConfiguredAlgorithm(IBatch const &model, PreviewRow &row, IAlgorithm_sptr alg) {
  // Create the algorithm
  if (!alg) {
    alg = Mantid::API::AlgorithmManager::Instance().create("ReflectometryISISSumBanks");
  }
  alg->setRethrows(true);
  alg->setAlwaysStoreInADS(false);
  alg->getPointerToProperty("OutputWorkspace")->createTemporaryValue();

  auto properties = std::make_unique<Mantid::API::AlgorithmRuntimeProps>();

  // Look up properties for this run on the lookup table
  auto lookupRow = model.findLookupRow(row);
  if (lookupRow) {
    AlgorithmProperties::update("ROIDetectorIDs", lookupRow->roiDetectorIDs(), *properties);
  }

  // Set the algorithm properties from the row
  updateInputProperties(*properties, row.getLoadedWs(), row.getSelectedBanks());

  // Return the configured algorithm
  auto jobAlgorithm =
      std::make_shared<BatchJobAlgorithm>(std::move(alg), std::move(properties), updateRowOnAlgorithmComplete, &row);
  return jobAlgorithm;
}

void updateRowOnAlgorithmComplete(const IAlgorithm_sptr &algorithm, Item &item) {
  auto &row = dynamic_cast<PreviewRow &>(item);
  MatrixWorkspace_sptr outputWs = algorithm->getProperty("OutputWorkspace");
  row.setSummedWs(outputWs);
}
} // namespace MantidQt::CustomInterfaces::ISISReflectometry::SumBanks
