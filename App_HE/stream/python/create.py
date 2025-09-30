from cmsis_stream.cg.scheduler import Graph,Configuration

from appnodes import *

    
the_graph = Graph()

AUDIO_BLOCK = 512 
FFT_SIZE = 512 

# Use CMSIS VStream to connect to microphones
src = VStreamAudioSource("audioSource",AUDIO_BLOCK)
video = VStreamVideoSource("videoSource")

# Debug source can be used instead to generate a sine
# with amplitude modulation
#src = DebugSource("audioSource",AUDIO_BLOCK)
src_f32 = Convert("src_f32",Q15_STEREO,F32_STEREO,AUDIO_BLOCK)

win_left = Hanning("winLeft",AUDIO_BLOCK)
win_right= Hanning("winRight",AUDIO_BLOCK)

stereo_to_mono = StereoToMono("stereoToMono",F32,AUDIO_BLOCK)
to_complex_left = RealToComplex("toComplexLeft",F32,AUDIO_BLOCK)
to_complex_right= RealToComplex("toComplexRight",F32,AUDIO_BLOCK)
fft_left = CFFT("fftLeft",F32_COMPLEX,FFT_SIZE)
fft_right = CFFT("fftRight",F32_COMPLEX,FFT_SIZE)

spectrogram_left = Spectrogram("spectrogramLeft",FFT_SIZE)
spectrogram_right= Spectrogram("spectrogramRight",FFT_SIZE)

display = VStreamVideoSink("display")


the_graph.connect(src.o,src_f32.i)
the_graph.connect(src_f32.o,stereo_to_mono.i)
the_graph.connect(stereo_to_mono.l,win_left.i)
the_graph.connect(stereo_to_mono.r,win_right.i)

the_graph.connect(win_left.o,to_complex_left.i)
the_graph.connect(win_right.o,to_complex_right.i)

the_graph.connect(to_complex_left.o,fft_left.i)
the_graph.connect(to_complex_right.o,fft_right.i)
the_graph.connect(fft_left.o,spectrogram_left.i)
the_graph.connect(fft_right.o,spectrogram_right.i)

the_graph.connect(spectrogram_left[0],display[0])
the_graph.connect(spectrogram_right[0],display[1])
the_graph.connect(video[0],display[2])

#
conf = Configuration()
conf.CMSISDSP = False
conf.asynchronous = False
conf.horizontal=True
conf.nodeIdentification = True
conf.schedName = "scheduler"
conf.memoryOptimization = True

scheduling = the_graph.computeSchedule(config=conf)

print("Schedule length = %d" % scheduling.scheduleLength)
print("Memory usage %d bytes" % scheduling.memory)

scheduling.ccode("../scheduler",conf)
scheduling.genJsonIdentification("../json",conf)
scheduling.genJsonSelectors("../json",conf)
scheduling.genJsonSelectorsInit("../json",conf)

with open("../scheduler/AppNodes.hpp","w") as f:
    s = set([x.typeName for x in scheduling.allNodes])
    for n in s:
        print(f'#include "nodes/{n}.hpp"',file=f)

with open("../scheduler/graph.dot","w") as f:
    scheduling.graphviz(f)