from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class VStreamAudioSource(GenericSource):
    def __init__(self,name,outLength,master=True):
        GenericSource.__init__(self,name)
        # Stereo output
        self.addOutput("o",Q15_STEREO,outLength)
        self.addLiteralArg(1 if master else 0)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "VStreamAudioSource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"