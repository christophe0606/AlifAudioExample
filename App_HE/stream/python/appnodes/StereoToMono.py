from cmsis_stream.cg.scheduler import *


class StereoToMono(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name)
        self.addInput("i",theType,2*outLength)
        self.addOutput("l",theType,outLength)
        self.addOutput("r",theType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "StereoToMono"