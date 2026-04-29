from cmsis_stream.cg.scheduler import GenericNode,CType,F32

class MFCC(GenericNode):
    def __init__(self,name,inLength,outLength):
        GenericNode.__init__(self,name)
        self.addInput("i",CType(F32),inLength)
        self.addOutput("o",CType(F32),outLength)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "appnodes"
    
    @property
    def typeName(self):
        return "MFCC"
    
    # Specializations already provided by the C++ and that should not be
    # instantiated again when enabling code size optimization
    @classmethod
    def specializations(cls):
        return set(["<float,640,float,10>"])