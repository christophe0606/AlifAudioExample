from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class EmptySource(GenericSource):
    def __init__(self,name,theType,outLength):
        GenericSource.__init__(self,name,identified=True)
        # Stereo output
        self.addOutput("o",theType,outLength)
        self.addVariableArg(f"params->{name}")

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "EmptySource"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"