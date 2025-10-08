from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class VStreamAudioSink(GenericSink):
    COUNT = 0
    def __init__(self,name,outLength,volume=5,master=True):
        GenericSink.__init__(self,name,selectors=["volume"])
        VStreamAudioSink.COUNT = VStreamAudioSink.COUNT + 1
        if (VStreamAudioSink.COUNT > 1):
            raise Exception("Only one VStreamAudioSink node can be instantiated")
        # Stereo output
        self.addInput("i",Q15_STEREO,outLength)
        self.addEventInput(1)
        self.addLiteralArg(volume)
        self.addLiteralArg(1 if master else 0)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "VStreamAudioSink"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"