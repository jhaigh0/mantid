#include "MantidAPI/FileProperty.h"
#include "MantidCrystal/SaveLauenorm.h"
#include "MantidGeometry/Instrument/RectangularDetector.h"
#include "MantidKernel/Utils.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/ListValidator.h"
#include "MantidCrystal/AnvredCorrection.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/Strings.h"
#include "MantidAPI/Sample.h"
#include "MantidGeometry/Instrument/Goniometer.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <cmath>

using namespace Mantid::Geometry;
using namespace Mantid::DataObjects;
using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::PhysicalConstants;

namespace Mantid {
namespace Crystal {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(SaveLauenorm)

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void SaveLauenorm::init() {
  declareProperty(make_unique<WorkspaceProperty<PeaksWorkspace>>(
                      "InputWorkspace", "", Direction::Input),
                  "An input PeaksWorkspace.");
  declareProperty(
      make_unique<API::FileProperty>("Filename", "", API::FileProperty::Save),
      "Select the directory and base name for the output files.");
  auto mustBePositive = boost::make_shared<BoundedValidator<double>>();
  mustBePositive->setLower(0.0);
  declareProperty("ScalePeaks", 1.0, mustBePositive,
                  "Multiply FSQ and sig(FSQ) by scaleFactor");
  declareProperty("MinDSpacing", 0.0, "Minimum d-spacing (Angstroms)");
  declareProperty("MinWavelength", 0.0, "Minimum wavelength (Angstroms)");
  declareProperty("MaxWavelength", EMPTY_DBL(),
                  "Maximum wavelength (Angstroms)");
  std::vector<std::string> histoTypes{"Bank", "RunNumber",
                                      "Both Bank and RunNumber"};
  declareProperty("SortFilesBy", histoTypes[0],
                  boost::make_shared<StringListValidator>(histoTypes),
                  "Sort into files by bank(default), run number or both.");
  declareProperty("MinIsigI", EMPTY_DBL(), mustBePositive,
                  "The minimum I/sig(I) ratio");
  declareProperty("WidthBorder", EMPTY_INT(), "Width of border of detectors");
  declareProperty("MinIntensity", EMPTY_DBL(), mustBePositive,
                  "The minimum Intensity");
  declareProperty("UseDetScale", false, "Scale intensity and sigI by scale "
                                        "factor of detector if set in "
                                        "SetDetScale.\n"
                                        "If false, no change (default).");
  declareProperty(
      Kernel::make_unique<ArrayProperty<std::string>>("EliminateBankNumbers",
                                                      Direction::Input),
      "Comma deliminated string of bank numbers to exclude for example 1,2,5");
  declareProperty("LaueScaleFormat", false, "New format for Lauescale");
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void SaveLauenorm::exec() {

  std::string filename = getProperty("Filename");
  Poco::Path path(filename);
  std::string basename = path.getBaseName(); // Filename minus extension
  ws = getProperty("InputWorkspace");
  double scaleFactor = getProperty("ScalePeaks");
  double dMin = getProperty("MinDSpacing");
  double wlMin = getProperty("MinWavelength");
  double wlMax = getProperty("MaxWavelength");
  std::string type = getProperty("SortFilesBy");
  double minIsigI = getProperty("MinIsigI");
  double minIntensity = getProperty("MinIntensity");
  int widthBorder = getProperty("WidthBorder");
  bool newFormat = getProperty("LaueScaleFormat");

  // sequenceNo and run number
  int sequenceNo = 0;
  int oldSequence = -1;

  std::fstream out;
  std::ostringstream ss;

  // We must sort the peaks
  std::vector<std::pair<std::string, bool>> criteria;
  if (type.compare(0, 2, "Ba") == 0)
    criteria.push_back(std::pair<std::string, bool>("BankName", true));
  else if (type.compare(0, 2, "Ru") == 0)
    criteria.push_back(std::pair<std::string, bool>("RunNumber", true));
  else {
    criteria.push_back(std::pair<std::string, bool>("RunNumber", true));
    criteria.push_back(std::pair<std::string, bool>("BankName", true));
  }
  criteria.push_back(std::pair<std::string, bool>("h", true));
  criteria.push_back(std::pair<std::string, bool>("k", true));
  criteria.push_back(std::pair<std::string, bool>("l", true));
  ws->sort(criteria);

  std::vector<Peak> peaks = ws->getPeaks();

  // ============================== Save all Peaks
  // =========================================
  // HKL is flipped by -1 due to different q convention in ISAW vs mantid.
  // Default for kf-ki has -q
  double qSign = -1.0;
  std::string convention = ConfigService::Instance().getString("Q.convention");
  if (convention == "Crystallography")
    qSign = 1.0;
  // scaleDet scales intensity and sigI for detector banks
  bool scaleDet = getProperty("UseDetScale");
  auto inst = ws->getInstrument();
  OrientedLattice lattice;
  if (newFormat) {
    type = "RunNumber";
    if (!ws->sample().hasOrientedLattice()) {

      const std::string fft("FindUBUsingIndexedPeaks");
      API::IAlgorithm_sptr findUB = this->createChildAlgorithm(fft);
      findUB->initialize();
      findUB->setProperty<PeaksWorkspace_sptr>("PeaksWorkspace", ws);
      findUB->executeAsChildAlg();

      if (!ws->sample().hasOrientedLattice()) {
        g_log.notice(std::string("Could not find UB for ") +
                     std::string(ws->getName()));
        throw std::invalid_argument(std::string("Could not find UB for ") +
                                    std::string(ws->getName()));
      }
    }
    lattice = ws->sample().getOrientedLattice();
  }
  // Count peaks
  std::vector<int> numPeaks;
  int count = 0;
  std::vector<double> maxLamVec;
  std::vector<double> minLamVec;
  std::vector<double> sumLamVec;
  std::vector<double> minDVec;
  double maxLam = 0;
  double minLam = EMPTY_DBL();
  double sumLam = 0;
  double minD = EMPTY_DBL();
  for (int wi = 0; wi < ws->getNumberPeaks(); wi++) {

    Peak &p = peaks[wi];
    double intensity = p.getIntensity();
    double sigI = p.getSigmaIntensity();
    if (intensity == 0.0 || !(std::isfinite(sigI)))
      continue;
    if (minIsigI != EMPTY_DBL() && intensity < std::abs(minIsigI * sigI))
      continue;
    int sequence = p.getRunNumber();
    std::string bankName = p.getBankName();
    int nCols, nRows;
    sizeBanks(bankName, nCols, nRows);
    if (widthBorder != EMPTY_INT() &&
        (p.getCol() < widthBorder || p.getRow() < widthBorder ||
         p.getCol() > (nCols - widthBorder) ||
         p.getRow() > (nRows - widthBorder)))
      continue;
    // Take out the "bank" part of the bank name and convert to an int
    bankName.erase(remove_if(bankName.begin(), bankName.end(),
                             not1(std::ptr_fun(::isdigit))),
                   bankName.end());
    if (type.compare(0, 2, "Ba") == 0) {
      Strings::convert(bankName, sequence);
    }
    // Do not use peaks from these banks
    std::vector<std::string> notBanks = getProperty("EliminateBankNumbers");
    if (std::find(notBanks.begin(), notBanks.end(), bankName) != notBanks.end())
      continue;
    if (scaleDet) {
      if (inst->hasParameter("detScale" + bankName)) {
        double correc = static_cast<double>(
            inst->getNumberParameter("detScale" + bankName)[0]);
        intensity *= correc;
        sigI *= correc;
      }
    }
    if (minIntensity != EMPTY_DBL() && intensity < minIntensity)
      continue;
    double lambda = p.getWavelength();
    double dsp = p.getDSpacing();
    if (dsp < dMin || lambda < wlMin ||
        (wlMax != EMPTY_DBL() && lambda > wlMax))
      continue;
    if (p.getH() == 0 && p.getK() == 0 && p.getL() == 0)
      continue;

    if (sequence != oldSequence) {
      oldSequence = sequence;
      numPeaks.push_back(count);
      maxLamVec.push_back(maxLam);
      minLamVec.push_back(minLam);
      sumLamVec.push_back(sumLam);
      minDVec.push_back(minD);
      count = 0;
      maxLam = 0;
      minLam = EMPTY_DBL();
      sumLam = 0;
      minD = EMPTY_DBL();
    }
    count++;
    if (lambda < minLam)
      minLam = lambda;
    if (lambda > maxLam)
      maxLam = lambda;
    if (dsp < minD)
      minD = dsp;
    sumLam += lambda;
  }
  numPeaks.push_back(count);
  maxLamVec.push_back(maxLam);
  minLamVec.push_back(minLam);
  sumLamVec.push_back(sumLam);
  minDVec.push_back(minD);
  oldSequence = -1;
  // Go through each peak at this run / bank
  for (int wi = 0; wi < ws->getNumberPeaks(); wi++) {

    Peak &p = peaks[wi];
    double intensity = p.getIntensity();
    double sigI = p.getSigmaIntensity();
    if (intensity == 0.0 || !(std::isfinite(sigI)))
      continue;
    if (minIsigI != EMPTY_DBL() && intensity < std::abs(minIsigI * sigI))
      continue;
    int sequence = p.getRunNumber();
    std::string bankName = p.getBankName();
    int nCols, nRows;
    sizeBanks(bankName, nCols, nRows);
    if (widthBorder != EMPTY_INT() &&
        (p.getCol() < widthBorder || p.getRow() < widthBorder ||
         p.getCol() > (nCols - widthBorder) ||
         p.getRow() > (nRows - widthBorder)))
      continue;
    // Take out the "bank" part of the bank name and convert to an int
    bankName.erase(remove_if(bankName.begin(), bankName.end(),
                             not1(std::ptr_fun(::isdigit))),
                   bankName.end());
    if (type.compare(0, 2, "Ba") == 0) {
      Strings::convert(bankName, sequence);
    }
    // Do not use peaks from these banks
    std::vector<std::string> notBanks = getProperty("EliminateBankNumbers");
    if (std::find(notBanks.begin(), notBanks.end(), bankName) != notBanks.end())
      continue;
    if (scaleDet) {
      if (inst->hasParameter("detScale" + bankName)) {
        double correc = static_cast<double>(
            inst->getNumberParameter("detScale" + bankName)[0]);
        intensity *= correc;
        sigI *= correc;
      }
    }
    if (minIntensity != EMPTY_DBL() && intensity < minIntensity)
      continue;
    // Two-theta = polar angle = scattering angle = between +Z vector and the
    // scattered beam
    double scattering = p.getScattering();
    double lambda = p.getWavelength();
    double dsp = p.getDSpacing();
    if (dsp < dMin || lambda < wlMin ||
        (wlMax != EMPTY_DBL() && lambda > wlMax))
      continue;
    // This can be bank number of run number depending on
    if (sequence != oldSequence) {
      oldSequence = sequence;
      out << "END-OF-REFLECTION-DATA\n";
      out << "HARMONICS DATA    0 REFLECTIONS\n";
      out << "END-OF-FILE\n";
      out.flush();
      out.close();
      sequenceNo++;
      ss.str("");
      ss.clear();
      ss << std::setw(3) << std::setfill('0') << sequenceNo;

      // Chop off filename
      path.makeParent();
      path.append(basename + ss.str());
      if (newFormat)
        path.setExtension("geasc");
      Poco::File fileobj(path);
      out.open(path.toString().c_str(), std::ios::out);
      if (newFormat) {
        out << "TITL\n";
        out << basename << "\n";
        out << "CRYS " << basename.substr(0, 6) << "\n";
        out << "FIDX     1.00000     1.00000     1.00000     1.00000     "
               "1.00000     1.00000\n";
        out << "FIDY     1.00000     1.00000     1.00000     1.00000     "
               "1.00000     1.00000\n";
        out << "OMEG     1.00000     1.00000     1.00000     1.00000     "
               "1.00000     1.00000\n";
        out << "CELL " << std::setw(11) << std::setprecision(4)
            << 1.0 / lattice.a() << std::setw(12) << std::setprecision(4)
            << 1.0 / lattice.b() << std::setw(12) << std::setprecision(4)
            << 1.0 / lattice.c() << std::setw(9)
            << static_cast<int>(lattice.alpha() + 0.5) << std::setw(9)
            << static_cast<int>(lattice.beta() + 0.5) << std::setw(9)
            << static_cast<int>(lattice.gamma() + 0.5) << "\n";
        std::vector<int> systemNo = crystalSystem(lattice, peaks);
        out << "SYST    " << systemNo[0] << "   " << systemNo[1] << "   0   0"
            << "\n";
        out << "RAST      0.050"
            << "\n";
        out << "IBOX   1  1   1   1   1"
            << "\n";
        Goniometer gon(p.getGoniometerMatrix());
        std::vector<double> angles = gon.getEulerAngles("yzy");

        double phi = angles[2];
        double chi = angles[1];
        double omega = angles[0];

        out << "PHIS " << std::setw(11) << std::setprecision(4) << phi
            << std::setw(12) << std::setprecision(4) << chi << std::setw(12)
            << std::setprecision(4) << omega << "\n";
        out << "LAMS      ";

        out << std::setprecision(1) << std::fixed
            << sumLamVec[sequenceNo] / numPeaks[sequenceNo] << " "
            << minLamVec[sequenceNo] << " " << maxLamVec[sequenceNo] << "\n";

        out << "DMIN      ";
        out << std::setprecision(2) << std::fixed << minDVec[sequenceNo]
            << "\n";

        // distance from sample to detector (use first pixel) in mm
        double L2 = 500.0;
        out << "RADI     " << std::setprecision(0) << std::fixed << L2 << "\n";
        out << "SPIN      0.000"
            << "\n";
        out << "XC_S     0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000\n";
        out << "YC_S     0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000\n";
        out << "WC_S     0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000\n";
        out << "DELT       0.0000"
            << "\n";
        out << "TWIS    0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000 \n";
        out << "TILT    0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000 \n";
        out << "BULG    0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000 \n";
        out << "CTOF     " << L2 << "\n";
        out << "YSCA     0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000\n";
        out << "CRAT     0.00000     0.00000     0.00000     0.00000     "
               "0.00000     0.00000\n";
        out << "MINI          ";
        if (minIntensity != EMPTY_DBL()) {
          out << minIntensity << "\n";
        } else {
          out << "0.0\n";
        }
        out << "MULT  ";
        out << numPeaks[sequenceNo]
            << "     0      0      0      0      0      0      0      "
               "0      0\n";
        out << "      0      0      0      0      0      0      0      0      "
               "0      0\n";
        out << "      0 \n";
        out << "LAMH  " << numPeaks[sequenceNo]
            << "     0      0      0      0      0      0      0      0      "
               "0\n";
        out << "      0      0      0      0      0      0\n";
        out << "VERS  1"
            << "\n";
        out << "PACK        0"
            << "\n";
        out << "NSPT   " << numPeaks[sequenceNo]
            << "      0      0      0      0"
            << "\n";
        out << "NODH " << numPeaks[sequenceNo]
            << "    0      0      0      0      0      0      0      0      0\n"
            << "      0      0\n";
        out << "INTF        0"
            << "\n";
        out << "REFLECTION DATA   " << numPeaks[sequenceNo] << " REFLECTIONS"
            << "\n";
      }
    }
    // h k l lambda theta intensity and  sig(intensity)  in format
    // (3I5,2F10.5,2I10)
    // HKL is flipped by -1 due to different q convention in ISAW vs mantid.
    // unless Crystallography convention
    if (p.getH() == 0 && p.getK() == 0 && p.getL() == 0)
      continue;
    out << std::setw(5) << Utils::round(qSign * p.getH()) << std::setw(5)
        << Utils::round(qSign * p.getK()) << std::setw(5)
        << Utils::round(qSign * p.getL());
    if (newFormat) {
      // Convert to mm from centre
      out << std::setw(10) << std::fixed << std::setprecision(5)
          << (p.getCol() - 127.5) * 150.0 / 256.0;
      out << std::setw(10) << std::fixed << std::setprecision(5)
          << (p.getRow() - 127.5) * 150.0 / 256.0 << "\n";
    }
    out << std::setw(10) << std::fixed << std::setprecision(5) << lambda;
    if (newFormat) {
      // mult nodal ovlp close h2 k2 l2 nidx lambda2 ipoint
      out << " 1 0 0 0 0 0 0 0 0 0 ";
    }

    if (newFormat) {
      // Dmin threshold squared for next harmonic
      out << std::setw(10) << std::fixed << std::setprecision(5)
          << dsp * dsp * 0.25 << "\n";
    } else {
      // Assume that want theta not two-theta
      out << std::setw(10) << std::fixed << std::setprecision(5)
          << 0.5 * scattering;
    }

    // SHELX can read data without the space between the l and intensity
    if (p.getDetectorID() != -1) {
      double ckIntensity = scaleFactor * intensity;
      if (ckIntensity > 999999999.985)
        g_log.warning() << "Scaled intensity, " << ckIntensity
                        << " is too large for format.  Decrease ScalePeaks.\n";
      out << std::setw(10) << Utils::round(ckIntensity);
      if (newFormat) {
        // mult nodal ovlp close h2 k2 l2 nidx lambda2 ipoint
        out << " -9999 -9999 -9999 -9999 -9999 \n";
      }

      out << std::setw(10) << Utils::round(scaleFactor * sigI);
      if (newFormat) {
        // mult nodal ovlp close h2 k2 l2 nidx lambda2 ipoint
        out << " -9999 -9999 -9999 -9999 -9999 \n";
        out << std::setw(10) << Utils::round(ckIntensity);
        out << " -9999 -9999 -9999 -9999 -9999 \n";
        out << std::setw(10) << Utils::round(scaleFactor * sigI);
        out << " -9999 -9999 -9999 -9999 -9999 * ";
      }
    } else {
      // This is data from LoadLauenorm which is already corrected
      out << std::setw(10) << Utils::round(intensity);
      if (newFormat) {
        // 5 more films (dummy)
        out << " -9999 -9999 -9999 -9999 -9999 \n";
      }
      out << std::setw(10) << Utils::round(sigI);
      if (newFormat) {
        // 5 more films (dummy)
        out << " -9999 -9999 -9999 -9999 -9999 \n";
        out << std::setw(10) << Utils::round(intensity);
        out << " -9999 -9999 -9999 -9999 -9999 \n";
        out << std::setw(10) << Utils::round(sigI);
        out << " -9999 -9999 -9999 -9999 -9999 * ";
      }
    }

    out << '\n';
  }
  if (newFormat) {
    out << "END-OF-REFLECTION-DATA\n";
    out << "HARMONICS DATA    0 REFLECTIONS\n";
    out << "END-OF-FILE\n";
  }
  out.flush();
  out.close();
}
void SaveLauenorm::sizeBanks(std::string bankName, int &nCols, int &nRows) {
  if (bankName == "None")
    return;
  boost::shared_ptr<const IComponent> parent =
      ws->getInstrument()->getComponentByName(bankName);
  if (!parent)
    return;
  if (parent->type() == "RectangularDetector") {
    boost::shared_ptr<const RectangularDetector> RDet =
        boost::dynamic_pointer_cast<const RectangularDetector>(parent);

    nCols = RDet->xpixels();
    nRows = RDet->ypixels();
  } else {
    std::vector<Geometry::IComponent_const_sptr> children;
    boost::shared_ptr<const Geometry::ICompAssembly> asmb =
        boost::dynamic_pointer_cast<const Geometry::ICompAssembly>(parent);
    asmb->getChildren(children, false);
    boost::shared_ptr<const Geometry::ICompAssembly> asmb2 =
        boost::dynamic_pointer_cast<const Geometry::ICompAssembly>(children[0]);
    std::vector<Geometry::IComponent_const_sptr> grandchildren;
    asmb2->getChildren(grandchildren, false);
    nRows = static_cast<int>(grandchildren.size());
    nCols = static_cast<int>(children.size());
  }
}
std::vector<int> SaveLauenorm::crystalSystem(OrientedLattice lattice,
                                             std::vector<Peak> peaks) {
  std::vector<int> systemVec;
  int alpha = static_cast<int>(lattice.alpha() + 0.5);
  int beta = static_cast<int>(lattice.beta() + 0.5);
  int gamma = static_cast<int>(lattice.gamma() + 0.5);
  int a = static_cast<int>(lattice.a() * 1000 + 0.5);
  int b = static_cast<int>(lattice.b() * 1000 + 0.5);
  int c = static_cast<int>(lattice.c() * 1000 + 0.5);
  if (alpha == 90 && beta == 90 && gamma == 90) {
    if (a == b && a == c) {
      systemVec.push_back(7); // cubic I,F
    } else if (a == b) {
      systemVec.push_back(4); // tetragonal I
    } else {
      systemVec.push_back(3); // orthorhombic I,A,B,C,F
    }
  } else if (alpha == 90 && beta == 90 && gamma == 120 && a == b) {
    systemVec.push_back(6); // hexagonal
  } else if ((alpha == 90 && beta == 90) || (alpha == 90 && gamma == 90) ||
             (beta == 90 && gamma == 90)) {
    systemVec.push_back(2); // monoclinic I,A,B,C
  } else if (alpha == 90 && beta == 90 && gamma != 90 && a == b && a == c) {
    systemVec.push_back(5); // rhombohedral R
  } else {
    systemVec.push_back(1); // triclinic
  }
  int i = 0;
  int fp = 0;
  int fm = 0;
  int cc = 0;
  int bc = 0;
  int ac = 0;
  int r = 0;
  int total = 0;
  for (size_t j = 0; j < peaks.size(); j++) {
    int h = static_cast<int>(peaks[j].getH() + 0.5);
    int k = static_cast<int>(peaks[j].getK() + 0.5);
    int l = static_cast<int>(peaks[j].getL() + 0.5);
    if (h + k + l == 0)
      continue;
    total++;
    if ((h + k + l) % 2 == 0) {
      i++;
    }
    if (h % 2 == 0 && k % 2 == 0 && l % 2 == 0) {
      fp++;
    }
    if (h % 2 != 0 && k % 2 != 0 && l % 2 != 0) {
      fm++;
    }
    if ((h + k) % 2 == 0) {
      cc++;
    }
    if ((h + l) % 2 == 0) {
      bc++;
    }
    if ((k + l) % 2 == 0) {
      ac++;
    }
    if ((-h + k + l) % 3 == 0) {
      r++;
    }
  }
  int maxCen = 1;
  int maxPeaks = 0;
  if (maxPeaks < i) {
    maxCen = 2; // I
    maxPeaks = i;
  }
  if (maxPeaks < ac) {
    maxCen = 3; // A
    maxPeaks = ac;
  }
  if (maxPeaks < bc) {
    maxCen = 4; // B
    maxPeaks = bc;
  }
  if (maxPeaks < cc) {
    maxCen = 5; // C
    maxPeaks = cc;
  }
  if (maxPeaks < fp || maxPeaks < fm) {
    maxCen = 6; // F
    maxPeaks = std::max(fp, fm);
  }
  if (maxPeaks < r) {
    maxCen = 7; // R
    maxPeaks = r;
  }
  if (maxPeaks < 6 * total / 10) {
    maxCen = 1; // P
  }
  systemVec.push_back(maxCen); // P
  return systemVec;
}

} // namespace Mantid
} // namespace Crystal
