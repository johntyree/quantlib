
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "forwardoption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/forwardvanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/PricingEngines/Forward/forwardengine.hpp>
#include <ql/PricingEngines/Forward/forwardperformanceengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, moneyness, reset, expected, calculated, \
                       error, tolerance) \
    BOOST_FAIL("Forward " + exerciseTypeToString(exercise) + " " \
               + OptionTypeFormatter::toString(payoff->optionType()) + \
               " option with " \
               + payoffTypeToString(payoff) + " payoff:\n" \
               "    spot value:        " \
               + DecimalFormatter::toString(s) + "\n" \
               "    strike:            " \
               + DecimalFormatter::toString(payoff->strike()) +"\n" \
               "    moneyness:         " \
               + DecimalFormatter::toString(moneyness) +"\n" \
               "    dividend yield:    " \
               + RateFormatter::toString(q) + "\n" \
               "    risk-free rate:    " \
               + RateFormatter::toString(r) + "\n" \
               "    reference date:    " \
               + DateFormatter::toString(today) + "\n" \
               "    reset date:        " \
               + DateFormatter::toString(reset) + "\n" \
               "    maturity:          " \
               + DateFormatter::toString(exercise->lastDate()) + "\n" \
               "    volatility:        " \
               + VolatilityFormatter::toString(v) + "\n\n" \
               "    expected   " + greekName + ": " \
               + DecimalFormatter::toString(expected) + "\n" \
               "    calculated " + greekName + ": " \
               + DecimalFormatter::toString(calculated) + "\n" \
               "    error:            " \
               + DecimalFormatter::toString(error) + "\n" \
               "    tolerance:        " \
               + DecimalFormatter::toString(tolerance));

namespace {

    struct ForwardOptionData {
        Option::Type type;
        Real moneyness;
        Real s;          // spot
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time start;      // time to reset
        Time t;          // time to maturity
        Volatility v;    // volatility
        Real result;     // expected result
        Real tol;        // tolerance
    };

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}

// tests

void ForwardOptionTest::testValues() {

    BOOST_MESSAGE("Testing forward option values...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    ForwardOptionData values[] = {
        //  type, moneyness, spot,  div, rate,start,   t,  vol, result, tol
        // "Option pricing formulas", pag. 37
        { Option::Call, 1.1, 60.0, 0.04, 0.08, 0.25, 1.0, 0.30, 4.4064, 1.0e-4 },
        // "Option pricing formulas", VBA code
        {  Option::Put, 1.1, 60.0, 0.04, 0.08, 0.25, 1.0, 0.30, 8.2971, 1.0e-4 }
    };

    DayCounter dc = Settings::instance().dayCounter();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> engine(
                 new ForwardEngine<VanillaOption::arguments,
                                   VanillaOption::results>(underlyingEngine));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
            new BlackScholesProcess(Handle<Quote>(spot),
                                    Handle<YieldTermStructure>(qTS),
                                    Handle<YieldTermStructure>(rTS),
                                    Handle<BlackVolTermStructure>(volTS)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                 new PlainVanillaPayoff(values[i].type, 0.0));
        Date exDate = today + Integer(values[i].t*Settings::instance().dayCounterBase()+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + Integer(values[i].start*Settings::instance().dayCounterBase()+0.5);

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ForwardVanillaOption option(values[i].moneyness, reset,
                                    stochProcess, payoff, exercise, engine);

        Real calculated = option.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].moneyness, reset,
                           values[i].result, calculated,
                           error, tolerance);
        }
    }

}


