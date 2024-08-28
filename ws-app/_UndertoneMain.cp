#include "OxyCoreLib_api.h"

#include "Globals.h"

#include "sndfile.h"
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <cassert>
#include <math.h>

#include "CliParser.hxx"
#include "Mixer.h"

#include "Loudness.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>

//#include "client_ws.hpp"
//#include "server_ws.hpp"
//#include <future>

#include "client_wss.hpp"
#include "server_wss.hpp"
#include <future>


#ifndef MIN
  #define MIN(a,b) ((a <= b) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a,b) ((a >= b) ? (a) : (b))
#endif

using namespace std;

using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

WssServer server("/app/ws-app/build/server.crt", "/app/ws-app/build/server.key");

struct AudioData
{
    Uint8 *idx;
    Uint32 len;
};

void callbackfun(void *data, uint8_t *buffer, int buffer_len)
{
    AudioData *audio = (AudioData *)data;

    if (audio->len == 0)
    {
        return;
    }

    uint32_t length = (uint32_t)buffer_len;
    length = (length > audio->len ? audio->len : length);

    memcpy(buffer, audio->idx, length);

    audio->idx += length;
    audio->len -= length;
}

char getCharFromIdx(int idx)
{
  if      (idx==0)  return '0';
  else if (idx==1)  return '1';
  else if (idx==2)  return '2';
  else if (idx==3)  return '3';
  else if (idx==4)  return '4';
  else if (idx==5)  return '5';
  else if (idx==6)  return '6';
  else if (idx==7)  return '7';
  else if (idx==8)  return '8';
  else if (idx==9)  return '9';
  else if (idx==10) return 'a';
  else if (idx==11) return 'b';
  else if (idx==12) return 'c';
  else if (idx==13) return 'd';
  else if (idx==14) return 'e';
  else if (idx==15) return 'f';
  else if (idx==16) return 'g';
  else if (idx==17) return 'h';
  else if (idx==18) return 'i';
  else if (idx==19) return 'j';
  else if (idx==20) return 'k';
  else if (idx==21) return 'l';
  else if (idx==22) return 'm';
  else if (idx==23) return 'n';
  else if (idx==24) return 'o';
  else if (idx==25) return 'p';
  else if (idx==26) return 'q';
  else if (idx==27) return 'r';
  else if (idx==28) return 's';
  else if (idx==29) return 't';
  else if (idx==30) return 'u';
  else if (idx==31) return 'v';
  else              return '0';
}

char *fromDecToBase(int num, int rad)
{
  char digits[] = "0123456789abcdefghijklmnopqrstuvWXYZabcdefghijklmnopqrstuvwxyz";
  int i;
  char buf[66];   /* enough space for any 64-bit in base 2 */
  buf[0]=0;

  /* bounds check for radix */
  if (rad < 2 || rad > 62)
      return NULL;
  /* if num is zero */
  if (!num)
      return strdup("0");

  /* null terminate buf, and set i at end */
  buf[65] = '\0';
  i = 65;

  if (num > 0) {  /* if positive... */
      while (num) { /* until num is 0... */
          /* go left 1 digit, divide by radix, and set digit to remainder */
          buf[--i] = digits[num % rad];
          num /= rad;
      }
  } else {    /* same for negative, but negate the modulus and prefix a '-' */
      while (num) {
          buf[--i] = digits[-(num % rad)];
          num /= rad;
      }
      buf[--i] = '-';
  }
  /* return a duplicate of the used portion of buf */
  return strdup(buf + i);
}

int charToVal(char curChar)
{
  char convArray[] =
  {'0','1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v' };
  for(int i=0;i<32; ++i)
  {
    if(curChar==convArray[i])
    {
      return i;
    }
  }
  return -1;
}

int fromBaseToDec(char * number, int length, int rad)
{
  int decimal = 0;
  int factor = 1;
  for (int i=length-1; i >=0; --i)
  {
    int curVal = charToVal(number[i]);
    decimal += factor*curVal;
    factor*=rad;
  }
  return decimal;
}

