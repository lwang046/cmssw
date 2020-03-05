#ifndef HcalSimAlgos_HcalTimingBit_h
#define HcalSimAlgos_HcalTimingBit_h

#include <bitset>

class HcalTimingBit {
   public:

      HcalTimingBit() {};

      std::bitset<4> compute(int ibin, HcalUpgradeTriggerPrimitiveDigi& TPDigi) const;

   private:

};

#endif
