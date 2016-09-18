#ifndef KALMANFILTERCALCULATIONELEMENTSFACTORY_H
#define KALMANFILTERCALCULATIONELEMENTSFACTORY_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <boost/shared_ptr.hpp>
#pragma GCC diagnostic pop

#include "kalmanfiltercalculationelements.h"

namespace ribi {
namespace kalman {

struct KalmanFilterCalculationElementsFactory
{
  KalmanFilterCalculationElementsFactory();

  ///Create a KalmanFilterCalculationElements of a certain type
  boost::shared_ptr<KalmanFilterCalculationElements> Create(
    const KalmanFilterType type
  ) const noexcept;

  ///Deep copy
  boost::shared_ptr<KalmanFilterCalculationElements> DeepCopy(
    const boost::shared_ptr<KalmanFilterCalculationElements>& elements
  ) const noexcept;

};

} //~namespace kalman
} //~namespace ribi

#endif // KALMANFILTERCALCULATIONELEMENTSFACTORY_H