int generateAudio(string const keyStr){
    
    
    
    SDL_AudioSpec aud_wav_spec;

    Uint8 *wav_start;
    Uint32 wav_len;
    

    
    void* mOxyCore;
    


  //Should be equal to the one in Globals::durToken

  double durToken = Globals::durToken; //dur in seconds for each token
  int bufferSize = 128;

  int i = 0;

  clock_t total_start,total_end;
  total_start = clock();
  //Uncomment to put Undertone back into default mode ultrasonic otherwise all modes are accessible
  const int param_mode = 2;

    string inputFnStr = ""; //chunk2;
    //string keyStr = "12345";
    const float duration = 5.1;
    const float interval = 10.0;
    float startTime = 5.0;
    string outputFnStr = keyStr + ".wav";

    const int mixmode = 0;
    const float volumeoxys = -3.f;
    const float volumeprogram = 0.f;
    const float sampleRate = 44100.0;

    const int loudnessStats = 0;

    const float baseFreq = 12000.0;
    const int tonesSeparation = 1;

    const int synthMode = 0;
    const int speakerMode = 0;
    const float synthVolume = 0.0;
    
  //CHECK THAT PARAMETERS ARE CORRECT
  if (keyStr.size() != 5)
  {
    cerr << "Nothing todo" << endl;
    return -1;
  }
  else //check that digits are valid
  {
    for (int i = 0; i < keyStr.size(); i++)
    {
      if (charToVal(keyStr.c_str()[i]) == -1)
      {
          cerr << "Nothing todo" << endl;
       // cerr << "Wrong character in key [" << keyStr.c_str()[i] << "]. Please use digits in {0-9, a-v} range only" << endl;
        return -1;
      }
    }
  }
     

  float min_startTime = (durToken*20.f) + 0.1f;
  float min_interval = (durToken*20.f) + 0.2f;
  float min_duration = startTime + 0.1f;

  if (interval < min_interval)
  {
    cerr << "Interval too short. The minimum allowed interval is " << min_interval << " seconds" << endl;
    return -1;
  }
  if (duration < min_duration)
  {
    cerr << "Duration too short. The minimum allowed duration is " << min_duration << " seconds (start time + 0.1 seconds)" << endl;
    return -1;
  }
  if (duration > 86400.f) //24 hours of wav 44Khz 16 bits mono is 7.620.480.000 bytes
  { //1 hour is 317.520.000 bytes (317 Mbytes)
    cerr << "Duration too big. The maximum allowed duration is 86400 seconds (=24 hours)" << endl;
    return -1;
  }
  if ((startTime > duration) || (startTime < min_startTime)) //valid start time for first audio mark
  {
    cerr << "Start time is not valid. It should be > " << min_startTime << " secs." << endl;
    return -1;
  }

  startTime = MAX(startTime, min_startTime);

  int mode = /*UNDERTONE_MODE::*/OXY_MODE_NONAUDIBLE;
  if (param_mode == 0)
    mode = OXY_MODE_AUDIBLE;
  else if (param_mode == 1)
    mode = OXY_MODE_COMPRESSION;
  else if (param_mode == 2)
    mode = OXY_MODE_NONAUDIBLE;
  else if (param_mode == 3)
    mode = OXY_MODE_CUSTOM;


  //Creation
  mOxyCore = OXY_Create();

  if (param_mode == 3)
  {
    OXY_SetCustomBaseFreq(baseFreq, tonesSeparation, mOxyCore);

    cerr << "Custom mode" << outputFnStr.c_str() << endl;
  }

  OXY_SetSynthMode(synthMode, mOxyCore);
  OXY_SetSynthVolume(synthVolume, mOxyCore);


  //OUTPUT FILE
  SF_INFO sfinfoOutput;
  memset(&sfinfoOutput, '\0', sizeof(sfinfoOutput));
  SNDFILE *pWaveFileOutput = NULL;

  if (inputFnStr.size() == 0) //No input audio generate output of course
  {
    //Configuration
    OXY_Configure(mode, sampleRate, bufferSize, mOxyCore);

    //CREATE OUTPUT AUDIO FILE
    sfinfoOutput.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    sfinfoOutput.channels = 1; //or 2
    sfinfoOutput.frames = 0;
    sfinfoOutput.samplerate = (int)sampleRate;

    pWaveFileOutput = sf_open(outputFnStr.c_str(), SFM_WRITE, &sfinfoOutput);
    if (!pWaveFileOutput)
    {
      cerr << "Gave up!, program refushed to create Output WaveFile " << outputFnStr.c_str() << endl;
      return -1;
    }

    double currentTimeInSeconds = 0.0;
    double nextMarkTime = currentTimeInSeconds + startTime;

    float defUndertoneLevel = pow(10.f, volumeoxys / 20.f);


    //ENCODE *******************************************************

    float *silenceBuffer = new float[bufferSize];
    memset(silenceBuffer, 0, bufferSize * sizeof(float));

    float *audioBuffer = new float[bufferSize];

    int progress_oxy = 0;
    cout << "Audio build = " << progress_oxy << endl;

    while (currentTimeInSeconds < duration)
    {
      float current_progress_oxy = (currentTimeInSeconds / duration)*100.f;
      if (current_progress_oxy > progress_oxy + 5)
      {
        progress_oxy = current_progress_oxy;
        cout << "Audio build = " << progress_oxy << endl;
      }

      if (currentTimeInSeconds >= (nextMarkTime - (durToken*20.f)))
      {
        int timestampInSeconds = (int)(nextMarkTime + 0.5f);

        char* timestamp;
        timestamp = fromDecToBase(timestampInSeconds, 32);

        char currentTimestamp[5] = "\0";
        int len = strlen(timestamp);
        for (int i = len; i < 4; i++)
        {
          strcat(currentTimestamp, "0");
        }
        strcat(currentTimestamp, timestamp);

        char stringToDecode[10];
        sprintf(stringToDecode, "%s%s", keyStr.c_str(), currentTimestamp);

        int size = strlen(stringToDecode);

        int type = Globals::synthMode;

        int sizeAudioBuffer = OXY_EncodeDataToAudioBuffer(stringToDecode, size, type, 0, 0, mOxyCore);

        int samplesRetrieved = 0;

        do
        {
          //set to size of audioBuffer
          memset(audioBuffer, 0, bufferSize * sizeof(float));
          samplesRetrieved = OXY_GetEncodedAudioBuffer(audioBuffer, mOxyCore);

          //adjust volume based on parameter volume of undertone

          for (int i = 0; i < bufferSize; i++)
          {
            audioBuffer[i] = defUndertoneLevel * audioBuffer[i];
          }

          int count = (int)sf_write_float(pWaveFileOutput, audioBuffer, samplesRetrieved);

          currentTimeInSeconds = currentTimeInSeconds + (double)samplesRetrieved / sampleRate;

        } while (samplesRetrieved > 0);

        OXY_ResetEncodedAudioBuffer(mOxyCore);

        nextMarkTime += interval;
      }
      else
      {
        //add silence between marks
       sf_write_float(pWaveFileOutput, silenceBuffer, bufferSize);
        currentTimeInSeconds = currentTimeInSeconds + bufferSize / sampleRate;
      }
    }

    delete[] silenceBuffer;
    delete[] audioBuffer;

    cout << "Audio built = " << 100 << endl;
      


      
  }
  else //MIX WITH INPUT AUDIO
  {
    //READ INPUT FILE TO BUFFER
    SNDFILE *pWaveFileInput = NULL;
    SF_INFO sfinfoInput;
    memset(&sfinfoInput, '\0', sizeof(sfinfoInput));
    float **ppInputBuffer = NULL;

    pWaveFileInput = sf_open(inputFnStr.c_str(), SFM_READ, &sfinfoInput);

    long nFrames;
    int nch;
    float sampleRate;
    int buffersamples = 4096;

    if (pWaveFileInput) // read Input File to buffer
    {
      nFrames = sfinfoInput.frames;
      nch = (int)sfinfoInput.channels;
      sampleRate = (float)sfinfoInput.samplerate;
      if ((sampleRate != 44100.f) && (sampleRate != 48000.f) && (sampleRate != 16000.f))
      {
        printf("%s is not a valid Wav File! Please use 44.1Khz or 48Khz 16bits PCM Wave File\n", inputFnStr.c_str());
        sf_close(pWaveFileInput);
        return -2;
      }

      //Configuration of core lib
      OXY_Configure(mode, sampleRate, bufferSize, mOxyCore);

      //Create audio file as output
      sfinfoOutput.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
      sfinfoOutput.channels = 1;
      sfinfoOutput.frames = 0;
      sfinfoOutput.samplerate = (int)sampleRate;

      pWaveFileOutput = sf_open(outputFnStr.c_str(), SFM_WRITE, &sfinfoOutput);
      if (!pWaveFileOutput)
      {
        cerr << "Time again for the bat signal, Cant create Output WaveFile " << outputFnStr.c_str() << endl;
        return -1;
      }

      float *pInputBufferInterleaved = new float[buffersamples*nch];

      ppInputBuffer = new float*[nch];
      for (int i = 0; i < nch; i++)
        ppInputBuffer[i] = new float[nFrames];

      int readsamples = 0;
      while (readsamples < nFrames*nch)
      {
        int ReadCount = (int)sf_read_float(pWaveFileInput, pInputBufferInterleaved, buffersamples*nch);

        //Copy from interleaved to buffers
        for (int t = 0; t < nch; t++)
        {
          for (int i = 0; i < (ReadCount/nch); i++)
          {
            ppInputBuffer[t][(readsamples/nch)+i] = pInputBufferInterleaved[i*nch + t];
          }
        }

        readsamples += ReadCount;
      }

      //Delete interleaved prevent memory leak
      delete[] pInputBufferInterleaved;
      sf_close(pWaveFileInput);

    }
    else
    {
      printf("%s is not a valid Wav File or file not found! Please use 44.1Khz or 48Khz 16bits PCM Wave File\n", inputFnStr.c_str());
      return -2;
    }

    // Calculate Undertone in buffer

    int progress_oxy = 0;
    cout << "Build Progress = " << progress_oxy << endl;

    float *pUndertoneBuffer = new float[nFrames + (int)((durToken*20.f)*sampleRate)];
    
    //added duration of one message just to avoid buffer overflow when starts at the end of file (not sure it is needed though)

    memset(pUndertoneBuffer, 0, nFrames*sizeof(float));

    long counterSamples = 0;
    double currentTimeInSeconds = 0.0;
    double nextMarkTime = currentTimeInSeconds + startTime;
    float input_duration = nFrames / sampleRate;
    while (currentTimeInSeconds < (input_duration-(bufferSize/sampleRate)))
    {
      float current_progress_oxy = (currentTimeInSeconds / input_duration)*100.f;
      if (current_progress_oxy > progress_oxy + 5)
      {
        progress_oxy = current_progress_oxy;
        cout << "Build Progress = " << progress_oxy << endl;
      }

      if (currentTimeInSeconds >= (nextMarkTime - (durToken*20.f)))
      {
        int timestampInSeconds = (int)(nextMarkTime + 0.5f);

        char* timestamp;
        timestamp = fromDecToBase(timestampInSeconds, 32);

        char currentTimestamp[5] = "\0";
        int len = strlen(timestamp);
        for (int i = len; i < 4; i++)
        {
          strcat(currentTimestamp, "0");
        }
        strcat(currentTimestamp, timestamp);

        char stringToDecode[10];
        sprintf(stringToDecode, "%s%s", keyStr.c_str(), currentTimestamp);

        int size = strlen(stringToDecode);
        int type = synthMode;

        int sizeAudioBuffer = OXY_EncodeDataToAudioBuffer(stringToDecode, size, type, 0, 0, mOxyCore);

        int samplesRetrieved = 0;

        do
        {
          samplesRetrieved = OXY_GetEncodedAudioBuffer(pUndertoneBuffer+counterSamples, mOxyCore);

          counterSamples += samplesRetrieved;

          currentTimeInSeconds = currentTimeInSeconds + (double)samplesRetrieved / sampleRate;

        } while (samplesRetrieved > 0);

        OXY_ResetEncodedAudioBuffer(mOxyCore);

        nextMarkTime += interval;
      }
      else
      {
        //add silence between marks

        counterSamples += bufferSize;
        currentTimeInSeconds = currentTimeInSeconds + bufferSize / sampleRate;
      }
    }

    cout << "Build Progress = " << 100 << endl;

    //MIX BUFFERS

    Mixer mixer;
    mixer.setUndertoneLevel(volumeoxys);
    mixer.setMinUndertoneLevel(-20.f);
    mixer.setProgramLevel(volumeprogram);
    mixer.setMode(mixmode);
    mixer.setUseNormalize(false);

    float **ppMixedBuffer = new float*[nch];
    for (int i = 0; i < nch; i++)
      ppMixedBuffer[i] = new float[nFrames];

    mixer.mix((const float**)ppInputBuffer, nFrames, nch, sampleRate, pUndertoneBuffer, ppMixedBuffer);

    delete[] pUndertoneBuffer;

    //Write mixes audio to output file
    int progress_save = 0;
    cout << "Progress SAVE = " << progress_save << endl;
    sfinfoInput.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    pWaveFileOutput = sf_open(outputFnStr.c_str(), SFM_WRITE, &sfinfoInput);
    if (!pWaveFileOutput)
    {
      if (ppInputBuffer)
      {
        for (int i = 0; i < nch; i++)
        {
          delete[] ppInputBuffer[i];
        }
        delete[] ppInputBuffer;
      }
      ppInputBuffer = NULL;

      if (ppMixedBuffer)
      {
        for (int i = 0; i < nch; i++)
        {
          delete[] ppMixedBuffer[i];
        }
        delete[] ppMixedBuffer;
      }
      ppMixedBuffer = NULL;
      
      printf("Something happened %s!\n", outputFnStr.c_str());

      return -4;
    }
    else
    {
      int buffersamples = 4096;
      float *pOutputBufferInterleaved = new float[buffersamples*nch];

      int samplesread = 0;

      while (samplesread < nFrames)
      {
        float current_progress_save = ((float)samplesread / (float)nFrames)*100.f;
        if (current_progress_save > progress_save + 5)
        {
          progress_save = current_progress_save;
          cout << "Progress SAVE = " << progress_save << endl;
        }

        int samplesToWrite = MIN(buffersamples, nFrames - samplesread);

        for (int t = 0; t < nch; t++)
        {
          for (int i = 0; i < samplesToWrite; i++)
          {
            pOutputBufferInterleaved[i*nch + t] = ppMixedBuffer[t][samplesread+i];
          }
        }

        int count = (int)sf_write_float(pWaveFileOutput, pOutputBufferInterleaved, samplesToWrite*nch);

        samplesread += samplesToWrite;
      }

      delete[] pOutputBufferInterleaved;

      if (ppMixedBuffer)
      {
        for (int i = 0; i < nch; i++)
        {
          delete[] ppMixedBuffer[i];
        }
        delete[] ppMixedBuffer;
      }
      ppMixedBuffer = NULL;

      sf_close(pWaveFileOutput);

    }

    cout << "Progress SAVE = " << 100 << endl;

  }

  if (loudnessStats == 1)
  {
    cout << "Loud Stats: " << endl;

    //LUFS stands for Loudness Unit Full Scale, which references Loudness Units to full scale (i.e., the maximum level a system can handle).

    double lufs = test_global_loudness(outputFnStr.c_str());
    cout << " Lufs:      " << lufs << " dB" << endl;

    double tp = test_true_peak(outputFnStr.c_str());
    cout << " True Peak: " << tp << " dB" << endl;

    cout << "If empty not using custom freq: " << endl;
    double bf = OXY_GetDecodingBeginFreq(mOxyCore);
    cout << " Begin Freq: " << bf << " Hz" << endl;
    double ef = OXY_GetDecodingEndFreq(mOxyCore);
    cout << " End Freq:   " << ef << " Hz" << endl;
  }

  //Destroy

  OXY_Destroy(mOxyCore);

  sf_close(pWaveFileOutput);

  total_end = clock();
  double totalDuration = double(total_end - total_start) / (double)CLOCKS_PER_SEC;
  cout << "Total Duration: " << totalDuration << " secs" << endl;

    // speaker mode
    if (speakerMode==1) {
    if(SDL_LoadWAV(outputFnStr.c_str(), &aud_wav_spec, &wav_start, &wav_len) == NULL)
    {
        cout << "File load error" << endl;
        return 1;
    }
    else
    {
    cout << "File loaded" << endl;
    }

    AudioData audio;
    audio.idx = wav_start;
    audio.len = wav_len;


    cout << "Calling callback function" << endl;

    aud_wav_spec.callback = callbackfun;
    aud_wav_spec.userdata = &audio;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL,0, &aud_wav_spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(device == 0)
    {
        cout << "Audio device error" << endl;
        SDL_Quit();
        return 1;
    }
    else
    {
    cout << "Audio device success" << endl;
    }

    SDL_PauseAudioDevice(device,0);
    cout << ":::Playing audio:::" << endl;

    while(audio.len > 0)
    {
        SDL_Delay(100);
        //cout << "Looping \n";
    }
    
        //Close audio device
        SDL_CloseAudioDevice(device);
        cout << "Audio device free" << endl;
        SDL_FreeWAV(wav_start);
        cout << "File free" << endl;
        
    }
    
    
  return 0;
}

