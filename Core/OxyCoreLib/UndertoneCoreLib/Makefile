OBJS = \
  ./src/OxyCoreLib_api.o \
  ./src/Encoder.o \
  ./src/Decoder.o \
  ./src/EncoderAudibleMode.o \
  ./src/DecoderAudibleMode.o \
  ./src/EncoderNonAudibleMode.o \
  ./src/DecoderNonAudibleMode.o \
  ./src/EncoderAudibleMultiToneMode.o \
  ./src/DecoderAudibleMultiToneMode.o \
  ./src/EncoderNonAudibleMultiToneMode.o \
  ./src/DecoderNonAudibleMultiToneMode.o \
  ./src/EncoderCompressionMultiToneMode.o \
  ./src/DecoderCompressionMultiToneMode.o \
  ./src/EncoderCustomMultiToneMode.o \
  ./src/DecoderCustomMultiToneMode.o \
  ./src/DecoderAllMultiToneMode.o \
  ./src/Globals.o \
  ./src/SpectralAnalysis.o \
  ./src/ReedSolomon.o \
  ./src/Hex.o

all: libOxyCore.so

DEPS=$(OBJS:.o=.d)

depend: $(DEPS)

include $(DEPS)

libOxyCore.so: $(OBJS)
	g++ -shared -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(DEPS) libOxyCore.so

CXXFLAGS= -w -DLINUX -I./include -I. -I././src//fftooura \
          -O3 -DNDEBUG

%.o: %.c
	g++ $(CXXFLAGS) -c -fPIC -o $@ $<

%.d: %.c
	echo $(@:.d=.o): \\> $@
	g++ $(CXXFLAGS) -M -o - $< | sed s/.*:// >> $@

%.o: %.cxx
	g++ $(CXXFLAGS) -c -fPIC -o $@ $<

%.d: %.cxx
	echo $(@:.d=.o): \\> $@
	g++ $(CXXFLAGS) -M -o - $< | sed s/.*:// >> $@

%.o: %.cpp
	g++ $(CXXFLAGS) -c -fPIC -o $@ $<

%.d: %.cpp
	echo $(@:.d=.o): \\> $@
	g++ $(CXXFLAGS) -M -o - $< | sed s/.*:// >> $@