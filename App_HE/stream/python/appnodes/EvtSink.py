from cmsis_stream.cg.scheduler import *


    
class EvtSink(GenericSink):
    def __init__(self,name):
        GenericSink.__init__(self,name,selectors=["increment","value"])
        self.addEventInput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "EvtSink"