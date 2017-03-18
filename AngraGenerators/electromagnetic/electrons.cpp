//--------------------------------------------------------------
//--------------------------------------------------------------
//
//  Cosmic electrons generator
//
//  Author: P.Chimenti
//
//  14-9-2010 v0.1, 28-05-2013 v0.9 
//
//--------------------------------------------------------------
//--------------------------------------------------------------

#include "TMatrixD.h"
#include "TVectorD.h"
#include "TMath.h"


#ifndef __CINT__
#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/Rotation3D.h"
#include "Math/EulerAngles.h"
#include "Math/AxisAngle.h"
#include "Math/Quaternion.h"
#include "Math/RotationX.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"
#include "Math/RotationZYX.h"
#include "Math/LorentzRotation.h"
#include "Math/Boost.h"
#include "Math/BoostX.h"
#include "Math/BoostY.h"
#include "Math/BoostZ.h"
#include "Math/Transform3D.h"
#include "Math/Plane3D.h"
#include "Math/VectorUtil.h"

using namespace ROOT::Math;

#endif

void electrons(int INI_RAND=9999,int MAXEVENTS=10000, TString file="event.hepev")
{

  // ROOT initialization		
#ifdef __CINT__
  gSystem->Load("libMathCore");
  using namespace ROOT::Math;
#endif
  delete gRandom;
  gRandom = new TRandom3(INI_RAND);

  // constant definitions
  const Double_t pi = TMath::Pi();

  //geometric definitions
  const Double_t SURF_HALF_LEN=2.5;//units: m 
  const Double_t SURF_DIST=4.0;//units: m

  //physical definitions
  const Double_t  electronMass=0.00051099891;//units: GeV/c^2 

  //
  // The model is from P.K.F.Grieder, "Cosmic rays at earth", pg 344
  // angular dependence: n=2.0
  // 
  
  // Spectrum from simulation
  // from figure pg. 344 of Greider

  // points taken with gimp: need to transform to physical values
  // set initial values of spectrum data
  // values to tranform points to momentum or flux
  // flux in cm^-2 s^-1 sr^-1 GeV^-1
  // energy (momentum) in GeV (/c)

  const double delPoints=48.6;
  const double iniPx=656;
  const double finPx=2600;
  const double iniPy=3101;
  const double finPy=191;
  const double iniMx=-3.;
  const double finMx=1.;
  const double iniFy=-8.;
  const double finFy=1.;

  // data vectors
  const int nbins=40;
  double xPoints[nbins+1]={
    680.3-delPoints/2., 728.9-delPoints/2., 777.5-delPoints/2.,
    826.1-delPoints/2., 874.4-delPoints/2., 923.3-delPoints/2.,
    971.9-delPoints/2., 1020.5-delPoints/2., 1069.1-delPoints/2.,
    1117.7-delPoints/2., 1166.3-delPoints/2., 1214.9-delPoints/2.,
    1263.5-delPoints/2., 1312.1-delPoints/2., 1360.7-delPoints/2.,
    1409.3-delPoints/2., 1457.9-delPoints/2., 1506.5-delPoints/2.,
    1555.1-delPoints/2., 1603.7-delPoints/2., 1652.3-delPoints/2.,
    1700.9-delPoints/2., 1749.5-delPoints/2., 1798.1-delPoints/2.,
    1846.7-delPoints/2., 1895.3-delPoints/2., 1943.9-delPoints/2.,
    1992.5-delPoints/2., 2041.1-delPoints/2., 2089.7-delPoints/2.,
    2138.3-delPoints/2., 2186.9-delPoints/2., 2235.5-delPoints/2.,
    2284.1-delPoints/2., 2331.7-delPoints/2., 2381.3-delPoints/2.,
    2429.9-delPoints/2., 2478.5-delPoints/2., 2527.1-delPoints/2.,
    2575.7-delPoints/2., 2575.7+delPoints/2.
  };

  double yPoints[nbins]={
    478,524,571,
    610,650,684,
    716,752,792,
    827,866,906,
    947,987,1037,
    1084,1134,1184,
    1239,1293,1346,
    1404,1466,1521,
    1581,1645,1714,
    1784,1850,1925,
    2005,2087,2166,
    2244,2322,2408,
    2486,2584,2675,
    2772
    };

  // now translate point coordinates to momentum and flux 
  // use ultrarelativistic eletrons approximation (energy = momentum)
  double xMomentumLog[nbins+1];
  double xMomentum[nbins+1];
  for(int i=0; i<nbins+1; i++){
    xMomentumLog[i]=((xPoints[i]-iniPx)/(finPx-iniPx))*(finMx-iniMx)+iniMx;
    xMomentum[i]=pow(10,xMomentumLog[i]);
  }

  double yFluxLog[nbins+1];
  double yFlux[nbins+1];
  for(int i=0; i<nbins; i++){
    yFluxLog[i]=((yPoints[i]-iniPy)/(finPy-iniPy))*(finFy-iniFy)+iniFy;
    yFlux[i]=pow(10,yFluxLog[i]);
  }

  // now set histo
  histoFlux =new TH1F ("ElectronFlux","electronFlux",nbins,xMomentum);
  for(int i=0; i<nbins; i++){
    histoFlux->SetBinContent(i+1,yFlux[i]);
  }

  // just check histo
//   histoFlux->Draw();
  
  // now azimuthal angle
  double thetaIndex=2.;
  TF1 *thetaDistr = new TF1("Theta Distribution","TMath::Sin(x)*(TMath::Cos(x))**[0]",0,pi/2);
  thetaDistr->SetParameter(0,thetaIndex);

  // open out file
  ofstream outfile;
  outfile.open(file);

  // start generating
  int ievent=0;
//   ROOT::Math::XYZVector v1();// why that? Misteries of cint!
    XYZVector iv(); // y is vertical, x and z horizonthal 
    RotationX rx();
    RotationY ry();
    XYZVector fv();
    XYZVector iposition();
    XYZVector fposition();
    Double_t  momentum, ltheta, phi, initX,initY,initZ, electronEnergy;
  
  for(;ievent<MAXEVENTS; ievent++){    
    outfile << ievent << " " << 1 << endl;
    
    // generate event variables
    momentum = histoFlux->GetRandom();
    ltheta = thetaDistr->GetRandom();
    phi = gRandom->Uniform(0.,2*pi);
 

    // generate verticla particle and then rotates
    // momentum first
    iv = XYZVector(0.0, -1.0*momentum, 0.0); // y is vertical, x and z horizonthal 
    rx = RotationX(ltheta);
    ry = RotationY(phi);
    fv = XYZVector(ry*rx*iv);
    
    // position
    initX = gRandom->Uniform(-1.*SURF_HALF_LEN,SURF_HALF_LEN);
    initZ = gRandom->Uniform(-1.*SURF_HALF_LEN,SURF_HALF_LEN);
    initY = SURF_DIST;
    iposition = XYZVector(initX,initY,initZ);
    fposition = XYZVector(ry*rx*iposition);
    
    // energy
    electronEnergy = sqrt(momentum*momentum+electronMass*electronMass);


    // now printout
    outfile << 1 << " " << 11  << " " //
	    << 0  << " " << 0  << " "//
	    << 0  << " " << 0  << " "//
	    << fv.X()  << " " << fv.Y()  << " " //
	    << fv.Z()  << " " << electronEnergy  << " " //
	    << electronMass  << " " << fposition.X()*1000  << " " // position in mm
	    << fposition.Y()*1000  << " " << fposition.Z()*1000 << " " // position in mm
	    << 0 << endl;
  }
  delete histoFlux;
  delete thetaDistr;
  
}
