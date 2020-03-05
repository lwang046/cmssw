#include "SimCalorimetry/HcalTrigPrimAlgos/interface/HcalTimingBit.h"

#include <cassert>

std::bitset<4> HcalTimingBit::compute(int ibin, HcalUpgradeTriggerPrimitiveDigi& digi) const
{
      std::bitset<4> result;

      std::vector<int> energy_depth = digi.getDepthData();

      for (int i = 0; i < static_cast<int>(energy_depth.size()); ++i) {
        


      }

      result[0] = 1;
      result[1] = 0;
      result[2] = 1;
      result[3] = 0;

      return result;
}
