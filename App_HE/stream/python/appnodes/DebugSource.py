from cmsis_stream.cg.scheduler import GenericSource
from nodes import *

class DebugSource(GenericSource):
    def __init__(self,name,outLength):
        GenericSource.__init__(self,name)
        # Stereo output
        self.addOutput("o",Q15_STEREO,outLength)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DebugSource"