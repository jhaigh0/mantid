// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/Algorithm.h"
#include "MantidDataHandling/DllConfig.h"
#include "MantidGeometry/IDetector.h"

namespace Mantid {
namespace Geometry {
class DetectorInfo;
class Instrument;
} // namespace Geometry

namespace DataHandling {
/**

Update detector positions initially loaded in from Instrument Defintion File
(IDF) from information in the provided files.
Note doing this will result in a slower performance (likely slightly slower
performance) compared to specifying the
correct detector positions in the IDF in the first place.

Note that this algorithm moves the detectors without subsequent rotation, hence
this means that detectors may not for
example face the sample perfectly after this algorithm has been applied.

Required Properties:
<UL>
<LI> Workspace - The name of the workspace </LI>
<LI> Filename - The name of and path to the input RAW file </LI>
</UL>

@author Martyn Gigg, Tessella plc
*/
class MANTID_DATAHANDLING_DLL UpdateInstrumentFromFile final : public API::Algorithm {
public:
  /// Default constructor
  UpdateInstrumentFromFile();
  /// Algorithm's name for identification overriding a virtual method
  const std::string name() const override { return "UpdateInstrumentFromFile"; }
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "Updates detector positions initially loaded in from the Instrument "
           "Definition File (IDF) with information from the provided file.";
  }

  /// Algorithm's alias for the old UpdateInstrumentFromRaw
  const std::string alias() const override { return "UpdateInstrumentFromRaw"; }

  /// Algorithm's version for identification overriding a virtual method
  int version() const override { return 1; };
  const std::vector<std::string> seeAlso() const override { return {"LoadInstrument"}; }

  /// Algorithm's category for identification overriding a virtual method
  const std::string category() const override { return "DataHandling\\Instrument;DataHandling\\Raw"; }

private:
  /// Overwrites Algorithm method. Does nothing at present
  void init() override;
  /// Overwrites Algorithm method
  void exec() override;

  /// Assumes the file is a raw file
  void updateFromRaw(const std::string &filename);
  /// Assumes the file is an ISIS NeXus file
  void updateFromNeXus(Nexus::File &nxFile);
  /// Updates from a more generic ascii file
  void updateFromAscii(const std::string &filename);

  /**
   * Simple structure to store information about the ASCII file header
   */
  struct AsciiFileHeader {
    AsciiFileHeader()
        : colCount(0), rColIdx(0), thetaColIdx(0), phiColIdx(0) {} // Zero is invalid as this is reserved for detID

    size_t colCount;
    size_t rColIdx, thetaColIdx, phiColIdx;
    std::set<size_t> detParCols;
    std::map<size_t, std::string> colToName;
  };

  /// Parse the header and fill the headerInfo struct
  bool parseAsciiHeader(AsciiFileHeader &headerInfo);
  /// Set the new detector positions
  void setDetectorPositions(const std::vector<int32_t> &detID, const std::vector<float> &l2,
                            const std::vector<float> &theta, const std::vector<float> &phi);
  /// Set the new detector position for a single det ID
  void setDetectorPosition(Geometry::DetectorInfo &detectorInfo, const size_t index, const float l2, const float theta,
                           const float phi);

  /// The input workspace to modify
  API::MatrixWorkspace_sptr m_workspace;

  /// Cached ignore phi
  bool m_ignorePhi;
  /// Cached ignore Monitors
  bool m_ignoreMonitors;
};

} // namespace DataHandling
} // namespace Mantid
