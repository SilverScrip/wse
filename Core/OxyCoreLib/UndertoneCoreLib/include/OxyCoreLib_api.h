/*--------------------------------------------------------------------------------
 OxySoundCoreLib_api.h
 
 CONFIDENTIAL: This document contains confidential information. 
 Do not disclose any information contained in this document to any
 third-party without the prior written consent of OxyCom, Ltd.
 --------------------------------------------------------------------------------*/

// This file contains all the prototypes needed for
// transmitting numeric data through sound 


#ifndef __OXYCORELIB_API__
#define __OXYCORELIB_API__

#ifndef __APPLE__
  #ifdef OXY_AS_DLL
    #define OXY_DLLEXPORT __declspec(dllexport)
  #else
    #define OXY_DLLEXPORT
  #endif

#else
  #define OXY_DLLEXPORT __attribute__((visibility("default")))
#endif

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

  enum OXY_MODE { OXY_MODE_AUDIBLEOLD=0, OXY_MODE_NONAUDIBLEOLD=1, OXY_MODE_AUDIBLE=2, OXY_MODE_NONAUDIBLE=3, OXY_MODE_COMPRESSION=4, OXY_MODE_ALL=5, OXY_MODE_CUSTOM=6 };
  OXY_DLLEXPORT void *OXY_Create();
  OXY_DLLEXPORT void OXY_Destroy(void *oxyingObject);

  ///////////////////////////////////////////
  ///// VERSIONING //////////////////////////
  ///////////////////////////////////////////
  //Return string with version information
  OXY_DLLEXPORT const char* OXY_GetVersion();

  //Return string with version information
  OXY_DLLEXPORT int32_t OXY_GetVersionInfo(char * versioninfo);

  ///////////////////////////////////////////
  ///// CONFIGURE
  ///////////////////////////////////////////

  //OXY_Configure function, call this function to configure parameters of the OxyComCore Library
  //* Parameters:
  //    mode: mode (2 for audible, 3 for non-audible, dont use other modes, 0 and 1 are old modes)
  //    samplingRate: sampling rate in Hz
  //    nChannels: number of channels of the input audio
  //    oxyingObject: OXY object instance, created in OXY_Create()
  //* Returns: 0=ok, <0=fail
  OXY_DLLEXPORT int32_t OXY_Configure(int mode, float samplingRate, int32_t bufferSize, void *oxyingObject);
 
  //OXY_SetAudioSignature function, call this function to set a personalized audio that will be played 
  // simultaneously during oxying playback on top of non-audible, audible or hidden oxys
  //* Parameters:
  //    samplesSize: number of samples in samples buffer (maximum size is 2 seconds= 44100*2)
  //    samples: array with samples (44Khz, 16bits, mono)
  //    oxyingObject: OXY object instance, created in OXY_Create()
  //* Returns: 0=ok, <0=fail

  OXY_DLLEXPORT int32_t OXY_SetAudioSignature(int32_t samplesSize, const float *samplesBuffer, void *oxyingObject);
 
  //OXY_EncodeDataToAudioBuffer function
  //* Parameters:
  //    stringToEncode: string containing the characters to encode
  //    size: number of characters in string characters to encode
  //    type: 0 for encoding only tones, 1 for encoding tones + R2D2 sounds, 2 for encoding melody
  //    melodyString: string containing characters to synthesize melody over the tones (null if type parameter is 0 or 1)
  //    melodySize: size of melody in number of notes (0 if type parameter is 0 or 1)
  //    oxyingObject: OXY object instance, created in OXY_Create()
  //* Returns: number of samples in encoded audio buffer

  OXY_DLLEXPORT int32_t OXY_EncodeDataToAudioBuffer(const char *stringToEncode, int32_t size, int32_t type, const char *melodyString, int32_t melodySize, void *oxyingObject);
 

  //OXY_GetEncodedAudioBuffer function
  //* Parameters:
  //    audioBuffer: float array of bufferSize size to fill with encoded audio data
  //    oxyingObject: OXY object instance, created in OXY_Create()  
  //* Returns: number of samples read, maximum will be configured bufferSize, 0 or < bufferSize means that end has been reached
  OXY_DLLEXPORT int32_t OXY_GetEncodedAudioBuffer(float *audioBuffer, void *oxyingObject);

  //OXY_CreateAudioBufferFromData function, resets the read index on the internal buffer that has the encoded string
  //* Parameters:
  //    oxyingObject: OXY object instance, created in OXY_Create()  
  //* Returns: 0=ok, <0=fail
  OXY_DLLEXPORT int32_t OXY_ResetEncodedAudioBuffer(void *oxyingObject);


  //OXY_DecodeAudioBuffer function, receives an audiobuffer of specified size and outputs if encoded data is found
  //* Parameters:
  //    audioBuffer: float array of bufferSize size with audio data to be decoded
  //    size: size of audioBuffer
  //    oxyingObject: OXY object instance, created in OXY_Create()  
  //* Returns: -1 if no decoded data is found, -2 if start token is found, -3 if complete word has been decoded, positive number if character is decoded (number is the token idx)

  OXY_DLLEXPORT int32_t OXY_DecodeAudioBuffer(float *audioBuffer, int size, void *oxyingObject);

  //OXY_GetDecodedData function, retrieves the last decoded data found
  //* Parameters:
  //    stringDecoded: string containing decoded characters
  //    oxyingObject: OXY object instance, created in OXY_Create()
  //* Returns: 0 if no decoded data is available, >0 if data is available and it's ok, <0 if data is available but it's wrong, for the last two cases the return value magnitude contains number of characters in string decoded
  OXY_DLLEXPORT int32_t OXY_GetDecodedData(char *stringDecoded, void *oxyingObject);
  //we should include maxsize?? int32_t maxsize

  //OXY_GetConfidence function, outputs Reception Quality Measure to give confidence about the received audio. 
  // A Reception Quality value of 1.0 will mean that the reception conditions are ideal, a lower value will mean that 
  // listener is in a noisy environment, the listener should be closer to the transmitter, etc.
  //* Parameters:
  //    oxyingObject: OXY object instance, created in OXY_Create()
  //* Returns: confidence value from 0.0 o 1.0
  OXY_DLLEXPORT float OXY_GetConfidence(void *oxyingObject); //Get global confidence (combination of the other confidence values)
  OXY_DLLEXPORT float OXY_GetConfidenceError(void *oxyingObject); //Get confidence due to tokens corrected by correction algorithm
  OXY_DLLEXPORT float OXY_GetConfidenceNoise(void *oxyingObject); //Get confidence due to signal to noise ratio in received audio

  OXY_DLLEXPORT float OXY_GetReceivedOxysVolume(void *oxyingObject); // Get average received volume of last audio transmission in DB

  //OXY_GetDecodedMode function, outputs an integer representation of the decoded mode found from all 
  // available decoding modes, it only makes sense when decoder is configured with the ALL mode, for other modes
  // decoded mode will be always the same as the decoding mode.
  //* Parameters:
  //    none
  //* Returns: decoded mode found ( AUDIBLE = 0, NONAUDIBLE = 1, COMPRESSION = 2 )
  OXY_DLLEXPORT int32_t OXY_GetDecodedMode(void *oxyingObject);

  /////////////////////////////////////////////////////////////////////////////
  // FOR CUSTOM MODE //////////////////////////////////////////////////////////

  OXY_DLLEXPORT int32_t OXY_SetCustomBaseFreq(float baseFreq, int oxysSeparation, void *oxyingObject);

  /////////////////////////////////////////////////////////////////////////////
  // Functions to get decoding frequency range (begin range frequency and end range frequency)
  OXY_DLLEXPORT float OXY_GetDecodingBeginFreq(void *oxyingObject);
  OXY_DLLEXPORT float OXY_GetDecodingEndFreq(void *oxyingObject);

  /////////////////////////////////////////////////////////////////////////////
  // FOR SYNTH MODE //////////////////////////////////////////////////////////

  OXY_DLLEXPORT int32_t OXY_SetSynthMode(int synthMode, void *oxyingObject);
  OXY_DLLEXPORT int32_t OXY_SetSynthVolume(float synthVolume, void *oxyingObject);


  //Not used
  OXY_DLLEXPORT int32_t OXY_GetSpectrum(float *spectrumBuffer, void *oxyingObject);
  

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__OXYCORELIB_API__
