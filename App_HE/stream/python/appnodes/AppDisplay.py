from cmsis_stream.cg.scheduler import GenericSink

from .AppTypes import *
from .VStreamVideoSink import *

class AppDisplay(VStreamVideoSink):
    def __init__(self,name):
        VStreamVideoSink.__init__(self,name)
        self.addEventInput(3)


    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "AppDisplay"