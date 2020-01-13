#ifndef __L1Analysis_L1AnalysisCaloTPDataFormat_H__
#define __L1Analysis_L1AnalysisCaloTPDataFormat_H__

//-------------------------------------------------------------------------------
// Created 20/04/2010 - E. Conte, A.C. Le Bihan
// 
// 
// Original code : L1Trigger/L1TNtuples/L1NtupleProducer
//-------------------------------------------------------------------------------

#include <vector>

namespace L1Analysis
{
  struct L1AnalysisCaloTPDataFormat
  {
    L1AnalysisCaloTPDataFormat(){Reset();};
    ~L1AnalysisCaloTPDataFormat(){};
    
    
    void Reset() {
      nHCALTP = 0;
      hcalTPieta.clear();
      hcalTPiphi.clear();
      hcalTPCaliphi.clear();
      hcalTPet.clear();
      hcalTPcompEt.clear();
      hcalTPfineGrain.clear();
      // Hcal depth info
      hcalTPnDepths.clear();
      hcalTPDepth1.clear();
      hcalTPDepth2.clear();
      hcalTPDepth3.clear();
      hcalTPDepth4.clear();
      hcalTPDepth5.clear();
      hcalTPDepth6.clear();
      hcalTPDepth7.clear();
      // Hcal timing info

      hcalTPtiming1.clear();
      hcalTPtiming2.clear();
      hcalTPtiming3.clear();
      hcalTPtiming4.clear();
      hcalTPtiming5.clear();
      hcalTPtiming6.clear();
      hcalTPtiming7.clear();

      nECALTP = 0;
      ecalTPieta.clear();
      ecalTPiphi.clear();
      ecalTPCaliphi.clear();
      ecalTPet.clear();
      ecalTPcompEt.clear();
      ecalTPfineGrain.clear();
    }
    
    void Init() {

    }
    

    short nHCALTP;
    std::vector<short> hcalTPieta;
    std::vector<short> hcalTPiphi;
    std::vector<short> hcalTPCaliphi;
    std::vector<float> hcalTPet;
    std::vector<short> hcalTPcompEt;
    std::vector<short> hcalTPfineGrain;
    // Hcal dpeth
    std::vector<short> hcalTPnDepths;
    std::vector<float> hcalTPDepth1;
    std::vector<float> hcalTPDepth2;
    std::vector<float> hcalTPDepth3;
    std::vector<float> hcalTPDepth4;
    std::vector<float> hcalTPDepth5;
    std::vector<float> hcalTPDepth6;
    std::vector<float> hcalTPDepth7;
    // Hcal timing

    std::vector<double> hcalTPtiming1;
    std::vector<double> hcalTPtiming2;
    std::vector<double> hcalTPtiming3;
    std::vector<double> hcalTPtiming4;
    std::vector<double> hcalTPtiming5;
    std::vector<double> hcalTPtiming6;
    std::vector<double> hcalTPtiming7;

    short nECALTP;
    std::vector<short> ecalTPieta;
    std::vector<short> ecalTPiphi;
    std::vector<short> ecalTPCaliphi;
    std::vector<float> ecalTPet;
    std::vector<short> ecalTPcompEt;
    std::vector<short> ecalTPfineGrain;
    
  }; 
} 
#endif

