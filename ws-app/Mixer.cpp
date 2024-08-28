/*--------------------------------------------------------------------------------
 Mixer.cpp
 
 Copyright (C) 2020 OxyCom Ltd, All rights reserved
 
 CONFIDENTIAL: This document contains confidential information.
 Do not disclose any information contained in this document to any
 third-party without the prior written consent of OxyCom Ltd.
 --------------------------------------------------------------------------------*/

#include "Mixer.h"

#include <strstream>
#include <iostream>
#include <algorithm>
#include <cmath> // for M_PI define

#ifndef MIN
#define MIN(a,b) ((a <= b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a >= b) ? (a) : (b))
#endif

using namespace std;

int Mixer::mix(const float** bufferPgm, const int nsamples, int nchannels, const float samplerate, const float* bufferUndertone, float** bufferMix)
{
  progress_mix = 0;
  cout << "Mixed = " << progress_mix << endl;

  vector<float> timestamps;
  vector<float> undertoneLevel;
  float percentile10;
  
  computeUndertoneLevel(bufferPgm[0], nsamples, samplerate, timestamps, undertoneLevel, percentile10);
  int ntimestamps = undertoneLevel.size()-2;

  // get linear gain values
  float defUndertoneLevel = pow(10.f, mDefaultUndertoneLevel/20.f);
  float defPgmLevel = pow(10.f, mDefaultProgramLevel/20.f);
  
  cout << "Mixed= " << 95 << endl;

  float maxpeak = 0.;
  if (mMode == kDynamicLevelMode)
  {
    int eidx = 0; // energy index
    float level = 1.f;
    float interp = 0.f;
    
    for (int i=0; i < nsamples; i++)
    {
      // get current index in energy timestamps
      if ((i/samplerate) > timestamps[eidx+1])
        eidx++;
      eidx = min(eidx, ntimestamps);
      
      // interpolate level value per sample
      interp = ((i/samplerate) - timestamps[eidx])/ (timestamps[eidx+1] - timestamps[eidx]);
      level = (1.f - interp) * undertoneLevel[eidx] + interp * undertoneLevel[eidx+1];

      // mix buffers
      for (int j=0; j < nchannels; j++){
        bufferMix[j][i] = MAX(-1.0, MIN(1.0, level * bufferUndertone[i] + defPgmLevel * bufferPgm[j][i]));
        // update max peak
        maxpeak = (abs(bufferMix[j][i]) > maxpeak) ? abs(bufferMix[j][i]) : maxpeak;
      }
    }
  }
  else
    if (mMode == kGlobalLevelMode){
      
      float minlevelLin = pow(10.f, mMinUndertoneLevel/20.f);
      float globalLevelDB = percentile10 + mDefaultUndertoneLevel;
      float globalUndertoneLevel = max(minlevelLin, min(.95f, powf(10.f, globalLevelDB /20.f))); // TODO
      
      // apply default gain for all channels
      for (int i=0; i < nsamples; i++)
        for (int j=0; j < nchannels; j++){
          bufferMix[j][i] = MAX(-1.0, MIN(1.0, globalUndertoneLevel * bufferUndertone[i] + defPgmLevel * bufferPgm[j][i]));

          // update max peak
          maxpeak = (abs(bufferMix[j][i]) > maxpeak) ? abs(bufferMix[j][i]) : maxpeak;
        }
    }
    else
      if (mMode == kDefaultMode)
      {
        // apply default gain for all channels
        for (int i=0; i < nsamples; i++)
          for (int j=0; j < nchannels; j++){

            //bufferMix[j][i] = defUndertoneLevel * bufferUndertone[i] + defPgmLevel * bufferPgm[j][i];
            bufferMix[j][i] = MAX(-1.0,MIN(1.0,defUndertoneLevel * bufferUndertone[i] + defPgmLevel * bufferPgm[j][i]));
            // update max peak
            maxpeak = (abs(bufferMix[j][i]) > maxpeak) ? abs(bufferMix[j][i]) : maxpeak;
          }
      }
      else
      {
        return 1;
      }
  
  // might be disabled for optimization
  if (mUseNormalize)
    for (int i=0; i < nsamples; i++)
      for (int j=0; j < nchannels; j++)
        bufferMix[j][i] /= maxpeak;
  
  cout << "Fully Mixed = " << 100 << endl;

  return 0;
}


