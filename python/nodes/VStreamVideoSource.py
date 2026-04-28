from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class VStreamVideoSource(GenericSource):
    def __init__(self,name):
        GenericSource.__init__(self,name)
        self.addEventOutput(1)


    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "VStreamVideoSource"

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"