void ForwardOptionTest::testPerformanceValues() {

    BOOST_MESSAGE("Testing forward performance option values...");

    /* The data below are the performance equivalent of the
       forward options tested above and taken from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    ForwardOptionData values[] = {
        //  type, moneyness, spot,  div, rate,start, maturity,  vol,                       result, tol
        { Option::Call, 1.1, 60.0, 0.04, 0.08, 0.25,      1.0, 0.30, 4.4064/60*QL_EXP(-0.04*0.25), 1.0e-4 },
        {  Option::Put, 1.1, 60.0, 0.04, 0.08, 0.25,      1.0, 0.30, 8.2971/60*QL_EXP(-0.04*0.25), 1.0e-4 }
    };

    DayCounter dc = Settings::instance().dayCounter();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> engine(
      new ForwardPerformanceEngine<VanillaOption::arguments,
                                   VanillaOption::results>(underlyingEngine));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
            new BlackScholesProcess(Handle<Quote>(spot),
                                    Handle<YieldTermStructure>(qTS),
                                    Handle<YieldTermStructure>(rTS),
                                    Handle<BlackVolTermStructure>(volTS)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                 new PlainVanillaPayoff(values[i].type, 0.0));
        Date exDate = today + Integer(values[i].t*Settings::instance().dayCounterBase()+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + Integer(values[i].start*Settings::instance().dayCounterBase()+0.5);

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ForwardVanillaOption option(values[i].moneyness, reset,
                                    stochProcess, payoff, exercise, engine);

        Real calculated = option.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].moneyness, reset,
                           values[i].result, calculated,
                           error, tolerance);
        }
    }

}


namespace {

  template <class Engine>
  void testForwardGreeks() {

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]   = 1.0e-5;
    tolerance["gamma"]   = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;
    tolerance["divRho"]  = 1.0e-5;
    tolerance["vega"]    = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real moneyness[] = { 0.9, 1.0, 1.1 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Integer startMonths[] = { 6, 9 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

    DayCounter dc = Settings::instance().dayCounter();
    Date today = Date::todaysDate();
    Settings::instance().setEvaluationDate(today);

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
               new BlackScholesProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> engine(new Engine(underlyingEngine));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(moneyness); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          for (Size h=0; h<LENGTH(startMonths); h++) {

            Date exDate = today + lengths[k]*Years;
            boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

            Date reset = today + startMonths[h]*Months;

            boost::shared_ptr<StrikedTypePayoff> payoff(
                                       new PlainVanillaPayoff(types[i], 0.0));

            ForwardVanillaOption option(moneyness[j], reset,
                                        stochProcess, payoff,
                                        exercise, engine);

            for (Size l=0; l<LENGTH(underlyings); l++) {
              for (Size m=0; m<LENGTH(qRates); m++) {
                for (Size n=0; n<LENGTH(rRates); n++) {
                  for (Size p=0; p<LENGTH(vols); p++) {

                      Real u = underlyings[l];
                      Rate q = qRates[m],
                           r = rRates[n];
                      Volatility v = vols[p];
                      spot->setValue(u);
                      qRate->setValue(q);
                      rRate->setValue(r);
                      vol->setValue(v);

                      Real value = option.NPV();
                      calculated["delta"]   = option.delta();
                      calculated["gamma"]   = option.gamma();
                      calculated["theta"]   = option.theta();
                      calculated["rho"]     = option.rho();
                      calculated["divRho"]  = option.dividendRho();
                      calculated["vega"]    = option.vega();

                      if (value > spot->value()*1.0e-5) {
                          // perturb spot and get delta and gamma
                          Real du = u*1.0e-4;
                          spot->setValue(u+du);
                          Real value_p = option.NPV(),
                               delta_p = option.delta();
                          spot->setValue(u-du);
                          Real value_m = option.NPV(),
                               delta_m = option.delta();
                          spot->setValue(u);
                          expected["delta"] = (value_p - value_m)/(2*du);
                          expected["gamma"] = (delta_p - delta_m)/(2*du);

                          // perturb rates and get rho and dividend rho
                          Spread dr = r*1.0e-4;
                          rRate->setValue(r+dr);
                          value_p = option.NPV();
                          rRate->setValue(r-dr);
                          value_m = option.NPV();
                          rRate->setValue(r);
                          expected["rho"] = (value_p - value_m)/(2*dr);

                          Spread dq = q*1.0e-4;
                          qRate->setValue(q+dq);
                          value_p = option.NPV();
                          qRate->setValue(q-dq);
                          value_m = option.NPV();
                          qRate->setValue(q);
                          expected["divRho"] = (value_p - value_m)/(2*dq);

                          // perturb volatility and get vega
                          Volatility dv = v*1.0e-4;
                          vol->setValue(v+dv);
                          value_p = option.NPV();
                          vol->setValue(v-dv);
                          value_m = option.NPV();
                          vol->setValue(v);
                          expected["vega"] = (value_p - value_m)/(2*dv);

                          // perturb date and get theta
                          Time dT = dc.yearFraction(today-1, today+1);
                          Settings::instance().setEvaluationDate(today-1);
                          value_m = option.NPV();
                          Settings::instance().setEvaluationDate(today+1);
                          value_p = option.NPV();
                          Settings::instance().setEvaluationDate(today);
                          expected["theta"] = (value_p - value_m)/dT;

                          // compare
                          std::map<std::string,Real>::iterator it;
                          for (it = calculated.begin();
                               it != calculated.end(); ++it) {
                              std::string greek = it->first;
                              Real expct = expected  [greek],
                                   calcl = calculated[greek],
                                   tol   = tolerance [greek];
                              Real error = relativeError(expct,calcl,u);
                              if (error>tol) {
                                  REPORT_FAILURE(greek, payoff, exercise,
                                                 u, q, r, today, v,
                                                 moneyness[j], reset,
                                                 expct, calcl, error, tol);
                              }
                          }
                      }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

}


void ForwardOptionTest::testGreeks() {

    BOOST_MESSAGE("Testing forward option greeks...");

    QL_TEST_BEGIN

    testForwardGreeks<ForwardEngine<VanillaOption::arguments,
                                    VanillaOption::results> >();

    QL_TEST_TEARDOWN
}


void ForwardOptionTest::testPerformanceGreeks() {

    BOOST_MESSAGE("Testing forward performance option greeks...");

    QL_TEST_BEGIN

    testForwardGreeks<ForwardPerformanceEngine<VanillaOption::arguments,
                                               VanillaOption::results> >();

    QL_TEST_TEARDOWN
}


test_suite* ForwardOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Forward option tests");
    suite->add(BOOST_TEST_CASE(&ForwardOptionTest::testValues));
    suite->add(BOOST_TEST_CASE(&ForwardOptionTest::testGreeks));
    suite->add(BOOST_TEST_CASE(&ForwardOptionTest::testPerformanceValues));
    suite->add(BOOST_TEST_CASE(&ForwardOptionTest::testPerformanceGreeks));
    return suite;
}

