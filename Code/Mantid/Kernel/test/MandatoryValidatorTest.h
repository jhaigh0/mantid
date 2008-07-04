#ifndef MANDATORYVALIDATORTEST_H_
#define MANDATORYVALIDATORTEST_H_

#include <string>
#include <cxxtest/TestSuite.h>
#include "MantidKernel/MandatoryValidator.h"

using namespace Mantid::Kernel;

class MandatoryValidatorTest : public CxxTest::TestSuite
{
public:

  void testConstructor()
  {
    TS_ASSERT_THROWS_NOTHING(MandatoryValidator<std::string> nsv)
    TS_ASSERT_THROWS_NOTHING(MandatoryValidator<std::vector<int> > nsv)
    TS_ASSERT_THROWS_NOTHING(MandatoryValidator<std::vector<double> > nsv)
    TS_ASSERT_THROWS_NOTHING(MandatoryValidator<std::vector<std::string> > nsv)
  }

  void testClone()
  {
    IValidator<std::string> *v = new MandatoryValidator<std::string>;
    IValidator<std::string> *vv = v->clone();
    TS_ASSERT_DIFFERS( v, vv )
    TS_ASSERT( dynamic_cast<MandatoryValidator<std::string>*>(vv) )

    IValidator<std::vector<int> > *i = new MandatoryValidator<std::vector<int> >;
    IValidator<std::vector<int> > *ii = i->clone();
    TS_ASSERT_DIFFERS( i, ii )
    TS_ASSERT( dynamic_cast<MandatoryValidator<std::vector<int> >*>(ii) )

    IValidator<std::vector<double> > *d = new MandatoryValidator<std::vector<double> >;
    IValidator<std::vector<double> > *dd = d->clone();
    TS_ASSERT_DIFFERS( d, dd )
    TS_ASSERT( dynamic_cast<MandatoryValidator<std::vector<double> >*>(dd) )

    IValidator<std::vector<std::string> > *s = new MandatoryValidator<std::vector<std::string> >;
    IValidator<std::vector<std::string> > *ss = s->clone();
    TS_ASSERT_DIFFERS( s, ss )
    TS_ASSERT( dynamic_cast<MandatoryValidator<std::vector<std::string> >*>(ss) )
}

  void testCast()
  {
    MandatoryValidator<std::string> *v = new MandatoryValidator<std::string>;
    TS_ASSERT( dynamic_cast<IValidator<std::string>*>(v) )

    MandatoryValidator<std::vector<int> > *i = new MandatoryValidator<std::vector<int> >;
    TS_ASSERT( dynamic_cast<IValidator<std::vector<int> >*>(i) )
    MandatoryValidator<std::vector<double> > *d = new MandatoryValidator<std::vector<double> >;
    TS_ASSERT( dynamic_cast<IValidator<std::vector<double> >*>(d) )
    MandatoryValidator<std::vector<std::string> > *s = new MandatoryValidator<std::vector<std::string> >;
    TS_ASSERT( dynamic_cast<IValidator<std::vector<std::string> >*>(s) )
  }

  void testMandatoryValidator()
  {
    MandatoryValidator<std::string> p;
    TS_ASSERT_EQUALS(p.isValid("AZ"), true);
    TS_ASSERT_EQUALS(p.isValid("B"), true);
    TS_ASSERT_EQUALS(p.isValid(""), false);
    TS_ASSERT_EQUALS(p.isValid("ta"), true);

    MandatoryValidator<std::vector<int> > i;
    std::vector<int> ivec;
    TS_ASSERT( ivec.empty() )
    TS_ASSERT( ! i.isValid(ivec) )
    ivec.push_back(1);
    TS_ASSERT( i.isValid(ivec) )

    MandatoryValidator<std::vector<double> > d;
    std::vector<double> dvec;
    TS_ASSERT( dvec.empty() )
    TS_ASSERT( ! d.isValid(dvec) )
    dvec.push_back(1.1);
    TS_ASSERT( d.isValid(dvec) )

    MandatoryValidator<std::vector<std::string> > s;
    std::vector<std::string> svec;
    TS_ASSERT( svec.empty() )
    TS_ASSERT( ! s.isValid(svec) )
    svec.push_back("OK");
    TS_ASSERT( s.isValid(svec) )
  }

};

#endif /*MANDATORYVALIDATORTEST_H_*/
