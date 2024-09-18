// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "SymmetrisePresenter.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidKernel/Logger.h"
#include "MantidQtWidgets/Common/UserInputValidator.h"
#include "MantidQtWidgets/Plotting/SingleSelector.h"
#include "MantidQtWidgets/Spectroscopy/DataValidationHelper.h"
#include "MantidQtWidgets/Spectroscopy/InterfaceUtils.h"

using namespace DataValidationHelper;
using namespace Mantid::API;
using namespace MantidQt::CustomInterfaces::InterfaceUtils;

namespace {
Mantid::Kernel::Logger g_log("SymmetrisePresenter");
} // namespace

namespace MantidQt {
using MantidWidgets::AxisID;
namespace CustomInterfaces {
//----------------------------------------------------------------------------------------------
/** Constructor
 */
SymmetrisePresenter::SymmetrisePresenter(QWidget *parent, ISymmetriseView *view,
                                         std::unique_ptr<ISymmetriseModel> model)
    : DataProcessor(parent), m_adsInstance(Mantid::API::AnalysisDataService::Instance()), m_view(view),
      m_model(std::move(model)), m_isPreview(false) {
  m_view->subscribePresenter(this);
  setRunWidgetPresenter(std::make_unique<RunPresenter>(this, m_view->getRunView()));
  setOutputPlotOptionsPresenter(
      std::make_unique<OutputPlotOptionsPresenter>(m_view->getPlotOptions(), PlotWidget::Spectra));

  m_model->setIsPositiveReflect(true);
  m_view->setDefaults();
}

SymmetrisePresenter::~SymmetrisePresenter() { m_propTrees["SymmPropTree"]->unsetFactoryForManager(m_dblManager); }

void SymmetrisePresenter::handleValidation(IUserInputValidator *validator) const {
  validateDataIsOfType(validator, m_view->getDataSelector(), "Sample", DataType::Red);
}

void SymmetrisePresenter::handleRun() {
  m_view->setRawPlotWatchADS(false);

  // There should never really be unexecuted algorithms in the queue, but it is
  // worth warning in case of possible weirdness
  size_t batchQueueLength = m_batchAlgoRunner->queueLength();
  if (batchQueueLength > 0)
    g_log.warning() << "Batch queue already contains " << batchQueueLength << " algorithms!\n";

  // Return if no data has been loaded
  auto const dataWorkspaceName = m_view->getDataName();
  if (dataWorkspaceName.empty())
    return;
  // Return if E range is incorrect
  if (!m_view->verifyERange(dataWorkspaceName))
    return;

  if (m_isPreview) {
    int spectrumNumber = static_cast<int>(m_view->getPreviewSpec());
    std::vector<int> spectraRange(2, spectrumNumber);

    m_model->setupPreviewAlgorithm(m_batchAlgoRunner, spectraRange);
  } else {
    clearOutputPlotOptionsWorkspaces();
    auto const outputWorkspaceName = m_model->setupSymmetriseAlgorithm(m_batchAlgoRunner);
    // Set the workspace name for Python script export
    m_pythonExportWsName = outputWorkspaceName;
  }

  // Execute the algorithm(s) on a separated thread
  m_batchAlgoRunner->executeBatchAsync();
}

/**
 * Handles saving of workspace
 */
void SymmetrisePresenter::handleSaveClicked() {
  if (checkADSForPlotSaveWorkspace(m_pythonExportWsName, false))
    addSaveWorkspaceToQueue(QString::fromStdString(m_pythonExportWsName), QString::fromStdString(m_pythonExportWsName));
  m_batchAlgoRunner->executeBatch();
}

/**
 * Handle plotting result or preview workspace.
 *
 * @param error If the algorithm failed
 */
void SymmetrisePresenter::runComplete(bool error) {
  if (error) {
    setIsPreview(false);
    return;
  }

  if (m_isPreview) {
    m_view->previewAlgDone();
  } else {
    setOutputPlotOptionsWorkspaces({m_pythonExportWsName});
    // Enable save and plot
    m_view->enableSave(true);
  }
  m_view->setRawPlotWatchADS(true);
  setIsPreview(false);
}

void SymmetrisePresenter::setFileExtensionsByName(bool filter) {
  QStringList const noSuffixes{""};
  auto const tabName("Symmetrise");
  m_view->setFBSuffixes(filter ? getSampleFBSuffixes(tabName) : getExtensions(tabName));
  m_view->setWSSuffixes(filter ? getSampleWSSuffixes(tabName) : noSuffixes);
}

void SymmetrisePresenter::handleReflectTypeChanged(int value) {
  if (m_runPresenter->validate()) {
    m_model->setIsPositiveReflect(value == 0);
    m_view->resetEDefaults(value == 0);
  }
}

void SymmetrisePresenter::handleDoubleValueChanged(std::string const &propName, double value) {
  if (propName == "Spectrum No") {
    m_view->replotNewSpectrum(value);
  } else {
    m_view->updateRangeSelectors(propName, value);
    if (propName == "Elow") {
      m_model->getIsPositiveReflect() ? m_model->setEMin(value) : m_model->setEMax((-1) * value);
    } else if (propName == "Ehigh") {
      m_model->getIsPositiveReflect() ? m_model->setEMax(value) : m_model->setEMin((-1) * value);
    }
  }
}

void SymmetrisePresenter::handlePreviewClicked() {
  setIsPreview(true);
  handleRun();
}
void SymmetrisePresenter::handleDataReady(std::string const &dataName) {
  if (m_runPresenter->validate()) {
    m_view->plotNewData(dataName);
  }
  m_model->setWorkspaceName(dataName);
}

void SymmetrisePresenter::setIsPreview(bool preview) { m_isPreview = preview; }
} // namespace CustomInterfaces
} // namespace MantidQt
