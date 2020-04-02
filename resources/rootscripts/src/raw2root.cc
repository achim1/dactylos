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


using namespace std;

int main(int argc, char** argv)
{
	if(argc!=3)
	{
		cout<<"Syntax: "<<argv[0]<<" [input file] [output file]"<<endl;
		exit(1);
	}
	TString infilename = TString(argv[1]);
	TString outfilename = TString(argv[2]);	

	char c0;
    int Nc=0;
	ifstream myfile0(argv[1]);
    if (!myfile0)
    {
		cerr <<" cannot open the data file " <<endl;
        return -1;
	}
    while (myfile0.get(c0))
    {
		if (c0=='\n')
        Nc++;
	}
    cout<<"processing file: "<<infilename<<"***"<<endl;

	double data[100000][2];
	ifstream myfile(argv[1]);
    for (int i=0;i<Nc;i++)
	{
		for (int j=0;j<2;j++)
		{
			myfile>>data[i][j];
//			cout<<data[i][j]<<endl;
		}
	}

	double chans;
	double weight;
	TFile *fout=new TFile(argv[2],"recreate");
    TTree *out_tree=new TTree("out_tree","");
    out_tree->Branch("chans", &chans, "chans/D");
    out_tree->Branch("weight", &weight, "weight/D");

	for (int k=0; k<Nc; k++)
	{
		chans=data[k][0];
		weight=data[k][1];
//		cout<<chans<<" "<<weight<<endl;

		out_tree->Fill();
	}

	out_tree->Write();
	out_tree->Delete();
	fout->Close();
	
	return 0;
}
