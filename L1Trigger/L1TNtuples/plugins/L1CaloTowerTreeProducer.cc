// -*- C++ -*-
//
// Package:    L1Trigger/L1Ntuples
// Class:      L1CaloTowerTreeProducer
// 
/**\class L1CaloTowerTreeProducer L1CaloTowerTreeProducer.cc L1TriggerDPG/L1Ntuples/src/L1CaloTowerTreeProducer.cc

Description: Produce L1 Extra tree

Implementation:
     
*/
//
// Original Author:  
//         Created:  
// $Id: L1CaloTowerTreeProducer.cc,v 1.8 2012/08/29 12:44:03 jbrooke Exp $
//
//


// system include files
#include <memory>

// framework
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

// data formats
#include "DataFormats/L1Trigger/interface/EGamma.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/L1Trigger/interface/Jet.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1Trigger/interface/EtSum.h"

// cond formats
#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"

// ROOT output stuff
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"

#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloTPDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1CaloTowerDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1CaloClusterDataFormat.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalUpgradeTriggerPrimitiveDigi.h"

#include "DataFormats/L1TCalorimeter/interface/CaloTower.h"
#include "DataFormats/L1TCalorimeter/interface/CaloCluster.h"


//
// class declaration
//

class L1CaloTowerTreeProducer : public edm::EDAnalyzer {
public:
  explicit L1CaloTowerTreeProducer(const edm::ParameterSet&);
  ~L1CaloTowerTreeProducer() override;
  
  
private:
  void beginJob(void) override ;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

public:
  
  L1Analysis::L1AnalysisCaloTPDataFormat* caloTPData_;
  L1Analysis::L1AnalysisL1CaloTowerDataFormat* l1CaloTowerData_;
  L1Analysis::L1AnalysisL1CaloClusterDataFormat* l1CaloClusterData_;

private:

  double ecalLSB_;
  unsigned maxCaloTP_;
  unsigned maxL1Tower_;
  unsigned maxL1Cluster_;

  // output file
  edm::Service<TFileService> fs_;
  
  // tree
  TTree * tree_;

  // EDM input tags
  edm::EDGetTokenT<EcalTrigPrimDigiCollection> ecalToken_;
  edm::EDGetTokenT<HcalUpgradeTrigPrimDigiCollection> hcalToken_;
  edm::EDGetTokenT<l1t::CaloTowerBxCollection> l1TowerToken_;
  edm::EDGetTokenT<l1t::CaloClusterBxCollection> l1ClusterToken_;

  bool storeCaloClusters_;
};



L1CaloTowerTreeProducer::L1CaloTowerTreeProducer(const edm::ParameterSet& iConfig)
{

  ecalToken_ = consumes<EcalTrigPrimDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("ecalToken"));
  hcalToken_ = consumes<HcalUpgradeTrigPrimDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("hcalToken"));
  l1TowerToken_ = consumes<l1t::CaloTowerBxCollection>(iConfig.getUntrackedParameter<edm::InputTag>("l1TowerToken"));

  edm::InputTag clusterTag = iConfig.getUntrackedParameter<edm::InputTag>("l1ClusterToken");
  storeCaloClusters_=true;
  if ( clusterTag.label() == std::string("") or clusterTag.label() == std::string("none")) storeCaloClusters_=false;
  
  if (clusterTag.instance() != std::string(""))
    l1ClusterToken_ = consumes<l1t::CaloClusterBxCollection>(clusterTag);
 
  ecalLSB_      = iConfig.getUntrackedParameter<double>("ecalLSB", 0.5);
  maxCaloTP_    = iConfig.getUntrackedParameter<unsigned int>("maxCaloTP", 5760);
  maxL1Tower_   = iConfig.getUntrackedParameter<unsigned int>("maxL1Tower", 5760);
  maxL1Cluster_ = iConfig.getUntrackedParameter<unsigned int>("maxL1Cluster", 5760);
 
  // set up output
  tree_=fs_->make<TTree>("L1CaloTowerTree", "L1CaloTowerTree");
  tree_->Branch("CaloTP", "L1Analysis::L1AnalysisCaloTPDataFormat", &caloTPData_, 32000, 3);
  tree_->Branch("L1CaloTower", "L1Analysis::L1AnalysisL1CaloTowerDataFormat", &l1CaloTowerData_, 32000, 3);

  if (storeCaloClusters_)
    tree_->Branch("L1CaloCluster", "L1Analysis::L1AnalysisL1CaloClusterDataFormat", &l1CaloClusterData_, 32000, 3);

  caloTPData_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  l1CaloTowerData_ = new L1Analysis::L1AnalysisL1CaloTowerDataFormat();
  l1CaloClusterData_ = new L1Analysis::L1AnalysisL1CaloClusterDataFormat();

}


