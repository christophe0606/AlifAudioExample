from cmsis_stream.cg.scheduler import *


class EvtSource(GenericSource):
    def __init__(self,name):
        GenericSource.__init__(self,name,selectors=["increment"])
        self.addEventOutput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "EvtSource"