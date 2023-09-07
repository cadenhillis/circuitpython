try: 
    import lib.adcs.data_types as tp
    import ulab.numpy as np
    import time
except:
    import time
    import data_types as tp
    import ulab.numpy as np
class sensors():
    def __init__(self, cubesat=None): #cubesat =None for testing purposes
        self.att = 0
        self.mag = [0,0,0]
        self.gyro = [0,0,0]
        self.time = 0
        self.cubesat= cubesat

    def read_mag1(self):
        try: return self.cubesat.imu.mag0
        except: return self.mag
    
    def read_mag2(self):
        try: return self.cubesat.imu.mag1
        except: return self.mag 

    def read_gyro1(self):
        try: return self.cubesat.imu.gyro0
        except: return self.gyro 

    def read_gyro2(self):
        try: return self.cubesat.imu.gyro1
        except: return self.gyro 
    def get_time(self):
        return time.monotonic()

    def measureB(self, sat):
        sat.data.mag1_meas.iterator += 1
        sat.data.mag1_meas.iterator %= tp.RING_SIZE
        # mag = [0] * 3
        # read mag1
        sat.data.mag1_meas.data[sat.data.mag1_meas.iterator] = np.array(sat.sensors.read_mag1())
        sat.data.mag2_meas.data[sat.data.mag2_meas.iterator] = np.array(sat.sensors.read_mag2())

        sat.data.mag1_meas.time[sat.data.mag1_meas.iterator] = self.time
        sat.data.mag2_meas.time[sat.data.mag2_meas.iterator] = self.time
        return sat.data

    def read_gyros(self, data):
        if (data.status == tp.status_t.ERROR):
            return data
    
        data.gyro1_meas = self.read_gyro1()
        data.gyro2_meas = self.read_gyro2()

        # sat.state_error(False)
        return data
        
