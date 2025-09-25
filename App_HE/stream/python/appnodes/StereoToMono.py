from cmsis_stream.cg.scheduler import GenericNode,F32,Q15
from .AppTypes import *


class StereoToMono(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name)
        if theType == F32:
            inputType = F32_STEREO
            outputType = F32_SCALAR
        elif theType == Q15:
            inputType = Q15_STEREO
            outputType = Q15_SCALAR
        else:
            raise ValueError("Unsupported type for StereoToMono: {}".format(theType))
        self.addInput("i",inputType,outLength)
        self.addOutput("l",outputType,outLength)
        self.addOutput("r",outputType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "StereoToMono"