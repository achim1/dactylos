#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iterator>

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMath.h"
#include "TString.h"
#include "TRegexp.h"
#include <TLeaf.h>
#include <TApplication.h>
#include <Rtypes.h>
#include <TSystem.h>
#include <TLine.h>
#include <TStyle.h>

#include <string.h>
using namespace std;

double myfun(double *xx, double *par)
{
    double E = xx[0];
    double c = par[0];
    double alpha = par[1];
    double mu = par[2];
    double sigma = par[3];
    double beta = par[4];
    double lambda = par[5];
    double pi = 3.141592654;

    double gaus = alpha/sigma/sqrt(2*pi)*exp(-0.5*(E-mu)*(E-mu)/sigma/sigma); //Gaussian term

    double exp_c = (beta)*lambda*exp((sigma*sigma*lambda*lambda+2*lambda*E)/2)/(exp(lambda*mu)-1);
    double exp = TMath::Erf((mu-E-sigma*sigma*lambda)/(TMath::Sqrt(2)*sigma))-TMath::Erf((-E-sigma*sigma*lambda)/sqrt(2)/sigma); // exponential term

    double constant = (1-alpha-beta)/mu*(TMath::Erf((mu-E)/sqrt(2)/sigma)-TMath::Erf(-E/sqrt(2)/sigma)); // constant term

    return c*(gaus+exp_c*exp+constant);
}

