#include "OxyCoreLib_api.h"

#include <iostream>
#include <ctime>
#include <cassert>
#include <cstddef>

#include <string.h>
#include <stdio.h>

#include <vector>
#include <algorithm> //for reverse

#include "EncoderAudibleMode.h"
#include "DecoderAudibleMode.h"
#include "EncoderNonAudibleMode.h"
#include "DecoderNonAudibleMode.h"
#include "EncoderAudibleMultiToneMode.h"
#include "DecoderAudibleMultiToneMode.h"
#include "EncoderNonAudibleMultiToneMode.h"
#include "DecoderNonAudibleMultiToneMode.h"
#include "EncoderCompressionMultiToneMode.h"
#include "DecoderCompressionMultiToneMode.h"
#include "DecoderAllMultiToneMode.h"
#include "EncoderCustomMultiToneMode.h"
#include "DecoderCustomMultiToneMode.h"

#include "Globals.h"

static const char version[100] = "OxyCoreLib version 1.0 [10062020]";


using namespace OXY;

class COxyCore
{
  public:
    COxyCore()
    {
      //will be created on configure()
      //mEncoder = new Encoder(44100,2048); //configure with default params sr, buffsize
      //mDecoder = new Decoder(44100,2048);
    }

    ~COxyCore()
    {
      delete mEncoder;
      delete mDecoder;
    }

    //public functions

    //public vars
    
    Encoder *mEncoder;
    Decoder *mDecoder;
    
    float mSampleRate; //needed?
    int mBufferSize; //needed?
    int mWindowSize;
    
  private:

};


