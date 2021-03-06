#ifndef RecoEcal_EgammaCoreTools_EcalClusterEnergyCorrectionBaseClass_h
#define RecoEcal_EgammaCoreTools_EcalClusterEnergyCorrectionBaseClass_h

/** \class EcalClusterEnergyCorrectionBaseClass
  *  Function to correct cluster for the so called local containment
  *
  *  $Id: EcalClusterEnergyCorrectionBaseClass.h
  *  $Date:
  *  $Revision:
  *  \author Yurii Maravin, KSU, March 20, 2009
  */

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterFunctionBaseClass.h"

//#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "CondFormats/EcalObjects/interface/EcalClusterEnergyCorrectionParameters.h"

#include "DataFormats/EgammaReco/interface/BasicCluster.h"

namespace edm {
  class EventSetup;
  class ParameterSet;
}  // namespace edm

class EcalClusterEnergyCorrectionBaseClass : public EcalClusterFunctionBaseClass {
public:
  EcalClusterEnergyCorrectionBaseClass();
  EcalClusterEnergyCorrectionBaseClass(const edm::ParameterSet &){};
  ~EcalClusterEnergyCorrectionBaseClass() override;

  // get/set explicit methods for parameters
  const EcalClusterEnergyCorrectionParameters *getParameters() const { return params_; }
  // check initialization
  void checkInit() const;

  // compute the correction
  float getValue(const reco::BasicCluster &, const EcalRecHitCollection &) const override = 0;
  float getValue(const reco::SuperCluster &, const int mode) const override = 0;

  // set parameters
  void init(const edm::EventSetup &es) override;

protected:
  edm::ESHandle<EcalClusterEnergyCorrectionParameters> esParams_;
  const EcalClusterEnergyCorrectionParameters *params_;
};

#endif
