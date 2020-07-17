#ifndef HcalSimAlgos_HcalTimingBit_h
#define HcalSimAlgos_HcalTimingBit_h

#include <bitset>
#include "DataFormats/HcalDigi/interface/HcalUpgradeTriggerPrimitiveDigi.h"

class HcalTimingBit {
   public:

      HcalTimingBit() {};

      std::bitset<3> compute(int ibin, HcalUpgradeTriggerPrimitiveDigi& TPDigi) const;

   private:

};

#endif