// returns a vector of level (linear gain) for the oxys signal
int Mixer::computeUndertoneLevel(const float* buffer, const int nsamples,  const float samplerate, vector<float> &timestamps, vector<float> &undertoneLevel, float &percentile10)
{
  // estimate energy and dynamics curves
  vector<float> energy;
  vector<float> energyDB;
  vector<float> stab;
  
  // Program statistics
  
  // set frameTime  to 11.6ms
  float frametime = 512.f/samplerate; // 11.6ms default
  
  computeEnergy(buffer, nsamples, samplerate, frametime, timestamps, energy);
  
  cout << "Still Mixing = " << 75 << endl;
  
  computeDynamicsStability(energy, frametime, energyDB, stab, percentile10);

  cout << "Mixed = " << 90 << endl;
  
  // Program Typical levels:
  // - comercial Rock music: [-5..-35dB]
  // - acoustic pop music: [-12..-40dB]
  // - documentary speech: [-15..-45dB]
  
  // Undertone typical levels:
  // -3dB: usual level for inaudible
  // -12dB: usual level for hidden
  // -3dB: usual level for audible
  
  
  // compute mixing levels for signal
  float maxLevelDB = mDefaultUndertoneLevel;
  float minLevelDB = mMinUndertoneLevel;
  float level = 1.f;
  float levelDB = 0.f;
  
  for (int i = 0; i < (int)energyDB.size(); i++)
  {
    // scale range [-5..-40] to [maxLevelDB..maxLevelDB-20] dB
    //levelDB = (max(maxLevelDB - minLevelDB, min(maxLevelDB, maxLevelDB + (energyDB[i] + 5.f) / (-5.f + 40.f))) / minLevelDB);
    levelDB = max(minLevelDB, min(energyDB[i]+mDefaultUndertoneLevel, maxLevelDB));
    level = powf(10.f, levelDB/20.f);

    //todo: this line
    //level = powf(10.f, (max(maxLevelDB-minLevelDB, min(maxLevelDB, maxLevelDB + (energyDB[i] + 5.f)/ (-5.f + 40.f)))/minLevelDB));
    undertoneLevel.push_back(level); // linear
  }

  return 0;
}


int Mixer::computeEnergy(const float *buffer, const int nsamples,  const float samplerate, float frameTime, vector<float> &timestamps, vector<float> &energy)
{ 
  int h = int(frameTime*samplerate + 0.5); // hop size
  int ws = 4*h; //int(2048*samplerate/44100.f); // win size
  ws = ws - (ws%2); // make it even
  vector<float> w;
  w.reserve(nsamples);
  energy.reserve(nsamples);
  timestamps.reserve(nsamples);
  float area = hanning(ws+1, w);
  int hws = ws/2;
  
  int nFrames = int(nsamples/h)-2;  
  
  int i,k;
  for (i=0; i<nFrames; i++) {
    
    float current_progress_mix = ((float)i / (float)nFrames)*75.f; //from 0% to 75%
    if (current_progress_mix > progress_mix + 5)
    {
      progress_mix = current_progress_mix;
      cout << "Mixing = " << progress_mix << endl;
    }

    // compute energy for one window frame
    int b = h*i-hws;
    int e = h*i+hws;
    float en = 0.f;
    if (b<1)
      for(k=0;k<=e;k++)
        en += (buffer[k]*buffer[k])*w[k-b];
    else
      if (e>=nsamples)
        for(k=b;k<nFrames;k++){
          en += (buffer[k]*buffer[k])*w[k-b];
        }
      else
        for(k=b;k<=e;k++)
          en += (buffer[k]*buffer[k])*w[k-b];
    
    // store values
    timestamps.push_back(i * frameTime);
    en /= area;
    energy.push_back(en);
  }

  return 0;
}


