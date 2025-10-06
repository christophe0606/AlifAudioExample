from cmsis_stream.cg.scheduler import GenericSource
from nodes import *

class DebugSource(GenericSource):
    def __init__(self,name,outLength,frequency=440,samplingFreq=16000,master=False):
        GenericSource.__init__(self,name)
        # Stereo output
        self.addOutput("o",Q15_STEREO,outLength)
        self.addLiteralArg(frequency)
        self.addLiteralArg(samplingFreq)
        self.addLiteralArg(master)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DebugSource"