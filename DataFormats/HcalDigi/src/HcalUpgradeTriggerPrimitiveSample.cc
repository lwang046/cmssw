#include "DataFormats/HcalDigi/interface/HcalUpgradeTriggerPrimitiveSample.h"

//------------------------------------------------------
// Null constructor
//------------------------------------------------------

HcalUpgradeTriggerPrimitiveSample::HcalUpgradeTriggerPrimitiveSample(): theSample(0){}

//------------------------------------------------------
// Constructor where we already know the data word
//------------------------------------------------------

HcalUpgradeTriggerPrimitiveSample::HcalUpgradeTriggerPrimitiveSample(uint32_t data): theSample(data){}

//------------------------------------------------------
// Constructor where we make the data word ourselves
//------------------------------------------------------

HcalUpgradeTriggerPrimitiveSample::HcalUpgradeTriggerPrimitiveSample(int encodedEt, int encodedFG, int slb, int slbchan){
  
  theSample = 
    (((slb         )&0x7 )<<21) | // slb          
    (((slbchan     )&0x3 )<<19) | // slbchan      
    (((encodedFG   )&0xFF)<<8 ) | // fine grain   
    (((encodedEt   )&0xFF)<<0 );  // et           
  
}

HcalUpgradeTriggerPrimitiveSample::HcalUpgradeTriggerPrimitiveSample(int timingbit, int slb, int slbchan){

  theSample &= 
    (((timingbit   )&0xF )<<24  | // timingbit
    (((slb         )&0x7 )<<18) | // slb          
    (((slbchan     )&0x3 )<<16) | // slbchan      

}

//------------------------------------------------------
// Print function for std::cout, etc
//------------------------------------------------------

std::ostream& operator<<(std::ostream& s, const HcalUpgradeTriggerPrimitiveSample& samp) {
  return s << "Et="      << samp.compressedEt()    << ", " 
	   << "FG="      << samp.fineGrain()       << ", "
	   << "timingbit=" << samp.timingbit() ;
}

