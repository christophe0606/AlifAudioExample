from cmsis_stream.cg.scheduler import *


class Convert(GenericNode):
    def __init__(self,name,srcType,dstType,outLength):
        GenericNode.__init__(self,name)
        self.addInput("i",srcType,outLength)
        self.addOutput("o",dstType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Convert"