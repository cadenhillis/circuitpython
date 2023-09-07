try:
    import ulab.numpy as np
except:
    import ulab.numpy as np
class guidance():

    # this guidance will be active during the dart mode.
    # if another form of guidance is needed, add a mode here and add an if statement below
    
    # modes for guidance type
    DART = 0
    # mode 0 is Tdes DART algorithm from simulation
    DETUMBLE = 1
    # mode 1 is only 0 angular velocity
 
    def __init__(self):
        self.mode = self.DART

    def q_des(self):
        if (self.mode == self.DART):
            return np.array([1,0,0,0])
    
    def T_dart(self, position, velocity):
        if (self.mode == self.DART):
            temp = ct.T_dart(position, velocity)
            print("guidance gives: ", temp[1])
            return temp

    
