/* Auto-generated by 'generate_mdevent_declarations.py' 
 *     on 2011-08-08 14:18:46.514442
 *
 * DO NOT EDIT!
 */ 
 
#include <boost/shared_ptr.hpp>
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidMDEvents/MDEventWorkspace.h"
 
#include "MantidMDEvents/IMDBox.h"
#include "MantidMDEvents/MDBox.h"
#include "MantidMDEvents/MDEventWorkspace.h"
#include "MantidMDEvents/MDGridBox.h"
#include "MantidMDEvents/MDBin.h"
#include "MantidMDEvents/MDBoxIterator.h"
#include "MantidMDEvents/MDLeanEvent.h"

// We need to include the .cpp files so that the declarations are picked up correctly. Weird, I know. 
// See http://www.parashift.com/c++-faq-lite/templates.html#faq-35.13 
#include "IMDBox.cpp"
#include "MDBox.cpp"
#include "MDEventWorkspace.cpp"
#include "MDGridBox.cpp"
#include "MDBin.cpp"
#include "MDBoxIterator.cpp"


namespace Mantid
{
namespace MDEvents
{
// Instantiations for MDLeanEvent
template DLLExport class MDLeanEvent<1>;
template DLLExport class MDLeanEvent<2>;
template DLLExport class MDLeanEvent<3>;
template DLLExport class MDLeanEvent<4>;
template DLLExport class MDLeanEvent<5>;
template DLLExport class MDLeanEvent<6>;
template DLLExport class MDLeanEvent<7>;
template DLLExport class MDLeanEvent<8>;
template DLLExport class MDLeanEvent<9>;


// Instantiations for IMDBox
template DLLExport class IMDBox<MDLeanEvent<1>, 1>;
template DLLExport class IMDBox<MDLeanEvent<2>, 2>;
template DLLExport class IMDBox<MDLeanEvent<3>, 3>;
template DLLExport class IMDBox<MDLeanEvent<4>, 4>;
template DLLExport class IMDBox<MDLeanEvent<5>, 5>;
template DLLExport class IMDBox<MDLeanEvent<6>, 6>;
template DLLExport class IMDBox<MDLeanEvent<7>, 7>;
template DLLExport class IMDBox<MDLeanEvent<8>, 8>;
template DLLExport class IMDBox<MDLeanEvent<9>, 9>;


// Instantiations for MDBox
template DLLExport class MDBox<MDLeanEvent<1>, 1>;
template DLLExport class MDBox<MDLeanEvent<2>, 2>;
template DLLExport class MDBox<MDLeanEvent<3>, 3>;
template DLLExport class MDBox<MDLeanEvent<4>, 4>;
template DLLExport class MDBox<MDLeanEvent<5>, 5>;
template DLLExport class MDBox<MDLeanEvent<6>, 6>;
template DLLExport class MDBox<MDLeanEvent<7>, 7>;
template DLLExport class MDBox<MDLeanEvent<8>, 8>;
template DLLExport class MDBox<MDLeanEvent<9>, 9>;


// Instantiations for MDEventWorkspace
template DLLExport class MDEventWorkspace<MDLeanEvent<1>, 1>;
template DLLExport class MDEventWorkspace<MDLeanEvent<2>, 2>;
template DLLExport class MDEventWorkspace<MDLeanEvent<3>, 3>;
template DLLExport class MDEventWorkspace<MDLeanEvent<4>, 4>;
template DLLExport class MDEventWorkspace<MDLeanEvent<5>, 5>;
template DLLExport class MDEventWorkspace<MDLeanEvent<6>, 6>;
template DLLExport class MDEventWorkspace<MDLeanEvent<7>, 7>;
template DLLExport class MDEventWorkspace<MDLeanEvent<8>, 8>;
template DLLExport class MDEventWorkspace<MDLeanEvent<9>, 9>;


// Instantiations for MDGridBox
template DLLExport class MDGridBox<MDLeanEvent<1>, 1>;
template DLLExport class MDGridBox<MDLeanEvent<2>, 2>;
template DLLExport class MDGridBox<MDLeanEvent<3>, 3>;
template DLLExport class MDGridBox<MDLeanEvent<4>, 4>;
template DLLExport class MDGridBox<MDLeanEvent<5>, 5>;
template DLLExport class MDGridBox<MDLeanEvent<6>, 6>;
template DLLExport class MDGridBox<MDLeanEvent<7>, 7>;
template DLLExport class MDGridBox<MDLeanEvent<8>, 8>;
template DLLExport class MDGridBox<MDLeanEvent<9>, 9>;


// Instantiations for MDBin
template DLLExport class MDBin<MDLeanEvent<1>, 1>;
template DLLExport class MDBin<MDLeanEvent<2>, 2>;
template DLLExport class MDBin<MDLeanEvent<3>, 3>;
template DLLExport class MDBin<MDLeanEvent<4>, 4>;
template DLLExport class MDBin<MDLeanEvent<5>, 5>;
template DLLExport class MDBin<MDLeanEvent<6>, 6>;
template DLLExport class MDBin<MDLeanEvent<7>, 7>;
template DLLExport class MDBin<MDLeanEvent<8>, 8>;
template DLLExport class MDBin<MDLeanEvent<9>, 9>;


// Instantiations for MDBoxIterator
template DLLExport class MDBoxIterator<MDLeanEvent<1>, 1>;
template DLLExport class MDBoxIterator<MDLeanEvent<2>, 2>;
template DLLExport class MDBoxIterator<MDLeanEvent<3>, 3>;
template DLLExport class MDBoxIterator<MDLeanEvent<4>, 4>;
template DLLExport class MDBoxIterator<MDLeanEvent<5>, 5>;
template DLLExport class MDBoxIterator<MDLeanEvent<6>, 6>;
template DLLExport class MDBoxIterator<MDLeanEvent<7>, 7>;
template DLLExport class MDBoxIterator<MDLeanEvent<8>, 8>;
template DLLExport class MDBoxIterator<MDLeanEvent<9>, 9>;




/** Create a MDEventWorkspace of the given type
@param nd :: number of dimensions
@param eventType :: string describing the event type (currently ignored) 
@return shared pointer to the MDEventWorkspace created (as a IMDEventWorkspace).
*/
API::IMDEventWorkspace_sptr MDEventFactory::CreateMDEventWorkspace(size_t nd, std::string eventType)
{
  (void) eventType; // Avoid compiler warning
  switch(nd)
  {
  case (1):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<1>,1> >(new MDEventWorkspace<MDLeanEvent<1>,1>);
  case (2):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<2>,2> >(new MDEventWorkspace<MDLeanEvent<2>,2>);
  case (3):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<3>,3> >(new MDEventWorkspace<MDLeanEvent<3>,3>);
  case (4):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<4>,4> >(new MDEventWorkspace<MDLeanEvent<4>,4>);
  case (5):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<5>,5> >(new MDEventWorkspace<MDLeanEvent<5>,5>);
  case (6):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<6>,6> >(new MDEventWorkspace<MDLeanEvent<6>,6>);
  case (7):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<7>,7> >(new MDEventWorkspace<MDLeanEvent<7>,7>);
  case (8):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<8>,8> >(new MDEventWorkspace<MDLeanEvent<8>,8>);
  case (9):
    return boost::shared_ptr<MDEventWorkspace<MDLeanEvent<9>,9> >(new MDEventWorkspace<MDLeanEvent<9>,9>);
  default:
    throw std::invalid_argument("Invalid number of dimensions passed to CreateMDEventWorkspace.");
  }
}

} // namespace Mantid
} // namespace MDEvents 

/* THIS FILE WAS AUTO-GENERATED BY generate_mdevent_declarations.py - DO NOT EDIT! */ 
