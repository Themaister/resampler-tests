TARGET := resampler-test-delta resampler-test-alias resampler-test-alias-down


CFLAGS += -O2 -g -Wall -std=gnu99 -pedantic $(shell pkg-config speexdsp sndfile samplerate libavresample libswresample libavutil --cflags)
LDFLAGS += $(shell pkg-config speexdsp sndfile samplerate libavresample libswresample libavutil --libs) -lm

all: $(TARGET)

resampler-test-delta: resampler-test-delta.o

resampler-test-alias: resampler-test-alias.o

resampler-test-alias-down: resampler-test-alias-down.o

resampler-test-delta.o: resampler-test.c
	$(CC) -c -o $@ $< $(CFLAGS)

resampler-test-alias.o: resampler-test.c
	$(CC) -c -o $@ $< $(CFLAGS) -DTEST_ALIAS

resampler-test-alias-down.o: resampler-test.c
	$(CC) -c -o $@ $< $(CFLAGS) -DTEST_ALIAS_DOWN

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: clean

