#include <cmath>
#include <algorithm>
#include <vector>
#include <Randomize.hh>
#include <RAT/PDFPMTTime.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>

using namespace std;

namespace RAT {

PDFPMTTime::PDFPMTTime(string pmt_model) {
    DBLinkPtr model = DB::Get()->GetLink("PMTTRANSIT",pmt_model);
    info << "Setting up PDF PMTTime model for " << pmt_model << endl;
    
    fTime = model->GetDArray("time");
    fTimeProb = model->GetDArray("time_prob");
    fCableDelay = model->GetD("cable_delay");
    
    if (fTime.size() != fTimeProb.size()) 
        Log::Die("PDFPMTTime: time and probability arrays of different length");
    if (fTime.size() < 2) 
        Log::Die("PDFPMTTime: cannot define a PDF with fewer than 2 points");
        
    double integral = 0.0;
    fTimeProbCumu = vector<double>(fTime.size()-1);
    fTimeProbCumu[0] = 0.0; 
    for (size_t i = 0; i < fTime.size()-1; i++) {
        integral += (fTime[i+1]-fTime[i])*(fTimeProb[i]+fTimeProb[i+1])/2.0; //trapazoid integration
        fTimeProbCumu[i+1] = integral;
    }
    for (size_t i = 0; i < fTime.size(); i++) {
        fTimeProb[i] /= integral;
        fTimeProbCumu[i] /= integral;
    }
    
}

PDFPMTTime::~PDFPMTTime() {}

double PDFPMTTime::PickTime(double time) const {
    double rval = G4UniformRand();
    for (size_t i = 1; i < fTime.size(); i++) {
        if (rval <= fTimeProbCumu[i]) {
            return fCableDelay + (rval - fTimeProbCumu[i-1])*(fTime[i]-fTime[i-1])/(fTimeProbCumu[i]-fTimeProbCumu[i-1]) + fTime[i-1]; //linear interpolation
        }
    }
    Log::Die("Sans cosmis ray bit flips, cannot get here");
    return 0.0;
}
  
} // namespace RAT

