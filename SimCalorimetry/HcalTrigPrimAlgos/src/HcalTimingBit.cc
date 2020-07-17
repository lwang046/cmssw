#include "SimCalorimetry/HcalTrigPrimAlgos/interface/HcalTimingBit.h"
#include "DataFormats/HcalDetId/interface/HcalTrigTowerDetId.h"

#include <cassert>

std::bitset<3> HcalTimingBit::compute(int ibin, HcalUpgradeTriggerPrimitiveDigi& digi) const
{
      //std::bitset<3> result;

      std::vector<int> energy_depth = digi.getDepthData();
      int N_cell=0;
      double depth_tdc_rise = -99.;

      //HcalTrigTowerDetId id = digi.id(); 
      //id = HcalTrigTowerDetId(id.ieta(), id.iphi(), 1, id.version());
      //int tp_ieta_ = id.ieta();
      //int tp_iphi_ = id.iphi();

      for (int i = 0; i < static_cast<int>(energy_depth.size()); ++i) {
        int depth_energy = energy_depth[i];
        depth_tdc_rise = digi.SOI_rising_avg(i+1);

        if(depth_tdc_rise >= 5 && depth_energy >= 50) N_cell++;
      }

      return N_cell;
}
