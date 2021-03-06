#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#include "steadystatekalmanfilterfactory.h"

#include <cassert>
#include "kalmanfiltercalculationelementsfactory.h"

#pragma GCC diagnostic pop

ribi::kalman::SteadyStateKalmanFilterFactory::SteadyStateKalmanFilterFactory()
{

}

boost::shared_ptr<ribi::kalman::SteadyStateKalmanFilter>
ribi::kalman::SteadyStateKalmanFilterFactory::Create(
  const boost::shared_ptr<const KalmanFilterParameters>& parameters
) const noexcept
{
  assert(parameters);
  assert(parameters->GetType() == KalmanFilterType::steady_state);

  const boost::shared_ptr<KalmanFilterCalculationElements> calculation {
    KalmanFilterCalculationElementsFactory().Create(parameters->GetType())
  };

  assert(calculation);

  const boost::shared_ptr<SteadyStateKalmanFilter> kalman_filter {
    new SteadyStateKalmanFilter(calculation,parameters)
  };

  assert(kalman_filter);
  assert(kalman_filter->GetType() == KalmanFilterType::steady_state);
  return kalman_filter;
}

