from cmsis_stream.cg.scheduler import GenericSink

from .AppTypes import *

class VStreamVideoSink(GenericSink):
    def __init__(self,name):
        GenericSink.__init__(self,name)