#ifdef __cplusplus
extern "C"
#endif //__cplusplus
void *OXY_Create() //Create OxyCore Object, the returned object will be passed as parameter to all API functions
{
  COxyCore *oxying = new COxyCore();
  oxying->mEncoder = 0;
  oxying->mDecoder = 0;
  return (void*)oxying;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
void OXY_Destroy(void *oxyingObject) //Destroy oxyingObject Object
{
  COxyCore* oxying = static_cast<COxyCore*>(oxyingObject);
  delete oxying;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
const char* OXY_GetVersion()
{
  //static char version[50] = "OxyCoreLib version *.*.* [********]";

	return version;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int OXY_GetVersionInfo(char * versioninfo)
{
  sprintf(versioninfo, "%s", version);

  return strlen(versioninfo);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_Configure(int mode, float samplingRate, int32_t bufferSize, void *oxyingObject)
{
	COxyCore *oxying = (COxyCore*)oxyingObject;

	oxying->mSampleRate = samplingRate;
	oxying->mBufferSize = bufferSize;
  
  if (oxying->mSampleRate == 48000.0)
    oxying->mWindowSize = 2048;
  else if (oxying->mSampleRate == 44100.0)
    oxying->mWindowSize = 2048;
  else if (oxying->mSampleRate == 22050.0) //not valid!!
    oxying->mWindowSize = 1024;
  else if (oxying->mSampleRate == 11050.0) //not valid!!
    oxying->mWindowSize = 512;
  else //not tested
    oxying->mWindowSize = 256;

  if (oxying->mEncoder)
  {
    delete oxying->mEncoder;
    oxying->mEncoder = 0;
  }

  if (oxying->mDecoder)
  {
    delete oxying->mDecoder;
    oxying->mDecoder = 0;
  }

/* TODO: Change to this
 if (mode == OXY_MODE_AUDIBLE) //Audible Multi-Tone
  {
    oxying->mEncoder = new EncoderAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == OXY_MODE_NONAUDIBLE) //NonAudible Multi-Tone
  {
    oxying->mEncoder = new EncoderNonAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderNonAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == OXY_MODE_COMPRESSION) //Compression Multi-Tone
  {
    oxying->mEncoder = new EncoderCompressionMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderCompressionMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == OXY_MODE_ALL) //All modes decoded simultaneously
  {
    oxying->mEncoder = new EncoderNonAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderAllMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == OXY_MODE_CUSTOM) //Custom mode
  {
    oxying->mEncoder = new EncoderCustomMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderCustomMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else
  {
    //error
    return -1;
  } 
  */



  // Should we remove old stuff
  if (mode == /*OXY_MODE::*/OXY_MODE_AUDIBLEOLD) //Audible
  {
    oxying->mEncoder = new EncoderAudible(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderAudible(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == /*OXY_MODE::*/OXY_MODE_NONAUDIBLEOLD) //Non audible
  { 
    oxying->mEncoder = new EncoderNonAudible(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderNonAudible(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == /*OXY_MODE::*/OXY_MODE_AUDIBLE) //Audible Multi-Tone
  {
    oxying->mEncoder = new EncoderAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == /*OXY_MODE::*/OXY_MODE_NONAUDIBLE) //NonAudible Multi-Tone
  {
    oxying->mEncoder = new EncoderNonAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderNonAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == /*OXY_MODE::*/OXY_MODE_COMPRESSION) //Compression Multi-Tone
  {
    oxying->mEncoder = new EncoderCompressionMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderCompressionMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == /*OXY_MODE::*/OXY_MODE_ALL) //All modes decoded simultaneously
  {
    oxying->mEncoder = new EncoderNonAudibleMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderAllMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else if (mode == /*OXY_MODE::*/OXY_MODE_CUSTOM) //Custom mode
  {
    oxying->mEncoder = new EncoderCustomMultiTone(samplingRate, bufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderCustomMultiTone(samplingRate, bufferSize, oxying->mWindowSize);
  }
  else
  {
    //error
    return -1;
  }

	return 0;
}


#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_SetAudioSignature(int32_t samplesSize, const float *samplesBuffer, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mEncoder->SetAudioSignature(samplesSize, samplesBuffer);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_EncodeDataToAudioBuffer(const char *stringToEncode, int32_t size, int32_t type, const char *melodyString, int32_t melodySize, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

#ifdef _ANDROID_LOG_
  //  __android_log_print(ANDROID_LOG_INFO, "OxyCoreLibInfo", "OXY_EncodeDataToAudioBuffer %s type %d size %d object %ld", stringToEncode, type, size, (long)oxyingObject );
#endif //_ANDROID_LOG_
  
  return oxying->mEncoder->EncodeDataToAudioBuffer(stringToEncode, type, size, melodyString, melodySize);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_GetEncodedAudioBuffer(float *audioBuffer, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mEncoder->GetEncodedAudioBuffer(audioBuffer);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_ResetEncodedAudioBuffer(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mEncoder->ResetEncodedAudioBuffer();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_DecodeAudioBuffer(float *audioBuffer, int size, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;
  
  //Decode audioBuffer to check if begin token is found, we should keep previous buffer to check if token was started in previous
  //var mDecoding > 0 when token has been found, once decoding is finished, mDecoding = 0
  return oxying->mDecoder->DecodeAudioBuffer(audioBuffer, size);
}

//we should include maxsize?? int32_t maxsize
#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_GetDecodedData(char *stringDecoded, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mDecoder->GetDecodedData(stringDecoded);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float OXY_GetConfidenceError(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mDecoder->GetConfidenceError();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float OXY_GetConfidenceNoise(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mDecoder->GetConfidenceNoise();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float OXY_GetConfidence(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  //return (oxying->mDecoder->GetConfidence()/2.f)+0.5f;
  return oxying->mDecoder->GetConfidence();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float OXY_GetReceivedOxysVolume(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  //return (oxying->mDecoder->GetConfidence()/2.f)+0.5f;
  return oxying->mDecoder->GetReceivedOxysVolume();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_GetDecodedMode(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  //(AUDIBLE = 0, NONAUDIBLE = 1, COMPRESSION = 2, CUSTOM = 3)
  return oxying->mDecoder->GetDecodedMode();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_GetSpectrum(float *spectrumBuffer, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mDecoder->GetSpectrum(spectrumBuffer);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_SetCustomBaseFreq(float baseFreq, int oxysSeparation, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;
  
  if (oxying->mDecoder && oxying->mDecoder->mDecodingMode == Globals::DECODING_MODE_CUSTOM) //Custom mode
  {
    if (oxying->mEncoder)
    {
      delete oxying->mEncoder;
      oxying->mEncoder = 0;
    }
    
    if (oxying->mDecoder)
    {
      delete oxying->mDecoder;
      oxying->mDecoder = 0;
    }

    Globals::freqBaseForCustomMultiTone = baseFreq;
    Globals::oxysSeparationForCustomMultiTone = oxysSeparation;
    
    oxying->mEncoder = new EncoderCustomMultiTone(oxying->mSampleRate, oxying->mBufferSize, oxying->mWindowSize); //configure with default params sr, buffsize
    oxying->mDecoder = new DecoderCustomMultiTone(oxying->mSampleRate, oxying->mBufferSize, oxying->mWindowSize);
  }
  else
  {
    Globals::freqBaseForCustomMultiTone = baseFreq;
    Globals::oxysSeparationForCustomMultiTone = oxysSeparation;
  }

  return 0; //should return real custom freq after quantization
}


#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float OXY_GetDecodingBeginFreq(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mDecoder->GetDecodingBeginFreq();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float OXY_GetDecodingEndFreq(void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  return oxying->mDecoder->GetDecodingEndFreq();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_SetSynthMode(int synthMode, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  Globals::synthMode = synthMode;

  return 0;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t OXY_SetSynthVolume(float synthVolume, void *oxyingObject)
{
  COxyCore *oxying = (COxyCore*)oxyingObject;

  Globals::synthVolume = synthVolume;

  return 0; 
}
