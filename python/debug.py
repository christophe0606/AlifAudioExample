from cmsis_stream.cg.scheduler import Graph,Configuration,SlidingBuffer,CType,F32
from cmsis_stream.cg.scheduler.graphviz import Style

from .nodes import *
from .appnodes import *

from .generate import generate

def generate_debug(codeSizeOptimization=False,name="a" ):
    the_graph = Graph()
    NB = 320
    src = src = VStreamAudioSource("audioSource",NB)
    sink = NullSink("sink",Q15_STEREO,NB)
    
    the_graph.connect(src.o,sink.i)
   
   
    
    class MyStyle(Style):
        
        def edge_color(self,edge):
            nb = self.fifoLength(edge) 
            if nb is None:
                nb = 0
            s = self.edgeSrcNode(edge)
            d = self.edgeDstNode(edge)
            
            if d.nodeName ==  "display":
               return("magenta")
            else: 
                if (nb > 512):
                    return("orange")
                return(super().edge_color(edge))
    
    myStyle = MyStyle()
    
    generate(f"app{name}",the_graph,myStyle,codeSizeOptimization=codeSizeOptimization)

def generate_debug_a(codeSizeOptimization=False):
    generate_debug(codeSizeOptimization=codeSizeOptimization,name="a")

def generate_debug_b(codeSizeOptimization=False):
    generate_debug(codeSizeOptimization=codeSizeOptimization,name="b")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
                    prog='debug',
                    description='Regenerate debug demo')
    parser.add_argument("--size", help="Code size optimization enabled", action='store_true')
    parser.add_argument("--name", help="Name of the debug demo", choices=["a", "b"], default="a")

    args = parser.parse_args()

    if args.name == "a":
        generate_debug_a(codeSizeOptimization=args.size)
    else:
        generate_debug_b(codeSizeOptimization=args.size)

    if args.size:
        print("Debug demo generated with code size optimization")
        print("You need to call the generate script to regenerate the common files")
        print("shared between all applications.")

