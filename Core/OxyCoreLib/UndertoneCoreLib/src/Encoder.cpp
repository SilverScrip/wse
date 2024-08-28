#include "Encoder.h"
#include "ReedSolomon.h"

#include "Globals.h"
#include <iostream>
#include <cmath>

#include <time.h>

#include <string.h>

using namespace OXY;

Encoder::Encoder(float samplingRate, int buffsize, int windowSize, int numTokens, int numTones)
{
  mnAudioSignatureSamples = 0;
  mAudioSignature = NULL;
  
  mNumTokens = numTokens;
  mNumTones = numTones;

  mReadIndexEncodedAudioBuffer = 0;
  mNumSamplesEncodedString = 0;
  mNumMaxSamplesEncodedString = (int)(10.f*samplingRate);
  mAudioBufferEncodedString = new float[mNumMaxSamplesEncodedString];

  mSampleRate = samplingRate;
  mBufferSize = buffsize;
  mWindowSize = windowSize;

  Globals::init(windowSize, mSampleRate);

  mReedSolomon = new ReedSolomon();
}

Encoder::~Encoder(void)
{
  if (mAudioSignature)
  {
    delete [] mAudioSignature;
    mAudioSignature = NULL;
    mnAudioSignatureSamples = 0;
  }
  
  delete [] mAudioBufferEncodedString;
  delete mReedSolomon;
}


int Encoder::SetAudioSignature(int samplesSize, const float *samplesBuffer)
{
  if ((samplesBuffer == NULL) || (samplesSize == 0))
  {
    mnAudioSignatureSamples = 0;
    delete[] mAudioSignature;
    mAudioSignature = NULL;
    return 0;
  }

  if (mnAudioSignatureSamples > 0)
  {
    delete[] mAudioSignature;
    mAudioSignature = NULL;
  }
  
  
  if (samplesSize > 0)
  {
    mnAudioSignatureSamples = std::min(samplesSize, (int)(Globals::numTotalTokens * (Globals::durToken*mSampleRate)));
    mAudioSignature = new float[mnAudioSignatureSamples];
    memcpy(mAudioSignature, samplesBuffer, mnAudioSignatureSamples * sizeof(float));
  }
  
  return 0;
}

int Encoder::EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize)
{
  return 0;
}

int Encoder::GetEncodedAudioBuffer(float *audioBuffer)
{
  int samplesRead = 0;
  for (int i=0;i<mBufferSize;i++)
  {
    if (mReadIndexEncodedAudioBuffer >= mNumSamplesEncodedString)
      break;
    audioBuffer[i] = mAudioBufferEncodedString[mReadIndexEncodedAudioBuffer];
    samplesRead++;
    mReadIndexEncodedAudioBuffer++;
  }

  return samplesRead;
}

int Encoder::ResetEncodedAudioBuffer()
{
  mReadIndexEncodedAudioBuffer = 0;

  return 0;
}