// -------------------------------------------------------------------------------------------------
// compute a dynamics stability measure from a input energy (linear) vector and outputs the energy in DB
int Mixer::computeDynamicsStability(const vector<float> energy, float frameTime, vector<float> &energyDB, vector<float> &st, float &percentile10)
{
  int nFr = energy.size();
  energyDB.clear();       // empty vector
  st.clear();             // empty vector
  vector<float> ew;  // energy weight
  
  float edB = 0;
  for (int i=0; i < nFr; ++i)
  {
    edB = 10.f*log10f(energy[i] + 1e-200);
    energyDB.push_back(edB);
    st.push_back( edB );
    ew.push_back( edB );
  }
  
  cout << "Mixing = " << 80 << endl;

  // compute range of energy weight
  sort(ew.begin(),ew.end());
  int per10Idx =  floor(float(ew.size()) * .1f);
  double p10 = *(ew.end() - per10Idx); // compute percetile 10
  percentile10 = p10;

  //todo: REMOVE_BELOW
  printf("Percentile: %f  (%d frames)\n", p10, nFr);
  
  float rangeDB = 18; // in DB
  float stabThres = 3.25;
  float stabK = 1.75; // sigmoid factor
  
  float xval = 0.;
  st[0] = 0.;
  for(int i=1; i<nFr-2; i++)
  {
    st[i] = 0.5 * (energyDB[i+2] + energyDB[i+1] - energyDB[i] - energyDB[i-1]); // non energy-weighted
#ifdef __LINUX__
    st[i] = abs(st[i]);
#else
    st[i] = abs(st[i]);
#endif


    // sigmoid to scale it in a range [0..1] // from non-stable (0) to stable (1)
    xval = (st[i] - stabThres); // remove threshold

//Non-Linear Interpolation. It is the process of calculating the positions of points at regular intervals between two points, one at a time.
//For example, a line drawing algorithm takes 2 points as parameters, then it must calculate the exact position of each pixel on the line segment.
    st[i] = 1.f - ( 1.f / (1.f + exp(- stabK * xval)));  // non-linear interpolation (logistic function to preserve best the boundaries)
    
    // apply energy-weight to avoid having stablility low-energy (noise floor). We apply a weight relative to the 2 times the range dB from the percentile-10 of the energy values.

#ifdef __LINUX__
    st[i] *= max(0.0, min(1.0, (energyDB[i] - (p10-2.f*rangeDB)) / (rangeDB)));
#else
    st[i] *= fmax(0.0, min(1.0, (energyDB[i] - (p10-2.f*rangeDB)) / (rangeDB))); // apply energy-weighted relative to the range -24dB from the percentile-10 of the energy values.
#endif
  }
  
  cout << "Mixing = " << 85 << endl;

  st[st.size()-2] = 0.f;
  st[st.size()-1] = 0.f;
  for(int i=0; i<nFr-1; i++) // square to increase binary behaviour at boundaries
    st[i] *= st[i];
  
  // smooth
  int smoothframes = int(mSmoothTime / frameTime);
  smooth(st, smoothframes, false);
  smooth(energyDB, smoothframes, false);

  return 0;
}

void Mixer::smooth(vector<float> &v,int window, bool useNonZero)
{
  //
  int i,k;
  int hw = int(window/2);
  vector<float> v2 = v;
  for(i=1; i<v.size()-1; i++)
  {
    int b = max(0,i-hw);
    int e = min((int)v.size()-1,i+hw);
    float s=0.f;
    int den = 0;
    for(k=b;k<=e;k++){
      s+= v2[k];
      if (v2[k] > 0){
        den++;
      }
    }
    
    if (useNonZero){
      // consider only non-zero values
      if (v[i] > 0){
        v[i] = s/float(den);
      }
      else {
        v[i] = 0.f;
      }
    }
    else {
      if ((e-b)> 0)
        v[i] = s/float(e-b+1.f);
      else
        v[i] = s;
    }
    
  }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

float Mixer::hanning(const int n, vector<float> &w)
{
  float area = 0.f;
  float v = 0.f;
  for (int i=0; i<n;i++){
    v = 0.5f * (1 - cos(2*M_PI*i/(n-1)));
    w.push_back(v);
    area += v;
  }

  return area;
}
