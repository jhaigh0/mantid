#include "MantidAPI/FileProperty.h"
#include "MantidCrystal/SaveIsawUB.h"
#include "MantidKernel/Matrix.h"
#include "MantidKernel/System.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <exception>
#include <MantidGeometry/Crystal/OrientedLattice.h>
#include <MantidGeometry/Crystal/UnitCell.h>


using Mantid::Kernel::DblMatrix;
using Mantid::Geometry::UnitCell;

namespace Mantid
{
namespace Crystal
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(SaveIsawUB)
  
  using namespace Mantid::Kernel;
  using namespace Mantid::API;
  using namespace std;


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  SaveIsawUB::SaveIsawUB()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SaveIsawUB::~SaveIsawUB()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void SaveIsawUB::initDocs()
  {
    this->setWikiSummary("Save a UB matrix and lattice parameters from a workspace to an ISAW-style ASCII file.");
    this->setOptionalMessage("Save a UB matrix and lattice parameters from a workspace to an ISAW-style ASCII file.");
    this->setWikiDescription(
        "The ISAW UB Matrix file format is the transpose of the Mantid orientation matrix( |Q|=1/d) with columns beam,back, and up"
        );
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void SaveIsawUB::init()
  {
    declareProperty(new WorkspaceProperty<Workspace>("InputWorkspace","",Direction::Input),
        "An input workspace containing the orientation matrix.");

    std::vector<std::string> exts;
    exts.push_back(".mat");
    exts.push_back(".ub");
    exts.push_back(".txt");

    declareProperty(new FileProperty("Filename", "", FileProperty::Save, exts),
        "Path to an ISAW-style UB matrix text file.");
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void SaveIsawUB::exec()
  {
   try
      {
        Workspace_sptr ws1 = getProperty("InputWorkspace");
        ExperimentInfo_sptr ws = boost::dynamic_pointer_cast<ExperimentInfo>(ws1);

        if (!ws)
          throw std::invalid_argument(
              "Must specify either a MatrixWorkspace or a PeaksWorkspace or a MDEventWorkspace.");

        if (!ws->sample().hasOrientedLattice()) throw
          std::invalid_argument("Workspace must have an oriented lattice to save");

        std::string Filename = getProperty("Filename");

        ofstream out;
        out.open(Filename.c_str());

        OrientedLattice lattice = ws->sample().getOrientedLattice();
        Kernel::DblMatrix ub = lattice.getUB();

        // Write the ISAW UB matrix
        const int beam = 2;
        const int up = 1;
        const int back = 0;
        out << fixed;

        for (size_t basis = 0; basis < 3; basis++)
        {
          out << setw(11) << setprecision(8) << ub[beam][basis] << setw(12) << setprecision(8)
              << ub[back][basis] << setw(12) << setprecision(8) << ub[up][basis] << " " << endl;

        }

        out << setw(11) << setprecision(4) << lattice.a() << setw(12) << setprecision(4) << lattice.b()
            << setw(12) << setprecision(4) << lattice.c() << setw(12) << setprecision(4)
            << lattice.alpha() << setw(12) << setprecision(4) << lattice.beta() << setw(12)
            << setprecision(4) << lattice.gamma() << setw(12) << setprecision(4) << lattice.volume()
            << " " << endl;

        out << "     0.0000" << "      0.0000" << "      0.0000" << "      0.0000" << "      0.0000"
            << "      0.0000" << "      0.0000 " << endl;

        out << endl << endl;

        out << "The above matrix is the Transpose of the UB Matrix. ";
        out << "The UB matrix maps the column" << endl;
        out << "vector (h,k,l ) to the column vector ";
        out << "(q'x,q'y,q'z)." << endl;
        out << "|Q'|=1/dspacing and its coordinates are a ";
        out << "right-hand coordinate system where" << endl;
        out << " x is the beam direction and z is vertically ";
        out << "upward.(IPNS convention)" << endl;

        out.close();

      } catch (exception &s)
      {
        throw std::invalid_argument(s.what());
      }

    }



} // namespace Mantid
} // namespace Crystal

