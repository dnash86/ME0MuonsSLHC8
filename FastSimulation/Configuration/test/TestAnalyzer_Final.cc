#include <RecoMuon/MuonIdentification/src/ME0SegmentMatcher.h>

#include <FWCore/PluginManager/interface/ModuleDef.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include <DataFormats/Common/interface/Handle.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/MessageLogger/interface/MessageLogger.h> 

#include <Geometry/Records/interface/MuonGeometryRecord.h>

#include <DataFormats/MuonReco/interface/EmulatedME0Segment.h>
#include <DataFormats/MuonReco/interface/EmulatedME0SegmentCollection.h>

#include <DataFormats/MuonReco/interface/ME0Muon.h>
#include <DataFormats/MuonReco/interface/ME0MuonCollection.h>

// #include "CLHEP/Matrix/SymMatrix.h"
// #include "CLHEP/Matrix/Matrix.h"
// #include "CLHEP/Vector/ThreeVector.h"

#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
//#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "TLorentzVector.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"

#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
//#include "TRandom3.h"
#include "DataFormats/GeometrySurface/interface/Plane.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixStateInfo.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"


#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"

#include "TMath.h"
#include "TLorentzVector.h"

#include "TH1.h" 
#include "TFile.h"
#include <TProfile.h>


class TestAnalyzer_Final : public edm::EDAnalyzer {
public:
  explicit TestAnalyzer_Final(const edm::ParameterSet&);
  ~TestAnalyzer_Final();


  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();
  void beginJob();

  //protected:
  
  private:
  TFile* histoFile;
  TH1F *Candidate_Eta;  TH1F *Mass_h;  int NumCands; int NumSegs;
  TH1F *Segment_Eta;  TH1F *Track_Eta;  TH1F *Muon_Eta; TH1F *RealMuon_Eta;  TH1F *MuonTaggingEff_h;
  int TrackCount;
//Removing this
};

TestAnalyzer_Final::TestAnalyzer_Final(const edm::ParameterSet& iConfig) 
{
  histoFile = new TFile(iConfig.getParameter<std::string>("HistoFile").c_str(), "recreate");
  NumCands = 0;
  NumSegs = 0;
  TrackCount = 0;
}



void TestAnalyzer_Final::beginJob()
{
  Candidate_Eta = new TH1F("Candidate_Eta"      , "Candidate #eta"   , 40, 2.2, 4.2 );
  Track_Eta = new TH1F("Track_Eta"      , "Track #eta"   , 40, 2.2, 4.2 );
  Segment_Eta = new TH1F("Segment_Eta"      , "Segment #eta"   , 40, 2.2, 4.2 );
  Muon_Eta = new TH1F("Muon_Eta"      , "Muon #eta"   , 40, 2.2, 4.2 );
  RealMuon_Eta = new TH1F("RealMuon_Eta"      , "Muon #eta"   , 40, 2.2, 4.2 );
  Mass_h = new TH1F("Mass_h"      , "Mass"   , 100, 0., 200 );
  MuonTaggingEff_h = new TH1F("MuonTaggingEff_h"      , "Tagging Efficiency"   ,40, 2.2, 4.2  );
}


TestAnalyzer_Final::~TestAnalyzer_Final(){}

void
TestAnalyzer_Final::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)

