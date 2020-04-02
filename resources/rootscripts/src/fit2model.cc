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
#include "TGraphErrors.h"
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
#include <TLatex.h>

using namespace std;

int main(int argc, char** argv)
{
	if (argc!=5)
	{
		cout<<"Syntax: "<<argv[0]<<" "<<"[input file] [no of tpeaks] [temperature] [output file]"<<endl;
		exit(1);
	}

    double T = atof(argv[3])+273;    // temperature for testing
    double q = 1.6e-19; // electron charge
    double k = 1.38e-23; // Boltzmann constant
    double eps = 3.6;    // ionization energy of silicon, eV
    double Rp = 100e6;   // parallel resistance of preamp, 100 MOhm
    double gm = 18e-3;   // transconductance in FET, 18 ms
    double Bita = 1;
    double Fi = 0.367;   // noise form factor
    double Fv = 1.15;    // noise form factor
    double Fvf = 0.226;  // noise form factor
    double pi = 3.1415926;

	int nevts=0;
	char c;
	ifstream myfile1(argv[1]);
	if (!myfile1)
	{
		cerr <<" cannot open the data file " <<endl;
		return -1;
	}
        while (myfile1.get(c))
	{
		if (c=='\n')
		nevts++;
	}
    int npeakt = atoi(argv[2]);
    int nstrips = nevts / npeakt;
    cout<<"********total strips are: "<<nstrips<<"********"<<endl;

	ifstream myfile(argv[1]);
    vector<string> vec;
    string temp;
    while (getline(myfile, temp))
    {
        vec.push_back(temp);

    }
    vector<string> col0;
    vector<string> col1;
    vector<float> col2;
    vector<float> col3;
    vector<float> col4;
    vector<float> col5;
    vector<float> col6;
    for (auto it = vec.begin(); it != vec.end(); it++)
    {
        cout << *it << endl;
        istringstream is(*it);
        string s;
        int pam=0;
        while (is >> s)
        {
            if (pam ==0)
            {
                col0.push_back(s);
            }
            if (pam ==1)
            {
                col1.push_back(s);
            }
            if (pam ==2)
            {
                float a2 = atof(s.c_str());
                col2.push_back(a2);
            }
            if (pam ==3)
            {
                float a3 = atof(s.c_str());
                col3.push_back(a3);
            }
            if (pam ==4)
            {
                float a4 = atof(s.c_str());
                col4.push_back(a4);
            }
            if (pam ==5)
            {
                float a5 = atof(s.c_str());
                col5.push_back(a5);
            }
            if (pam ==6)
            {
                float a6 = atof(s.c_str());
                col6.push_back(a6);
            }
            pam ++;
        }
    }
    TFile *fout = new TFile(argv[4], "recreate");
    TTree *tree = new TTree("tree", "");
    string sn;
    string strip;
    float tpeak;
    float energy;
    float sigma;
    float res;
    float res_err;
    tree->Branch("sn", &sn);
    tree->Branch("strip", &strip);
    tree->Branch("tpeak", &tpeak, "tpeak/F");
    tree->Branch("energy", &energy, "energy/F");
    tree->Branch("sigma", &sigma, "sigma/F");
    tree->Branch("res", &res, "res/F");
    tree->Branch("res_err", &res_err, "res_err/F");
    for (long int ii = 0; ii<nevts; ii++)
    {
        if (ii%1000==0) cout<<"events: "<<ii<<endl;
        sn = col0[ii];
        strip = col1[ii];
        tpeak = col2[ii];
        energy = col3[ii];
        sigma = col4[ii];
        res = col5[ii];
        res_err = col6[ii];

        tree->Fill();
    }
    tree->Write();

//    TCanvas *myc = new TCanvas("myc","",1200,900);
//    myc->Divide(4,2);
    double gx[10];
    double gxx[10];
    double gy[10];
    double gyy[10];
    for (int jj = 0; jj<nstrips; jj++)
    {
        string sgraph = col0[0+jj*10]+"_"+col1[0+jj*10]+"_"+argv[3]+"C";
        string splot = col0[0+jj*10]+"_"+col1[0+jj*10]+"_"+argv[3]+"C.png";
        char* gname = const_cast<char *>(sgraph.c_str());
        char* pname = const_cast<char *>(splot.c_str());
        cout<<"graph name: "<<gname<<"pname: "<<pname<<endl;

        for (int kk=0; kk<10; kk++)
        {
            gx[kk] = col2[jj*10+kk];
            gxx[kk] = 0.;
            gy[kk] = col5[jj*10+kk];
            gyy[kk] = col6[jj*10+kk];
        }

        TCanvas *myc = new TCanvas("myc","",1200,900);
        myc->cd();
        myc->SetLogx();
        myc->SetLogy();
        myc->SetGridx();
        myc->SetGridy();
        TGraphErrors *g = new TGraphErrors(10,gx,gy,gxx,gyy);
        g->SetTitle(gname);
        g->GetXaxis()->SetRangeUser(0.5,35);
        g->GetYaxis()->SetRangeUser(1.99,30);
//        g->GetXaxis()->SetMoreLogLabels();
        g->GetYaxis()->SetMoreLogLabels();
        g->Draw("AP*");
        TF1 *f= new TF1("f","sqrt([0]*x*1e-6+[1]/(x*1e-6)+[2])",0,40); // us->s
        double factor = (2.355*eps*1e-3/q)*(2.355*eps*1e-3/q);
        f->SetParameters(5e5, 1e-5,1);
        g->Fit("f","","same",0.4,31);
        g->Fit("f","","same",0.4,31);
        g->Fit("f","","same",0.4,31);
        g->Fit("f","","same",0.4,31);
        g->Fit("f","","same",0.4,31);
        gStyle->SetOptFit(1);
//        gStyle->SetStatX(0.8);
//        gStyle->SetStatY(0.8);
//        gStyle->SetStatW(0.14);
//        gStyle->SetStatH(0.2);

        double p0=g->GetFunction("f")->GetParameter(0)/factor;
        double ep0=g->GetFunction("f")->GetParError(0)/factor;
        double p1=g->GetFunction("f")->GetParameter(1)/factor;
        double ep1=g->GetFunction("f")->GetParError(1)/factor;
        double p2=g->GetFunction("f")->GetParameter(2)/factor;
        double ep2=g->GetFunction("f")->GetParError(2)/factor;
        double Ileak, Ctot, Af, Rs, eIleak, eCtot, eAf, eRs;
        Ileak = (p0/Fi-4*k*T/Rp)/2/q;  // A
        eIleak = ep0/p0*Ileak;  // A
//        Rs = 10.;
        Ctot = 70e-12;  //pF
        Rs = p1/Fv/Ctot/Ctot/(4*k*T)-Bita/gm;
        if (Rs <0)
        {
            Rs=0.;
        }
        eRs = ep1/p1*Rs;  // Ohm
//        Ctot = sqrt(p1/Fv/(4*k*T)/(Rs+Bita/gm));
//       eCtot = sqrt(ep1/p1)*Ctot;
        Af = p2/Ctot/Ctot/Fvf;
        Af = p2/Ctot/Ctot/Fvf/2./pi;
        eAf = ep2/p2*Af;
        cout<<fixed<<setprecision(2)<<"Rs (fixed): "<<Rs<<endl;
        cout<<gname<<" "<<fixed<<setprecision(2)<<"Ileak (nA): "<<Ileak*1e9<<" "<<"Ctot (pF): "<<Ctot*1e12<<" "<<"Af: "<<Af*1e13<<endl;

        TLatex *latex = new TLatex();
        latex->SetTextAlign(12);
        latex->SetTextFont(63);
        latex->SetTextSizePixels(36);
        latex->DrawLatex(1,20.,Form("C_{tot}:   %4.2f [pF]", Ctot*1e12));
//        latex->DrawLatex(1,20.,Form("C_{tot}:   %4.2f #pm %4.2f #times [pF]", Ctot*1e12, eCtot*1e12));
        latex->DrawLatex(1,16,Form("I_{leak}:   %4.2f #pm %4.2f [nA]", Ileak*1e9, eIleak*1e9));
        latex->DrawLatex(1,13,Form("A_{f}:  %4.2f #pm %4.2f #times 10^{-13} [V^{2}]", Af*1e13, eAf*1e13));
        latex->DrawLatex(1,10.5,Form("R_{s}:  %4.2f #pm %4.2f [#Omega]", Rs, eRs));
        myc->Update();
        myc->Print(pname);

        g->SetName(gname);
        g->Write();
    }

    tree->Delete();
    fout->Close();

	return 0;
}
