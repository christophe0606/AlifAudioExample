from cmsis_stream.cg.scheduler import GenericSource

from .AppTypes import *

class VStreamVideoSource(GenericSource):
    def __init__(self,name,outLength):
        GenericSource.__init__(self,name)
        self.addEventOutput(1)


    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "VStreamVideoSource"