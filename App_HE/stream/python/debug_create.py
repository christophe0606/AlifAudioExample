from cmsis_stream.cg.scheduler import Graph,Configuration

from nodes import *
from appnodes import *

    
the_graph = Graph()

AUDIO_BLOCK = 512 
FFT_SIZE = 512 

src = DebugSource("audioSource",AUDIO_BLOCK,frequency=440)
speaker = VStreamAudioSink("audioSink",AUDIO_BLOCK)

the_graph.connect(src.o,speaker.i)


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

def maybeFolder(x):
    if hasattr(x, "folder"):
        return x.folder + "/"
    # Standard ndoes from cmsis stream package have no folders
    return ""

with open("../scheduler/AppNodes.hpp","w") as f:
    #print(scheduling.allNodes)
    s = set([(maybeFolder(x),x.typeName) for x in scheduling.allNodes])
    for folder,n in s:
        if folder:
           print(f'#include "{folder}{n}.hpp"',file=f)

with open("../scheduler/graph.dot","w") as f:
    scheduling.graphviz(f)