L1CaloTowerTreeProducer::~L1CaloTowerTreeProducer()
{
 
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
L1CaloTowerTreeProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  // do calo towers
  caloTPData_->Reset();

  edm::ESHandle<CaloTPGTranscoder> decoder;
  iSetup.get<CaloTPGRecord>().get(decoder);

  edm::Handle<EcalTrigPrimDigiCollection> ecalTPs;
  edm::Handle<HcalUpgradeTrigPrimDigiCollection> hcalTPs;

  iEvent.getByToken(ecalToken_, ecalTPs);
  iEvent.getByToken(hcalToken_, hcalTPs);

  if (ecalTPs.isValid()){ 

    for ( auto itr : *(ecalTPs.product()) ) {

      short ieta = (short) itr.id().ieta();
      //      unsigned short absIeta = (unsigned short) abs(ieta);
      //      short sign = ieta/absIeta;
      
      unsigned short cal_iphi = (unsigned short) itr.id().iphi();
      unsigned short iphi = (72 + 18 - cal_iphi) % 72;
      unsigned short compEt = itr.compressedEt();
      double et = ecalLSB_ * compEt;
      unsigned short fineGrain = (unsigned short) itr.fineGrain();

      if (compEt > 0) {
	caloTPData_->ecalTPieta.push_back( ieta );
	caloTPData_->ecalTPCaliphi.push_back( cal_iphi );
	caloTPData_->ecalTPiphi.push_back( iphi );
	caloTPData_->ecalTPet.push_back( et );
	caloTPData_->ecalTPcompEt.push_back( compEt );
	caloTPData_->ecalTPfineGrain.push_back( fineGrain );
	caloTPData_->nECALTP++;
      }
    }

  }
  else {
    edm::LogWarning("L1TNtuple") << "ECAL TPs not found, branch will not be filled";
  }
  
  
  if (hcalTPs.isValid()) {
      
    for ( auto itr : (*hcalTPs.product()) ) {

      int ver = itr.id().version();
      short ieta = (short) itr.id().ieta();
      unsigned short absIeta = (unsigned short) abs(ieta);
      //      short sign = ieta/absIeta;
      
      unsigned short cal_iphi = (unsigned short) itr.id().iphi();
      unsigned short iphi = (72 + 18 - cal_iphi) % 72;
      
      unsigned short compEt = itr.SOI_compressedEt();
      double et = decoder->hcaletValue(itr.id(), itr.t0());

      unsigned short fineGrain = (unsigned short) itr.SOI_fineGrain();
      unsigned short TimingBit = (unsigned short) itr.SOI_timingbit();
     
      unsigned short nDepths = (unsigned short) itr.getDepthData().size();
      float et_sum = 0;

      HcalTrigTowerDetId id = itr.id();
      id = HcalTrigTowerDetId(id.ieta(), id.iphi(), 1, id.version());
      std::vector<int> energy_depth = itr.getDepthData();
      for (int i = 0; i < static_cast<int>(energy_depth.size()); ++i) {
        int depth = energy_depth[i];
        if (depth > 0) {
	  et_sum += depth;}
      }
      float tp_energy_ = decoder->hcaletValue(id, itr.SOI_compressedEt());
      float DepthScale = 0;
      if (et_sum > 0){
        DepthScale = tp_energy_/et_sum;
      }      


      if (compEt > 0 && (absIeta<29 || ver==1)) {

	if (nDepths > 1){
	  float Depth1 = (float) energy_depth[1];
	  caloTPData_->hcalTPDepth1.push_back( Depth1*DepthScale );

	  double t1 = itr.SOI_rising_avg(2);
	  //	  std::cout << "Timing1 = " << t1 << std::endl;
	  if(t1>0) caloTPData_->hcalTPtiming1.push_back( t1 ); //itr.SOI_rising_avg(1) );
	  else caloTPData_->hcalTPtiming1.push_back( -1.0 );
	}
	else{
	  caloTPData_->hcalTPDepth1.push_back( 0 );
	  caloTPData_->hcalTPtiming1.push_back( -1.0 );
	}

	if (nDepths > 2){
	  float Depth2 = (float) energy_depth[2];
	  caloTPData_->hcalTPDepth2.push_back( Depth2*DepthScale );

	  double t2 = itr.SOI_rising_avg(3); 
	  //	  std::cout << "Timing2 = " << t2 << std::endl;
	  if(t2>0) caloTPData_->hcalTPtiming2.push_back( t2 ); //itr.SOI_rising_avg(1) ); 
	  else caloTPData_->hcalTPtiming2.push_back( -1.0 ); 
	  //	  caloTPData_->hcalTPtiming2.push_back( itr.SOI_rising_avg(2) ); 
	}
	else{
	  caloTPData_->hcalTPDepth2.push_back( 0 );
	  caloTPData_->hcalTPtiming2.push_back( -1.0 );  
	}

	if (nDepths > 3){
	  float Depth3 = (float) energy_depth[3];
	  caloTPData_->hcalTPDepth3.push_back( Depth3*DepthScale );
	  double t3 = itr.SOI_rising_avg(4); 
	  if(t3>0) caloTPData_->hcalTPtiming3.push_back( t3 ); 
	  else caloTPData_->hcalTPtiming3.push_back( -1.0 ); 
	  //caloTPData_->hcalTPtiming3.push_back( itr.SOI_rising_avg(3) ); 
	}
	else{
	  caloTPData_->hcalTPDepth3.push_back( 0 );
	  caloTPData_->hcalTPtiming3.push_back( -1.0 );  
	}

	if (nDepths > 4){
	  float Depth4 = (float) energy_depth[4];
	  caloTPData_->hcalTPDepth4.push_back( Depth4*DepthScale );
	  //	  caloTPData_->hcalTPtiming4.push_back( itr.SOI_rising_avg(4) ); 
	  double t4 = itr.SOI_rising_avg(5);  
	  if(t4>0) caloTPData_->hcalTPtiming4.push_back( t4 );  
	  else caloTPData_->hcalTPtiming4.push_back( -1.0 ); 
	}
	else{
	  caloTPData_->hcalTPDepth4.push_back( 0 );
	  caloTPData_->hcalTPtiming4.push_back( -1.0 );  
	}
	if (nDepths > 5){
	  float Depth5 = (float) energy_depth[5];
	  caloTPData_->hcalTPDepth5.push_back( Depth5*DepthScale );
	  double t5 = itr.SOI_rising_avg(6);  
	  if(t5>0) caloTPData_->hcalTPtiming5.push_back( t5 ); 
	  else caloTPData_->hcalTPtiming5.push_back( -1.0 );   
	  //	  caloTPData_->hcalTPtiming5.push_back( itr.SOI_rising_avg(5) ); 
	}
	else{
	  caloTPData_->hcalTPDepth5.push_back( 0 );
	  caloTPData_->hcalTPtiming5.push_back( -1.0 );  
	}

	if (nDepths > 6){
	  float Depth6 = (float) energy_depth[6];
	  caloTPData_->hcalTPDepth6.push_back( Depth6*DepthScale );
	  //	  caloTPData_->hcalTPtiming6.push_back( itr.SOI_rising_avg(6) ); 
	  double t6 = itr.SOI_rising_avg(7); 
	  if(t6>0) caloTPData_->hcalTPtiming6.push_back( t6 );  
	  else caloTPData_->hcalTPtiming6.push_back( -1.0 ); 
	}
	else{
	  caloTPData_->hcalTPDepth6.push_back( 0 );
	  caloTPData_->hcalTPtiming6.push_back( -1.0 );  
	}

	if (nDepths > 7){
	  float Depth7 = (float) energy_depth[7];
	  caloTPData_->hcalTPDepth7.push_back( Depth7*DepthScale );
	  //	  caloTPData_->hcalTPtiming7.push_back( itr.SOI_rising_avg(7) ); 
	  double t7 = itr.SOI_rising_avg(8);
	  if(t7>0) caloTPData_->hcalTPtiming7.push_back( t7 ); 
	  else caloTPData_->hcalTPtiming7.push_back( -1.0 );   
	}
	else{
	  caloTPData_->hcalTPDepth7.push_back( 0 );
	  caloTPData_->hcalTPtiming7.push_back( -1.0 );  
	}


        if (nDepths >2){
	  caloTPData_->hcalTPnDepths.push_back( nDepths );
	}
	else{caloTPData_->hcalTPnDepths.push_back( 0 );}

	caloTPData_->hcalTPieta.push_back( ieta );
	caloTPData_->hcalTPCaliphi.push_back( cal_iphi );
	caloTPData_->hcalTPiphi.push_back( iphi );
	caloTPData_->hcalTPet.push_back( et );
	caloTPData_->hcalTPcompEt.push_back( compEt );
	caloTPData_->hcalTPfineGrain.push_back( fineGrain );
        caloTPData_->hcalTPTimingBit.push_back( TimingBit );
	caloTPData_->nHCALTP++;
      }
    }
    
  }
  else {
    //edm::LogWarning("L1TNtuple") << "HCAL TPs not found, branch will not be filled";
  }
  
  // do L1 towers
  l1CaloTowerData_->Reset();

  edm::Handle<l1t::CaloTowerBxCollection> l1Towers;

  iEvent.getByToken(l1TowerToken_, l1Towers);

  if (l1Towers.isValid()){

    for ( int ibx=l1Towers->getFirstBX(); ibx<=l1Towers->getLastBX(); ++ibx) {

      for ( auto itr = l1Towers->begin(ibx); itr !=l1Towers->end(ibx); ++itr ) {

        if (itr->hwPt()<=0) continue;

	//	l1CaloTowerData_->bx.push_back( ibx );
	l1CaloTowerData_->et.push_back( itr->pt() );
	l1CaloTowerData_->eta.push_back( itr->eta() );
	l1CaloTowerData_->phi.push_back( itr->phi() );
	l1CaloTowerData_->iet.push_back( itr->hwPt() );
	l1CaloTowerData_->ieta.push_back( itr->hwEta() );
	l1CaloTowerData_->iphi.push_back( itr->hwPhi() );
	l1CaloTowerData_->iem.push_back( itr->hwEtEm() );
	l1CaloTowerData_->ihad.push_back( itr->hwEtHad() );
	l1CaloTowerData_->iratio.push_back( itr->hwEtRatio() );
	l1CaloTowerData_->iqual.push_back( itr->hwQual() );

	l1CaloTowerData_->nTower++;

      }

    }

  }
  else {
    edm::LogWarning("L1TNtuple") << "L1 Calo Towerss not found, branch will not be filled";
  }


  // do L1 clusters
  if (storeCaloClusters_){
    l1CaloClusterData_->Reset();

    edm::Handle<l1t::CaloClusterBxCollection> l1Clusters;

    if (!l1ClusterToken_.isUninitialized())
      iEvent.getByToken(l1ClusterToken_, l1Clusters);

    if (l1Clusters.isValid()){

      for ( int ibx=l1Clusters->getFirstBX(); ibx<=l1Clusters->getLastBX(); ++ibx) {

	for ( auto itr = l1Clusters->begin(ibx); itr !=l1Clusters->end(ibx); ++itr ) {

	  if (itr->hwPt()<=0) continue;

	  //	l1CaloClusterData_->bx.push_back( ibx );
	  l1CaloClusterData_->et.push_back( itr->pt() );
	  l1CaloClusterData_->eta.push_back( itr->eta() );
	  l1CaloClusterData_->phi.push_back( itr->phi() );
	  l1CaloClusterData_->iet.push_back( itr->hwPt() );
	  l1CaloClusterData_->ieta.push_back( itr->hwEta() );
	  l1CaloClusterData_->iphi.push_back( itr->hwPhi() );
	  l1CaloClusterData_->iqual.push_back( itr->hwQual() );

	  l1CaloClusterData_->nCluster++;

	}

      }

    }
    else {
      edm::LogWarning("L1TNtuple") << "L1 Calo Clusters not found, branch will not be filled";
    }
  }


  tree_->Fill();

}

// ------------ method called once each job just before starting event loop  ------------
void 
L1CaloTowerTreeProducer::beginJob(void)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
L1CaloTowerTreeProducer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1CaloTowerTreeProducer);
