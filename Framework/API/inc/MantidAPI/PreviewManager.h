// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/DllConfig.h"
#include "MantidAPI/IPreview.h"
#include "MantidKernel/RegistrationHelper.h"
#include "MantidKernel/SingletonHolder.h"

#include <map>
#include <type_traits>
#include <utility>
#include <vector>

namespace Mantid {
namespace API {

using PreviewRegister =
    std::map<std::string,
             std::map<std::string, std::map<std::string, IPreview_uptr>>>;

/** PreviewManager : Manages the raw data previews.
 */
class MANTID_API_DLL PreviewManagerImpl {
public:
  std::vector<std::string> getPreviews(const std::string &facility,
                                       const std::string &technique = "");
  const IPreview &getPreview(const std::string &facility,
                             const std::string &technique,
                             const std::string &preview);
  template <class T> void subscribe() {
    static_assert(std::is_base_of<IPreview, T>::value);
    T preview;
    const auto facility = preview.facility();
    const auto technique = preview.technique();
    const auto name = preview.name();
    if (checkPreview(facility, technique, name)) {
      throw std::runtime_error(
          "Preview with the same name is already registered for the same "
          "facility and technique.");
    }
    m_previews[facility][technique][name] = std::make_unique<T>();
  }

private:
  bool checkFacility(const std::string &facility);
  bool checkTechnique(const std::string &facility,
                      const std::string &technique);
  bool checkPreview(const std::string &facility, const std::string &technique,
                    const std::string &preview);
  PreviewRegister m_previews;
};

using PreviewManager = Mantid::Kernel::SingletonHolder<PreviewManagerImpl>;

} // namespace API
} // namespace Mantid

#define DECLARE_PREVIEW(classname)                                             \
  namespace {                                                                  \
  Mantid::Kernel::RegistrationHelper register_preview_##classname(             \
      ((Mantid::API::PreviewManager::Instance().subscribe<classname>()), 0));  \
  }
