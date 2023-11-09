// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "DllConfig.h"
#include "IndirectFitDataPresenter.h"
#include "IndirectFitOutputOptionsPresenter.h"
#include "IndirectFitPlotPresenter.h"
#include "IndirectFitPropertyBrowser.h"
#include "IndirectFittingModel.h"
#include "IndirectTab.h"
#include "ui_IndirectFitTab.h"

#include "MantidQtWidgets/Common/FunctionModelDataset.h"

#include <boost/optional.hpp>

#include <QtCore>

#include <memory>
#include <type_traits>

#include <QList>
#include <QPair>
#include <QString>

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

class MANTIDQT_INELASTIC_DLL IndirectDataAnalysisTab : public IndirectTab {
  Q_OBJECT

public:
  IndirectDataAnalysisTab(IndirectFittingModel *model, FunctionTemplateBrowser *templateBrowser,
                          IndirectFitDataView *fitDataView, std::string const &tabName, bool const hasResolution,
                          std::vector<std::string> const &hiddenProperties, QWidget *parent = nullptr);
  virtual ~IndirectDataAnalysisTab() override = default;

  template <typename FitDataPresenter> void setupFitDataPresenter() {
    m_dataPresenter =
        std::make_unique<FitDataPresenter>(m_fittingModel->getFitDataModel(), m_uiForm->dockArea->m_fitDataView);
    setupPlotView();
  }

  WorkspaceID getSelectedDataIndex() const;
  WorkspaceIndex getSelectedSpectrum() const;
  bool isRangeCurrentlySelected(WorkspaceID workspaceID, WorkspaceIndex spectrum) const;
  size_t getNumberOfCustomFunctions(const std::string &functionName) const;
  void setConvolveMembers(bool convolveMembers);

  static size_t getNumberOfSpecificFunctionContained(const std::string &functionName,
                                                     const IFunction *compositeFunction);

  std::string getTabName() const noexcept { return m_tabName; }
  bool hasResolution() const noexcept { return m_hasResolution; }
  void setFileExtensionsByName(bool filter);

protected:
  IndirectFittingModel *getFittingModel() const;
  void run() override;
  void setSampleWSSuffixes(const QStringList &suffices);
  void setSampleFBSuffixes(const QStringList &suffices);
  void setResolutionWSSuffixes(const QStringList &suffices);
  void setResolutionFBSuffixes(const QStringList &suffices);
  void setSampleSuffixes(std::string const &tab, bool filter);
  void setResolutionSuffixes(std::string const &tab, bool filter);

  void setAlgorithmProperties(const Mantid::API::IAlgorithm_sptr &fitAlgorithm) const;
  void runFitAlgorithm(Mantid::API::IAlgorithm_sptr fitAlgorithm);
  void runSingleFit(Mantid::API::IAlgorithm_sptr fitAlgorithm);
  void setupFit(Mantid::API::IAlgorithm_sptr fitAlgorithm);

  void setRunIsRunning(bool running);
  void setRunEnabled(bool enable);
  void setEditResultVisible(bool visible);
  std::unique_ptr<IndirectFitDataPresenter> m_dataPresenter;
  std::unique_ptr<IndirectFitPlotPresenter> m_plotPresenter;
  std::unique_ptr<IndirectFittingModel> m_fittingModel;
  IndirectFitPropertyBrowser *m_fitPropertyBrowser{nullptr};
  WorkspaceID m_activeWorkspaceID;
  WorkspaceIndex m_activeSpectrumIndex;

  std::unique_ptr<Ui::IndirectFitTab> m_uiForm;

private:
  void setup() override;
  bool validate() override;
  void setupPlotView();
  void connectPlotPresenter();
  void connectFitPropertyBrowser();
  void connectDataPresenter();
  void plotSelectedSpectra(std::vector<SpectrumToPlot> const &spectra);
  void plotSpectrum(std::string const &workspaceName, std::size_t const &index);
  std::string getOutputBasename() const;
  Mantid::API::WorkspaceGroup_sptr getResultWorkspace() const;
  std::vector<std::string> getFitParameterNames() const;
  QList<MantidWidgets::FunctionModelDataset> getDatasets() const;
  void enableFitButtons(bool enable);
  void enableOutputOptions(bool enable);
  void setPDFWorkspace(std::string const &workspaceName);
  void updateParameterEstimationData();
  std::string getFitTypeString() const;

  std::string m_tabName;
  bool m_hasResolution;

  std::unique_ptr<IndirectFitOutputOptionsPresenter> m_outOptionsPresenter;
  Mantid::API::IAlgorithm_sptr m_fittingAlgorithm;

protected slots:
  void setModelFitFunction();
  void setModelStartX(double startX);
  void setModelEndX(double endX);
  void tableStartXChanged(double startX, WorkspaceID workspaceID, WorkspaceIndex spectrum);
  void tableEndXChanged(double endX, WorkspaceID workspaceID, WorkspaceIndex spectrum);
  void handleStartXChanged(double startX);
  void handleEndXChanged(double endX);
  void updateFitOutput(bool error);
  void updateSingleFitOutput(bool error);
  void fitAlgorithmComplete(bool error);
  void singleFit();
  void singleFit(WorkspaceID workspaceID, WorkspaceIndex spectrum);
  void executeFit();
  void updateParameterValues();
  void updateParameterValues(const std::unordered_map<std::string, ParameterValue> &parameters);
  void updateFitBrowserParameterValues(const std::unordered_map<std::string, ParameterValue> &parameters =
                                           std::unordered_map<std::string, ParameterValue>());
  void updateFitBrowserParameterValuesFromAlg();
  void updateFitStatus();
  void updateDataReferences();
  void updateResultOptions();
  void respondToFunctionChanged();

private slots:
  void plotSelectedSpectra();
  void respondToSingleResolutionLoaded();
  void respondToDataChanged();
  void respondToDataAdded(IAddWorkspaceDialog const *dialog);
  void respondToDataRemoved();
  void respondToPlotSpectrumChanged();
  void respondToFwhmChanged(double);
  void respondToBackgroundChanged(double value);
};

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
