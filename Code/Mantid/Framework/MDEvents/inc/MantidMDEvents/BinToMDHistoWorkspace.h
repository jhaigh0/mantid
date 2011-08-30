#ifndef MANTID_MDEVENTS_BINTOMDHISTOWORKSPACE_H_
#define MANTID_MDEVENTS_BINTOMDHISTOWORKSPACE_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"
#include "MantidMDEvents/MDEventWorkspace.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidKernel/Strings.h"
#include "MantidKernel/System.h"
#include "MantidMDEvents/MDEventWorkspace.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDEvents/MDHistoWorkspace.h"
#include "MantidMDEvents/MDBox.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidGeometry/MDGeometry/MDImplicitFunction.h"

using Mantid::API::IMDEventWorkspace_sptr;


namespace Mantid
{
  namespace Geometry
  {
    //Forward declaration
    class MDImplicitFunction;
  }
namespace MDEvents
{

  /** Take a MDEventWorkspace and bin it to a dense histogram
   * in a MDHistoWorkspace. This is principally used for visualization.
   * 
   * The output workspace may have fewer
   * dimensions than the input MDEventWorkspace.
   *
   * @author Janik Zikovsky
   * @date 2011-03-29 11:28:06.048254
   */
  class DLLExport BinToMDHistoWorkspace  : public API::Algorithm
  {
  public:
    BinToMDHistoWorkspace();
    ~BinToMDHistoWorkspace();

    /// Algorithm's name for identification
    virtual const std::string name() const { return "BinToMDHistoWorkspace";};
    /// Algorithm's version for identification
    virtual int version() const { return 1;};
    /// Algorithm's category for identification
    virtual const std::string category() const { return "MDEvents";}

  private:
    /// Sets documentation strings for this algorithm
    virtual void initDocs();
    /// Initialise the properties
    void init();
    /// Run the algorithm
    void exec();

    void createTransform();
    void createAlignedTransform();

    template<typename MDE, size_t nd>
    Mantid::Geometry::MDImplicitFunction * getImplicitFunctionForChunk(typename MDEventWorkspace<MDE, nd>::sptr ws, size_t * chunkMin, size_t * chunkMax);

    /// Helper method
    template<typename MDE, size_t nd>
    void do_centerpointBin(typename MDEventWorkspace<MDE, nd>::sptr ws);

    /// Helper method
    template<typename MDE, size_t nd>
    void binByIterating(typename MDEventWorkspace<MDE, nd>::sptr ws);

    /// Method to bin a single MDBox
    template<typename MDE, size_t nd>
    void binMDBox(MDBox<MDE, nd> * box, size_t * chunkMin, size_t * chunkMax);

    /// Input workspace
    Mantid::API::IMDEventWorkspace_sptr in_ws;
    /// The output MDHistoWorkspace
    Mantid::MDEvents::MDHistoWorkspace_sptr outWS;
    /// Progress reporting
    Mantid::API::Progress * prog;
    /// ImplicitFunction used
    Mantid::Geometry::MDImplicitFunction * implicitFunction;

    /// Bin dimensions to actually use
    std::vector<Mantid::Geometry::MDHistoDimension_sptr> binDimensions;
    /// Index of the dimension in the MDEW for the dimension in the output.
    std::vector<size_t> dimensionToBinFrom;

    /// Coordinate transformation to apply
    CoordTransform * m_transform;

    /// Set to true if the cut is aligned with the axes
    bool m_axisAligned;

    /// Number of dimensions in the output (binned) workspace.
    size_t outD;

    /// Basis vectors of the output dimensions
    std::vector<std::vector<coord_t> > bases;

    /// Origin (this position in the input workspace = 0,0,0 in the output).
    std::vector<coord_t> origin;

    /// Cached values for speed up
    size_t * indexMultiplier;
    signal_t * signals;
    signal_t * errors;


  };


} // namespace Mantid
} // namespace MDEvents

#endif  /* MANTID_MDEVENTS_BINTOMDHISTOWORKSPACE_H_ */
