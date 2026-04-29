from cmsis_stream.cg.scheduler import GenericSource

from .NodeTypes import *

class VStreamVideoSource(GenericSource):
    def __init__(self,name):
        GenericSource.__init__(self,name,identified=True)
        self.addEventOutput(1)
         # hw_ is common to all node and does not name a specific node
        self.addVariableArg(f"params->hw_")



    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "VStreamVideoSource"

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "nodes"