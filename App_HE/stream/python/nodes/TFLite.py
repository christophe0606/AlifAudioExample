from cmsis_stream.cg.scheduler import GenericSink

from .NodeTypes import *

class TFLite(GenericSink):
    def __init__(self,name,nbInputs=1,nbOutputs=1,addr="nullptr",size=0):
        GenericSink.__init__(self,name)
        # Acknowledge event output to tell
        # producer that the network is ready
        self.addEventInput(nbInputs)
        self.addEventOutput(nbOutputs+1)
        self.addVariableArg(addr)
        self.addLiteralArg(size)



