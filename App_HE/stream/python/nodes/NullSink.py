from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class NullSink(GenericSink):
    def __init__(self,name,outLength):
        GenericSink.__init__(self,name)
        # Stereo output
        self.addInput("i",Q15_STEREO,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "NullSink"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"