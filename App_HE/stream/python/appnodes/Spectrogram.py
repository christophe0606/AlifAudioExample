from cmsis_stream.cg.scheduler import *


    
class Spectrogram(GenericSink):
    def __init__(self,name,nbSamples):
        GenericSink.__init__(self,name)
        self.addInput("i",CType(F32),2*nbSamples)
        self.addEventOutput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Spectrogram"