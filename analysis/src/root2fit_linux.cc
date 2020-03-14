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
#include <TF1.h>
#include <TText.h>

using namespace std;

double data[10000][2];

//int main(int argc, char** argv)
void root2fit_linux()
{
       //TFile *fin = new TFile("/home/mjxiao/auto_calib/ana_chain/data/03062020_det159/test4000.root","read");
       TFile* fin = new TFile("/opt/cranelab/src/python/dactylos/03062020_det159/test4000.root");
//	if (argc!=3)
//	{
//		cout<<"Syntax: "<<argv[0]<<" "<<"[input file] [output file]"<<endl;
//		exit(1);
//	}
//	TString infile = TString(argv[1]);
//	TString outfile = TString(argv[2]);

    TCanvas *myc= new TCanvas("myc","",1600,800);
    myc->Divide(4,2);
    gStyle->SetOptFit(1);

    TF1 *fun=new TF1("fun","gaus(0)+gaus(3)",0,5000);
    fun->SetParameters(10,900,150,10,1150,25);

//    TFile *fin = new TFile(argv[1],"read");
//    TFile *fin = new TFile("Sh0079_B.root","read");
    myc->cd(1);
    TH1D *a = (TH1D*)gDirectory->Get("ehistch0");
    a->SetTitle("Sh0213_A_-37C");
    a->GetXaxis()->SetRange(10,2000);
//    a->GetYaxis()->SetRange(0,1000);
//    ga->GetHistogram()->GetXaxis()->SetTitle("Peaking Time [#mus]");
//    ga->GetHistogram()->GetYaxis()->SetTitle("FWHM [keV]");
    a->Draw();
    a->Fit("fun","","same",800,1250);

    myc->cd(2);
    TH1D *b = (TH1D*)gDirectory->Get("ehistch1");
    b->SetTitle("Sh0213_B_-37C");
    b->GetXaxis()->SetRange(10,2000);
//    b->GetYaxis()->SetRange(0,1000);
    b->Draw();
    fun->SetParameters(10,900,150,10,1150,25);
    b->Fit("fun","","same",900,1250);

    myc->cd(3);
    TH1D *c = (TH1D*)gDirectory->Get("ehistch2");
    c->SetTitle("Sh0219_A_-37C");
    c->GetXaxis()->SetRange(10,2000);
//    c->GetYaxis()->SetRange(0,1000);
    c->Draw();
    fun->SetParameters(10,900,150,10,1150,25);
    c->Fit("fun","","same",800,1250);

    myc->cd(4);
    TH1D *d = (TH1D*)gDirectory->Get("ehistch3");
    d->SetTitle("Sh0219_B_-37C");
    d->GetXaxis()->SetRange(10,2000);
//    d->GetYaxis()->SetRange(0,1000);
    d->Draw();
    fun->SetParameters(10,900,150,10,1150,25);
    d->Fit("fun","","same",850,1250);

    myc->cd(5);
    TH1D *e = (TH1D*)gDirectory->Get("ehistch4");
    e->SetTitle("Sh0220_G_-37C");
    e->GetXaxis()->SetRange(10,2000);
//    e->GetYaxis()->SetRange(0,1000);
    e->Draw();
    fun->SetParameters(10,900,150,10,1150,25);
    e->Fit("fun","","same",800,1250);

    myc->cd(6);
    TH1D *f = (TH1D*)gDirectory->Get("ehistch5");
    f->SetTitle("Sh0220_H_-37C");
    f->GetXaxis()->SetRange(10,2000);
//    f->GetYaxis()->SetRange(0,1000);
    f->Draw();
    fun->SetParameters(10,900,150,10,1150,25);
    f->Fit("fun","","same",800,1250);

    myc->cd(7);
    TH1D *g = (TH1D*)gDirectory->Get("ehistch6");
    g->SetTitle("Sh0229_G_-37C");
    g->GetXaxis()->SetRange(10,2000);
//    g->GetYaxis()->SetRange(0,1000);
    g->Draw();
    fun->SetParameters(10,900,150,10,1150,25);
    g->Fit("fun","","same",800,1250);

    myc->cd(8);
    //TH1D *h = (TH1D*)gDirectory->Get("ehistch7");
    //
    //h->SetTitle("Sh0229_H_-37C");
    //h->GetXaxis()->SetRange(10,2000);
//  //  h->GetYaxis()->SetRange(0,1000);
    //h->Draw();
    //fun->SetParameters(10,900,150,10,1150,25);
    //h->Fit("fun","","same",800,1250);
    myc->SaveAs("plot.png");
}
