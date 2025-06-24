// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2025 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

#include "MantidAlgorithms/PolarizationCalculateTransmission.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidGeometry/Instrument.h"

#include <algorithm>

namespace Mantid {
namespace Algorithms {
using Mantid::API::WorkspaceProperty;
using Mantid::Kernel::Direction;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(PolarizationCalculateTransmission)

//----------------------------------------------------------------------------------------------

/// Algorithms name for identification. @see Algorithm::name
const std::string PolarizationCalculateTransmission::name() const { return "PolarizationCalculateTransmission"; }

/// Algorithm's version for identification. @see Algorithm::version
int PolarizationCalculateTransmission::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string PolarizationCalculateTransmission::category() const { return "Reflectometry"; }

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string PolarizationCalculateTransmission::summary() const { return "TODO: FILL IN A SUMMARY"; }

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void PolarizationCalculateTransmission::init() {
  declareProperty(std::make_unique<WorkspaceProperty<API::WorkspaceGroup>>("DirectWorkspace", "", Direction::Input),
                  "An input workspace.");
  declareProperty(
      std::make_unique<WorkspaceProperty<API::WorkspaceGroup>>("TransmissionWorkspace", "", Direction::Input),
      "An input workspace.");
  declareProperty(std::make_unique<WorkspaceProperty<API::WorkspaceGroup>>("OutputWorkspace", "", Direction::Output),
                  "An output workspace.");
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void PolarizationCalculateTransmission::exec() {
  auto &ads = API::AnalysisDataService::Instance();

  Mantid::API::WorkspaceGroup_sptr directWs = getProperty("DirectWorkspace");
  Mantid::API::WorkspaceGroup_sptr transmissionWs = getProperty("TransmissionWorkspace");

  auto w1 = loadTransmission(transmissionWs, true);
  auto w2 = loadTransmission(directWs);
  auto w2mean = meanTransmission(w2);

  auto divideAlg = createChildAlgorithm("Divide");
  divideAlg->initialize();
  divideAlg->setPropertyValue("LHSWorkspace", w1->getName());
  divideAlg->setProperty("RHSWorkspace", w2mean);
  divideAlg->setPropertyValue("OutputWorkspace", "__transWs");
  divideAlg->execute();
  Mantid::API::WorkspaceGroup_sptr transWs = std::dynamic_pointer_cast<API::WorkspaceGroup>(ads.remove("__transWs"));

  ads.remove(w1->getName());

  setProperty("OutputWorkspace", transWs);
}

Mantid::API::WorkspaceGroup_sptr
PolarizationCalculateTransmission::loadTransmission(Mantid::API::WorkspaceGroup_sptr wsGroup, bool keepInADS) {

  const auto firstItem = std::dynamic_pointer_cast<API::MatrixWorkspace>(wsGroup->getItem(0));
  const auto instrument = firstItem->getInstrument()->getName();

  auto &ads = API::AnalysisDataService::Instance();

  if (instrument == "LARMOR") {
    // monitors bit here
    /*API::WorkspaceGroup_sptr monitors;
    API::WorkspaceGroup_sptr detectors;

    auto extractMonitorsAlg = createChildAlgorithm("ExtractMonitors");
    extractMonitorsAlg->initialize();

    const auto workspaces = wsGroup->getAllItems();
    for (API::Workspace_sptr groupItem : workspaces) {
      extractMonitorsAlg->setProperty("InputWorkspace", groupItem);
      extractMonitorsAlg->setProperty("DetectorWorkspace", groupItem->getName() + "_det");
      extractMonitorsAlg->setProperty("MonitorWorkspace", groupItem->getName() + "_mon");
      extractMonitorsAlg->execute();

      API::MatrixWorkspace_sptr detWs = extractMonitorsAlg->getProperty("DetectorWorkspace");
      API::MatrixWorkspace_sptr monWs = extractMonitorsAlg->getProperty("MonitorWorkspace");
      extractMonitorsAlg->clear();

      if (detWs) {
        detectors->addWorkspace(detWs);
      }
      if (monWs) {
        monitors->addWorkspace(monWs);
      }
    }*/

    const std::string binning = "3.1,0.025,13.5";

    /*if (!monitors->isEmpty()) {
      auto convertUnitsAlg = createChildAlgorithm("ConvertUnits");
      convertUnitsAlg->initialize();
      convertUnitsAlg->setProperty("InputWorkspace", monitors);
      convertUnitsAlg->setProperty("Target", "Wavelength");
      convertUnitsAlg->setProperty("AlignBins", true);
      convertUnitsAlg->execute();
      monitors = convertUnitsAlg->getProperty("OutputWorkspace");

      auto rebinAlg = createChildAlgorithm("Rebin");
      rebinAlg->initialize();
      rebinAlg->setProperty("InputWorkspace", monitors);
      rebinAlg->setProperty("Params", binning);
      rebinAlg->setProperty("PreserveEvents", false);
      rebinAlg->execute();
      monitors = rebinAlg->getProperty("OutputWorkspace");

      auto appendAlg = createChildAlgorithm("AppendSpectra");
      appendAlg->initialize();
      appendAlg->setProperty("InputWorkspace1", monitors);
      appendAlg->setProperty("InputWorkspace2", detectors);
      appendAlg->execute();
      wsGroup = appendAlg->getProperty("OutputWorkspace");
    }*/

    // TODO define magic numbers somewhere
    int trans_mon = 6;
    int norm_mon = 1;

    auto moveComponentAlg = createChildAlgorithm("MoveInstrumentComponent");
    moveComponentAlg->initialize();
    moveComponentAlg->setProperty("Workspace", wsGroup);
    moveComponentAlg->setProperty("DetectorID", trans_mon);
    moveComponentAlg->setProperty("Z", 25.3);
    moveComponentAlg->execute();

    auto convertUnitsAlg = createChildAlgorithm("ConvertUnits");
    convertUnitsAlg->initialize();
    convertUnitsAlg->setPropertyValue("InputWorkspace", wsGroup->getName());
    convertUnitsAlg->setProperty("Target", "Wavelength");
    convertUnitsAlg->setProperty("AlignBins", true);
    convertUnitsAlg->setPropertyValue("OutputWorkspace", wsGroup->getName());
    convertUnitsAlg->execute();

    auto rebinAlg = createChildAlgorithm("Rebin");
    rebinAlg->initialize();
    rebinAlg->setPropertyValue("InputWorkspace", wsGroup->getName());
    rebinAlg->setProperty("Params", binning);
    rebinAlg->setProperty("PreserveEvents", false);
    rebinAlg->setPropertyValue("OutputWorkspace", wsGroup->getName());
    rebinAlg->execute();

    auto extractSpectrumAlg = createChildAlgorithm("ExtractSingleSpectrum");
    extractSpectrumAlg->setChild(false);
    extractSpectrumAlg->initialize();
    extractSpectrumAlg->setPropertyValue("InputWorkspace", wsGroup->getName());
    extractSpectrumAlg->setProperty("WorkspaceIndex", norm_mon - 1);
    extractSpectrumAlg->setPropertyValue("OutputWorkspace", "__norm_monitor");
    extractSpectrumAlg->execute();

    extractSpectrumAlg->setPropertyValue("InputWorkspace", wsGroup->getName());
    extractSpectrumAlg->setProperty("WorkspaceIndex", trans_mon - 1);
    extractSpectrumAlg->setPropertyValue("OutputWorkspace", "__trans_monitor");
    extractSpectrumAlg->execute();

    auto divideAlg = createChildAlgorithm("Divide");
    divideAlg->setChild(false);
    divideAlg->initialize();
    divideAlg->setPropertyValue("LHSWorkspace", "__trans_monitor");
    divideAlg->setPropertyValue("RHSWorkspace", "__norm_monitor");
    divideAlg->setPropertyValue("OutputWorkspace", "__normalised");
    divideAlg->execute();
    API::WorkspaceGroup_sptr wsNormalised =
        std::dynamic_pointer_cast<API::WorkspaceGroup>(ads.retrieve("__normalised"));

    if (!keepInADS) {
      ads.remove("__normalise");
    }
    ads.remove("__norm_monitor");
    ads.remove("__trans_monitor");

    return wsNormalised;
  }
}

Mantid::API::MatrixWorkspace_sptr
PolarizationCalculateTransmission::meanTransmission(Mantid::API::WorkspaceGroup_sptr wsGroup) {
  auto weightedMeanAlg = createChildAlgorithm("WeightedMean");
  weightedMeanAlg->initialize();

  const auto workspaces = wsGroup->getAllItems();
  auto mean = std::dynamic_pointer_cast<API::MatrixWorkspace>(workspaces[0]);
  mean = std::accumulate(workspaces.cbegin() + 1, workspaces.cend(), mean,
                         [&](API::MatrixWorkspace_sptr meanTotal, const API::Workspace_sptr ws) {
                           const auto groupItem = std::dynamic_pointer_cast<API::MatrixWorkspace>(ws);
                           weightedMeanAlg->setProperty("InputWorkspace1", meanTotal);
                           weightedMeanAlg->setProperty("InputWorkspace2", groupItem);
                           weightedMeanAlg->execute();
                           return weightedMeanAlg->getProperty("OutputWorkspace");
                         });
  return mean;
}

} // namespace Algorithms
} // namespace Mantid
