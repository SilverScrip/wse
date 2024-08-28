#ifndef __DECODERCOMPRESSIONMULTITONE__
#define __DECODERCOMPRESSIONMULTITONE__

#include <vector>

#include "Decoder.h"

#define MAX_DECODE_STRING_SIZE 30 //max decoded string size is 30

namespace OXY
{
  class SpectralAnalysis;
  class ReedSolomon;
  //class Decoder;

  class DecoderCompressionMultiTone: public Decoder
  {
  public:
    DecoderCompressionMultiTone(float sr, int buffsize, int windowSize);
    ~DecoderCompressionMultiTone(void);

    int *mIdxs;

    int *mBlockEnergyRatiosMaxToneIdx;
    int *mBlockEnergyRatiosSecondToneIdx;
    int *mToneRepetitions;

    int *idxTonesFrontDoorToken1;
    int *idxTonesFrontDoorToken2;

    int DecodeAudioBuffer(float *audioBuffer, int size);
    int GetDecodedData(char *stringDecoded);

    int GetSpectrum(float *spectrumBuffer);
    
    int AnalyzeStartTokens(float *audioBuffer);
    int AnalyzeToken(float *audioBuffer);

    int ComputeStatsStartTokens(void);
    int ComputeStats(void);

    int getSizeFilledFrameCircularBuffer();
    int getSizeFilledBlockCircularBuffer();
    
    float ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
    float ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
  };
}

#endif //__DECODERCOMPRESSIONMULTITONE__
