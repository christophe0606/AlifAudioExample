from cmsis_stream.cg.scheduler import *


class DebugSource(GenericSource):
    def __init__(self,name,outLength):
        GenericSource.__init__(self,name)
        # Stereo output
        self.addOutput("o",CType(Q15),2*outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DebugSource"