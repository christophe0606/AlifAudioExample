from cmsis_stream.cg.scheduler import GenericSink


    
class DisplayGPU(GenericSink):
    def __init__(self,name):
        GenericSink.__init__(self,name)
        self.addEventInput(2)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "DisplayGPU"