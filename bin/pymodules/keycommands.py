"""a module that executes test commands bound to keys,
to help py api dev and testing"""

import rexviewer as r
from circuits import Component

class KeyCommander(Component):
    def __init__(self):
        Component.__init__(self)
        self.inputmap = {
            r.MoveForwardPressed: self.run_commandpy #change to C key
        }
    
    def on_input(self, evid):
        #print "Commander got input", evid
        if evid in self.inputmap:
            self.inputmap[evid]()
            
    def run_commandpy(self):
        #print "Command:"
        import command
        command = reload(command)
        
    