void serverwss(){
    

    auto &echo = server.endpoint["^/audio/?$"];
    
    echo.on_message = [](shared_ptr<WssServer::Connection> connection, shared_ptr<WssServer::InMessage> in_message) {
        auto out_message = in_message->string();
        
        cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << endl;
        
        const int chunkSize = 5;

            // Splitting the string into chunks
            std::string chunk3 = out_message.substr(0, out_message.length() < chunkSize ? out_message.length() : chunkSize);
        
        cout << "Server: Sending message \"" << chunk3 << "\" to " << connection.get() << endl;
        
        generateAudio(out_message);
        
        // connection->send is an asynchronous function
        connection->send(out_message, [](const SimpleWeb::error_code &ec) {
            if(ec) {
                cout << "Server: Error sending message. " <<
                // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                "Error: " << ec << ", error message: " << ec.message() << endl;
            }
        });
        
    };
    
    echo.on_open = [](shared_ptr<WssServer::Connection> connection) {
        cout << "Server: Opened connection " << connection.get() << endl;
    };
    
    // See RFC 6455 7.4.1. for status codes
    echo.on_close = [](shared_ptr<WssServer::Connection> connection, int status, const string & /*reason*/) {
        cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
    };
    
    // Can modify handshake response header here if needed
    echo.on_handshake = [](shared_ptr<WssServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
      return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
    };
    
    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    echo.on_error = [](shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code &ec) {
        cout << "Server: Error in connection " << connection.get() << ". "
        << "Error: " << ec << ", error message: " << ec.message() << endl;
    };
    
    
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    
    server.config.port = 8080;
    
    // WebSocket (WS)-server at port 8080 using 1 thread
    serverwss();
    
    // Start server and receive assigned port when server is listening for requests
    promise<unsigned short> server_port;
    thread server_thread([&]() {
      // Start server
      server.start([&server_port](unsigned short port) {
        server_port.set_value(port);
      });
    });
    
    cout << "Server listening on port " << server_port.get_future().get() << endl
         << endl;
    
    server_thread.join();
    
    SDL_Quit();
    
    return 0;
}



