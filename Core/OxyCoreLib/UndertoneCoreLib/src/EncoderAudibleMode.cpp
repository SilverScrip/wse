#include "EncoderAudibleMode.h"
#include "ReedSolomon.h"

#include "Globals.h"
#include <iostream>
#include <cmath>

#include <time.h>

#ifdef LINUX
#include <string.h> //memset()
#include <stdlib.h> //srand()
#endif

//#include <android/log.h>

using namespace OXY;

EncoderAudible::EncoderAudible(float samplingRate, int buffsize, int windowSize) : Encoder(samplingRate, buffsize, windowSize, Globals::numTokensAudible, Globals::numTokensAudible)
{
//__android_log_print(ANDROID_LOG_INFO, "OxyCoreLibInfo", "EncoderAudible init" );
}

EncoderAudible::~EncoderAudible(void)
{
}

int EncoderAudible::EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize)
{
  memset(mAudioBufferEncodedString,0,mNumMaxSamplesEncodedString*sizeof(float));

//  __android_log_print(ANDROID_LOG_INFO, "OxyCoreLibInfo", "EncodeDataToAudioBuffer %s type %d size %d", stringToEncode, type, size );
  
  mNumSamplesEncodedString = 0;

  std::vector<int> digits;
  
  //Add front-door symbols (start tokens)
  digits.push_back(Globals::getIdxFromChar(Globals::frontDoorTokens[0])); //front-door symbols
  digits.push_back(Globals::getIdxFromChar(Globals::frontDoorTokens[1])); //front-door symbols
    
  
  //Add user symbols
  for (int i=0;i<size;i++)
  {
    digits.push_back(Globals::getIdxFromChar(stringToEncode[i]));
  }

//  __android_log_print(ANDROID_LOG_INFO, "OxyCoreLibInfo", "Added digits");
  
  //Add check digit
  int checkDigit = 0;
  for (int i=Globals::numFrontDoorTokens;i<(Globals::numFrontDoorTokens+Globals::numWordTokens);i++)
    checkDigit += digits[i];
  checkDigit = checkDigit % mNumTokens;

  digits.push_back(checkDigit);

  //Add Reed-Solomon characters
  mReedSolomon->SetMessage(digits);
  mReedSolomon->Encode();
  // get RS code to transmit
  mReedSolomon->GetCode(digits);

  for (int i=0;i<digits.size();i++)
  {
//__android_log_print(ANDROID_LOG_INFO, "OxyCoreLibInfo", "Digit %d", i);
    float tailLength = 0.5f;
    float gapLength = 0.1f; //gap between tokens
    int samplesPerDigit = (int)(mSampleRate * Globals::durToken);
    int samplesForFadeBegin = (int)(mSampleRate * Globals::durToken*Globals::durFade);
    int samplesForFadeEnd = (int)(mSampleRate * Globals::durToken*tailLength);
    int samplesForGap = (int)(mSampleRate * Globals::durToken*gapLength);
    float currentFreq = Globals::getFreqFromIdxAudible(digits[i],mSampleRate,mWindowSize);
    float currentFreqLoudness = Globals::getLoudnessFromIdx(digits[i]);

    for (int t=0;t<samplesPerDigit;t++)
    {
      float factor = Globals::tokenAmplitude;

      //if (i==0) samplesForFadeEnd = samplesForFadeBegin; //for first token
      if (i==0) 
      {
        samplesForFadeEnd = (int)(mSampleRate * Globals::durToken*(tailLength*0.5f)); //for first token
        samplesForGap = 0;
      }

      if (t<samplesForFadeBegin)
        factor = factor * (float)t/(float)samplesForFadeBegin;
      else if ( (t>(samplesPerDigit-(samplesForFadeEnd+samplesForGap))) && (t<(samplesPerDigit-samplesForGap)) )
        factor = factor * (float)((samplesPerDigit-samplesForGap)-t)/(float)(samplesForFadeEnd);
      else if (t>=(samplesPerDigit-samplesForGap))
        factor = 0.f;
      
//      __android_log_print(ANDROID_LOG_INFO, "OxyCoreLibInfo", "Sample %d", t);
      
      mAudioBufferEncodedString[t+i*samplesPerDigit] = currentFreqLoudness * factor * sinf((2.f*Globals::pi*currentFreq)*((float)t/(float)mSampleRate));

    }
    mNumSamplesEncodedString+=samplesPerDigit;
  }

  mReadIndexEncodedAudioBuffer = 0; //New audio has been created, then reset read index
  
  if (type == 1) //Add robotic sounds to mAudioBufferEncodedString of size mNumSamplesEncodedString
  {    
    //Initialize random seed:
	  srand(time(NULL));
    for (int i=0;i<digits.size();i++)
    {
      int samplesPerDigit = (int)(mSampleRate * Globals::durToken);
      int samplesForFade = (int)(mSampleRate * Globals::durToken*Globals::durFade);
      
		  //Generate a random number:
		  int randNumber = rand() % mNumTokens;
      float currentFreq = Globals::getMusicalNoteFromIdx(randNumber);
  
      float f_start = currentFreq;
      int randOffset = (rand() % 500) - 250;
      float f_end = currentFreq + randOffset;
      
      for (int t=0;t<samplesPerDigit;t++)
      {
        float factor = Globals::tokenAmplitude;

        if (t<samplesForFade)
          factor = factor * (float)t/(float)samplesForFade;
        else if (t>(samplesPerDigit-samplesForFade))
          factor = factor * (float)(samplesPerDigit-t)/(float)samplesForFade;

        float delta = t / (float)samplesPerDigit;
        float frequency = f_start + (delta * (f_end - f_start));
        float waveLength = 1.f / frequency;
      
        float timePos = (float)t / mSampleRate; 
        float pos = timePos / waveLength;

        //mAudioBufferEncodedString[t+i*samplesPerDigit] = (0.75f * mAudioBufferEncodedString[t+i*samplesPerDigit]) +
        //                                                 (0.25f * factor * sin(pos * 2.f * Globals::pi));

        float vol = pow(10.f, Globals::synthVolume / 20.f);
        mAudioBufferEncodedString[t + i*samplesPerDigit] = (1.f * mAudioBufferEncodedString[t + i*samplesPerDigit]) +
          (vol * factor * sin(pos * 2.f * Globals::pi));
      }

    }
  }
  else if (type == 2) //Add melody to mAudioBufferEncodedString of size mNumSamplesEncodedString
  {   
    std::vector<int> melodyDigits;
  
    //Add user symbols
    for (int i=0;i<melodySize;i++)
    {
      melodyDigits.push_back(Globals::getIdxFromChar(melodyString[i]));
    }

    int numSamplesMelodyString = 0;
    for (int i=0;i<melodyDigits.size();i++)
    {
      int samplesPerDigit = (int)(mSampleRate * Globals::durToken);
      int samplesForFade = (int)(mSampleRate * Globals::durToken*Globals::durFade);
      
		  float currentFreq = Globals::getMusicalNoteFromIdx(melodyDigits[i]);

      for (int t=0;t<samplesPerDigit;t++)
      {
        float factor = Globals::tokenAmplitude;

        if (t<samplesForFade)
          factor = factor * (float)t/(float)samplesForFade;
        else if (t>(samplesPerDigit-samplesForFade))
          factor = factor * (float)(samplesPerDigit-t)/(float)samplesForFade;
        
        mAudioBufferEncodedString[t+i*samplesPerDigit] = (0.75f * mAudioBufferEncodedString[t+i*samplesPerDigit]) +
                                                         (0.25f * factor * sinf((2.f*Globals::pi*currentFreq)*((float)t/(float)mSampleRate)));
      }
      numSamplesMelodyString+=samplesPerDigit;
    }

    if (numSamplesMelodyString > mNumSamplesEncodedString)
      mNumSamplesEncodedString = numSamplesMelodyString;
  }

  return mNumSamplesEncodedString;
}


int EncoderAudible::GetEncodedAudioBuffer(float *audioBuffer)
{
  return Encoder::GetEncodedAudioBuffer(audioBuffer);
}

int EncoderAudible::ResetEncodedAudioBuffer()
{
  return Encoder::ResetEncodedAudioBuffer();
}