{

  using namespace edm;

  //run_ = (int)iEvent.id().run();
  //event_ = (int)iEvent.id().event();


    //David's functionality
    

  using namespace reco;

  // Handle <ME0MuonCollection > OurMuons;
  // iEvent.getByLabel <ME0MuonCollection> ("me0SegmentMatcher", OurMuons);

  Handle <std::vector<RecoChargedCandidate> > OurCandidates;
  iEvent.getByLabel <std::vector<RecoChargedCandidate> > ("me0MuonConverter", OurCandidates);


  Handle<std::vector<EmulatedME0Segment> > OurSegments;
  iEvent.getByLabel<std::vector<EmulatedME0Segment> >("me0SegmentProducer", OurSegments);


  Handle<GenParticleCollection> genParticles;
  iEvent.getByLabel<GenParticleCollection>("genParticles", genParticles);

  unsigned int gensize=genParticles->size();
  for(unsigned int i=0; i<gensize; ++i) {
    const reco::GenParticle& CurrentParticle=(*genParticles)[i];
    if ( (CurrentParticle.status()==1) && ( (CurrentParticle.pdgId()==13)  || (CurrentParticle.pdgId()==-13) ) ){  
      RealMuon_Eta->Fill(CurrentParticle.eta());
    }
  }
  //unsigned int recosize=OurCandidates->size();
  for (std::vector<EmulatedME0Segment>::const_iterator thisSegment = OurSegments->begin();
       thisSegment != OurSegments->end();++thisSegment){
    // double theta = atan(thisSegment->localDirection().y()/ thisSegment->localDirection().x());
    // double tempeta = -log(tan (theta/2.));
    LocalVector TempVect(thisSegment->localDirection().x(),thisSegment->localDirection().y(),thisSegment->localDirection().z());
    Segment_Eta->Fill(TempVect.eta());
    NumSegs++;
  }

  Handle <TrackCollection > generalTracks;
  iEvent.getByLabel <TrackCollection> ("generalTracks", generalTracks);
  
  for (std::vector<Track>::const_iterator thisTrack = generalTracks->begin();
       thisTrack != generalTracks->end();++thisTrack){
    Track_Eta->Fill(thisTrack->eta());
    if ( (thisTrack->eta() > 2.4) && (thisTrack->eta() < 4.0)) TrackCount++;
    std::cout<<thisTrack->eta()<<std::endl;
  }

  Handle <std::vector<ME0Muon> > OurMuons;
  iEvent.getByLabel <std::vector<ME0Muon> > ("me0SegmentMatcher", OurMuons);

  for (std::vector<ME0Muon>::const_iterator thisMuon = OurMuons->begin();
       thisMuon != OurMuons->end(); ++thisMuon){
    TrackRef tkRef = thisMuon->innerTrack();
    LocalVector TempVect(tkRef->px(),tkRef->py(),tkRef->pz());
    Muon_Eta->Fill(TempVect.eta());
  }
  //std::cout<<recosize<<std::endl;
  for (std::vector<RecoChargedCandidate>::const_iterator thisCandidate = OurCandidates->begin();
       thisCandidate != OurCandidates->end(); ++thisCandidate){
    NumCands++;
    TLorentzVector CandidateVector;
    CandidateVector.SetPtEtaPhiM(thisCandidate->pt(),thisCandidate->eta(),thisCandidate->phi(),0);
    //std::cout<<"On a muon"<<std::endl;
    //std::cout<<thisCandidate->eta()<<std::endl;
    Candidate_Eta->Fill(thisCandidate->eta());
  }

  if (OurCandidates->size() == 2){
    TLorentzVector CandidateVector1,CandidateVector2;
    CandidateVector1.SetPtEtaPhiM((*OurCandidates)[0].pt(),(*OurCandidates)[0].eta(),(*OurCandidates)[0].phi(),0);
    CandidateVector2.SetPtEtaPhiM((*OurCandidates)[1].pt(),(*OurCandidates)[1].eta(),(*OurCandidates)[1].phi(),0);
    Double_t Mass = (CandidateVector1+CandidateVector2).M();
    Mass_h->Fill(Mass);
  }
  
  
}

void TestAnalyzer_Final::endJob() 
{
  histoFile->cd();
  Candidate_Eta->Write();
  Track_Eta->Write();
  Segment_Eta->Write();
  Muon_Eta->Sumw2();
  RealMuon_Eta->Sumw2();
  Track_Eta->Sumw2();
  //MuonTaggingEff_h->Add(Muon_Eta);
  //MuonTaggingEff_h->Divide(Track_Eta);
  MuonTaggingEff_h->Divide(Muon_Eta,Track_Eta, 1, 1, "B");
  //MuonTaggingEff_h->Sumw2();
  MuonTaggingEff_h->Write();
  Muon_Eta->Write();
  RealMuon_Eta->Write();
  Mass_h->Write();
  std::cout<<NumCands<<std::endl;
  std::cout<<NumSegs<<std::endl;
  std::cout<<TrackCount<<std::endl;
  delete histoFile; histoFile = 0;
}

DEFINE_FWK_MODULE(TestAnalyzer_Final);