int main(int argc, char** argv)
{
    if(argc!=8)
    {
        cout<<"Syntax: "<<argv[0]<<" [input file] [qcut_lower] [qcut_upper] [hcut] [xray energy] [plot file] [file.txt]"<<endl;
        exit(1);
    }

    TFile *file;
    TTree *t;
    file = new TFile(argv[1],"read");
//    file = new TFile("/home/mengjiao/Workspace/xrays_spec/outroot/08-07-2019/4.0us/det3_A_4.0us.root","read");
	t = (TTree*)file->Get("out_tree");

    double qcut_lower = atof(argv[2]);
    double qcut_upper = atof(argv[3]);
    double hcut = atof(argv[4]);

    // get the maximum bin
    double chans;
    double weight;
    t->SetBranchAddress("chans",&chans);
    t->SetBranchAddress("weight",&weight);
    int nevts = t->GetEntries();
    double mu = 0;
    for (int i=0;i<nevts;i++)
    {
        t->GetEntry(i);
        if (chans<qcut_lower||chans>qcut_upper||weight<hcut) continue;
        else
        {
            hcut = weight;
            mu = chans;
        }
    }
//    cout<<"max bin: "<<mu<<"peak height: "<<hcut<<endl;

    gStyle->SetOptFit(1);
    gStyle->SetStatX(0.9);
    gStyle->SetStatY(0.9);
    gStyle->SetStatW(0.14);
    gStyle->SetStatH(0.2);

    //pre-fitting on main peak
    t->Draw("chans>>h1(1000,500,2500)","weight");
    TH1F *h1 = (TH1F*)gDirectory->Get("h1");
    h1->Fit("gaus","","same",mu*0.9, mu*1.1);
    TF1* fun = h1->GetFunction("gaus");
    double C = fun->GetParameter(0);
    double mean = fun->GetParameter(1);
    double sigma = fun->GetParameter(2);
    cout<<"============"<<mean<<" "<<sigma<<"============"<<endl;

    // pre-fitting on compton tail
    t->Draw("chans>>h0(1000,500,2500)","weight");
    TH1F *h0 = (TH1F*)gDirectory->Get("h0");
    h0->Fit("gaus","","same",mu*0.6, mu*0.9);
    TF1* fun0 = h0->GetFunction("gaus");
    double C0 = fun0->GetParameter(0);
    double mean0 = fun0->GetParameter(1);
    double sigma0 = fun0->GetParameter(2);
    cout<<"============"<<mean0<<" "<<sigma0<<"============"<<endl;

    //full fitting: initial parameter values come from pre-fitting
    TF1 *frg = new TF1("frg","gaus(0)+gaus(3)",0,10000);
    TF1 *fre = new TF1("fre","[0]*TMath::Erfc((x-[1])/sqrt(2)/[2])+gaus(3)",0,10000);
//    TF1 *f = new TF1("f",myfun,0,10000,6);
    double q0, q1, q2, q3, q4, q5;
    if (mean0>0.3*mu&&mean0<0.9*mu)
    {
        q0 = C0;
        q1 = mean0;
        q2 = sigma0;
    }
    else
    {
        q0 = 10.;
        q1 = mu*0.8;
        q2 = q1*0.1;
    }
    if (mean>0.8*mu&&mean<1.1*mu)
    {
        q3 = C;
        q4 = mean;
        q5 = sigma;
    }
    else
    {
        q3 = 100.;
        q4 = mu;
        q5 = q4*0.01;
    }
    frg->SetParameters(q0, q1, q2, q3, q4, q5);
    frg->SetParNames("C_{0}","#mu_{0}","#sigma_{0}","C","#mu","#sigma");
//    f->SetParameters(C*sqrt(2*3.14159)*sigma,0.9,mean,sigma,0.01,0.01,0.02);
//    f->SetParNames("C","#alpha","#mu","#sigma","#beta","#lambda");
    TCanvas *c1 = new TCanvas("c1","c1",1500,1200);
    gStyle->SetOptTitle(0);
    c1->Divide(2,2);
    c1->cd(1);
    t->Draw("chans>>hraw(500,500,2500)","weight");
    TH1F *hraw = (TH1F*)gDirectory->Get("hraw");
    hraw->SetLineWidth(0);
    hraw->SetMarkerColor(1);
    hraw->SetMarkerStyle(2);
	hraw->Fit(frg,"","same", q1-1.0*q2, q4+5.*q5);

    c1->cd(2);
    t->Draw("chans>>hraw2(1000,500,2500)","weight");
    fre->SetParameters(0.1*q3, 0.85*q4, 10, q3, q4, q5);
    fre->SetParNames("C_{0}","#mu_{0}","#sigma_{0}","C","#mu","#sigma");
    TH1F *hraw2 = (TH1F*)gDirectory->Get("hraw2");
    hraw2->SetLineWidth(0);
    hraw2->SetMarkerColor(4);
    hraw2->SetMarkerStyle(2);
    hraw2->Fit(fre,"","same", 0.85*q4, q4+5.*q5);

    double Exray = atof(argv[5]);
    double Efactor = 0.;
    double nfactor = frg->GetParameter(4);
    if (nfactor>0.9*mu&&nfactor<1.1*mu)
    {
        Efactor = Exray/nfactor;
    }
    else
    {
        Efactor = Exray/mu;
    }
//    cout<<"Efactor: "<<Efactor<<endl;

    double energy;
    double w;
    TFile *fout=new TFile("tmp.root","recreate");
    TTree *tnew = new TTree("tnew","");
    tnew->Branch("energy",&energy, "energy/D");
    tnew->Branch("w",&w, "w/D");
    for (int ii=0;ii<nevts;ii++)
    {
        t->GetEntry(ii);
        energy = chans*Efactor;
        w = weight;
        tnew->Fill();
    }
    tnew->Write();
    fout->Close();

    TFile *file_tmp;
    file_tmp = new TFile("tmp.root","read");
    c1->cd(3);
//    file_tmp = new TFile("tmp.root","read");
    tnew = (TTree*)file_tmp->Get("tnew");
    tnew->Draw("energy>>hcal(400,25,125)","w");
    TH1F *hcal = (TH1F*)gDirectory->Get("hcal");
    hcal->SetLineWidth(0);
    hcal->SetMarkerColor(1);
    hcal->SetMarkerStyle(2);
    double p0, p1, p2, p3, p4, p5;
    if (mean0*Efactor>0.3*Exray&&mean0*Efactor<0.9*Exray)
    {
        p0 = C0/2.;
        p1 = mean0*Efactor;
        p2 = sigma0*Efactor;
    }
    else
    {
        p0 = 10.;
        p1 = Exray*0.75;
        p2 = 3.;
    }
    if (mean*Efactor>0.9*Exray&&mean*Efactor<1.1*Exray)
    {
        p3 = C/2.;
        p4 = mean*Efactor;
        p5 = sigma*Efactor;
    }
    else
    {
        p3 = 200.;
        p4 = Exray;
        p5 = 1.5;
    }
//    f->SetParameters(C0, mean0*Efactor, sigma0*Efactor, C, mean*Efactor, sigma*Efactor);

    TF1 *fcg = new TF1("fcg","gaus(0)+gaus(3)",0,200);
    TF1 *fce = new TF1("fce","[0]*TMath::Erfc((x-[1])/sqrt(2)/[2])+gaus(3)",0,10000);
    fcg->SetParameters(p0, p1, p2, p3, p4, p5);
    fce->SetParameters(10, Exray*0.8, 1, p3, p4, p5);
    fcg->SetParNames("C_{0}","#mu_{0}","#sigma_{0}","C","#mu","#sigma");
    fce->SetParNames("C_{0}","#mu_{0}","#sigma_{0}","C","#mu","#sigma");
    hcal->Fit(fcg,"","same", p1-1*p2, p4+3.5*p5);

    c1->cd(4);
    tnew->Draw("energy>>hcal2(400,25,125)","w");
    TH1F *hcal2 = (TH1F*)gDirectory->Get("hcal2");
    hcal2->SetLineWidth(0);
    hcal2->SetMarkerColor(4);
    hcal2->SetMarkerStyle(2);
    hcal2->Fit(fce,"","same", p4*0.85, p4+5*p5);

    c1->Print(argv[6]);
    file_tmp->Delete();

    ofstream resfile;
    resfile.open(argv[7]);
    string path = argv[7];
    int pos=path.find_last_of('/');
    string ss(path.substr(pos+1));
//    cout<<ss<<endl;
    string det= ss.substr(0,6);
    string strip= ss.substr(7,1);
    string peakingtime= ss.substr(14,ss.length()-14-10);
//    cout<<det<<" "<<strip<<" "<<peakingtime<<endl;

//%%%%%%%%%%%%%%%%%%% choose the best fitting results %%%%%%%%%%%%//
    double mu_fr =0.;
    double sig_fr =0.;
    double sig_fr_err =0.;
    double mu_frg = frg->GetParameter(4);
    double sig_frg = frg->GetParameter(5);
    double sig_frg_err = frg->GetParError(5);
    double mu_fre = fre->GetParameter(4);
    double sig_fre = fre->GetParameter(5);
    double sig_fre_err = fre->GetParError(5);
    if (abs(sig_frg/mu_frg)>abs(sig_fre/mu_fre))
    {
        mu_fr = mu_fre;
        sig_fr = sig_fre;
        sig_fr_err = sig_fre_err;
    }
    else
    {
        mu_fr = mu_frg;
        sig_fr = sig_frg;
        sig_fr_err = sig_frg_err;
    }
    cout<<"sig_fr: "<<sig_fr<<" "<<"mu_fr: "<<mu_fr<<" "<<"res: "<<sig_fr/mu_fr<<endl;

    double mu_fc =0.;
    double sig_fc =0.;
    double sig_fc_err =0.;
    double mu_fcg = fcg->GetParameter(4);
    double sig_fcg = fcg->GetParameter(5);
    double sig_fcg_err = fcg->GetParError(5);
    double mu_fce = fce->GetParameter(4);
    double sig_fce = fce->GetParameter(5);
    double sig_fce_err = fce->GetParError(5);
    if (abs(sig_fcg/mu_fcg)>abs(sig_fce/mu_fce))
    {
        mu_fc = mu_fce;
        sig_fc = sig_fce;
        sig_fc_err = sig_fce_err;
    }
    else
    {
        mu_fc = mu_fcg;
        sig_fc = sig_fcg;
        sig_fc_err = sig_fcg_err;
    }
    cout<<"sig_fc: "<<sig_fc<<" "<<"mu_fc: "<<mu_fc<<" "<<"res: "<<sig_fc/mu_fc<<endl;

    double mu_fit;
    double sig_fit;
    double sig_fit_err;
    if (abs(sig_fr/mu_fr)>abs(sig_fc/mu_fc))
    {
        mu_fit = mu_fc;
        sig_fit = sig_fc;
        sig_fit_err = sig_fc_err;
    }
    else
    {
        mu_fit = mu_fr*Efactor;
        sig_fit = sig_fr*Efactor;
        sig_fit_err = sig_fr_err*Efactor;
    }

    double fwhm = sig_fit/mu_fit*Exray*2.35;
    double fwhm_err = sig_fit_err/mu_fit*Exray;
    resfile<<det<<" "<<strip<<" "<<peakingtime<<" "<<mu_fit<<" "<<sig_fit<<" "<<fwhm<<" "<<fwhm_err<<endl;
    resfile.close();

    return 0;
}
