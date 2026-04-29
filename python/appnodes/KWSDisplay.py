from cmsis_stream.cg.scheduler import GenericSink

from ..nodes import VStreamVideoSink

class KWSDisplay(VStreamVideoSink):
    def __init__(self,name):
        VStreamVideoSink.__init__(self,name)
        self.addEventInput(1)
        # eventQueue is needed because this node can send
        # event to itself so it needs the eventQueue
        # to do it
        self.addVariableArg("evtQueue")


    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "KWSDisplay"