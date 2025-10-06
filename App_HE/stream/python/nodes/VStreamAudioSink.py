from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class VStreamAudioSink(GenericSink):
    def __init__(self,name,outLength,volume=5):
        GenericSink.__init__(self,name)
        # Stereo output
        self.addInput("i",Q15_STEREO,outLength)
        self.addLiteralArg(volume)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "VStreamAudioSink"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"