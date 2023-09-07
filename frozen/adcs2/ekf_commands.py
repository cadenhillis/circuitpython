import time


try:
    import lib.adcs.data_types as tp
    import lib.adcs.cryo_cube_CT_translated as ct
    import ulab.numpy as np
    from lib.adcs.igrf_vars import Gnm, Hnm, dGnmdt, dHnmdt
    import lib.adcs.time_converts as tc
    import lib.adcs.cryo_cube_rk_ode as ccODE
#from tabulate import tabulate

except:

    import data_types as tp
    import cryo_cube_CT_translated as ct
    import ulab.numpy as np
    from igrf_vars import Gnm, Hnm, dGnmdt, dHnmdt
    import time_converts as tc
    import cryo_cube_rk_ode as ccODE

# constants at line 51 
class ekf_commands():
    gps_r = np.array([0,0,0])
    gps_v = np.array([0,0,0])
    ## Constants have explanations below it
    position_eq = np.array([0.0,0.0,0.0])
    velocity_eq = np.array([0.0,0.0,0.0])
    Tref = np.array((3,3))
    Test    = np.zeros((3,3))
    Pde     = np.zeros((6,6))
    T       = np.zeros((3,3))
    ww      = np.zeros(3) # ang vel wrt inertial frame in body fixed
    wxIw    = np.zeros(3)
    Iwx     = np.zeros(3)
    wx      = np.zeros((3,3))
    wxI     = np.zeros((3,3))
    Ii      = np.zeros((3,3))
    LADtmp  = np.zeros((3,3))
    LAD     = np.zeros((3,3))
    F       = np.zeros((6,6))
    FP      = np.zeros((6,6))
    Ft      = np.zeros((6,6))
    PFt     = np.zeros((6,6))
    dP      = np.zeros((6,6))
    Om      = np.zeros((4,4))
    rde     = 0
    r_hat   = np.zeros(3)
    Ir_hat  = np.zeros(3)
    r_hat_cross_Ir_hat = np.zeros(3)
    TC      = np.zeros(3)
    TG      = np.zeros(3)
    TD      = np.zeros(3)
    TT      = np.zeros(3)
    wbrb    = np.zeros((3,3))
    wrI0    = np.zeros((3,3))
    Av_norm = 0
    LGG     = np.zeros((3,3))
    LAE     = np.zeros((3,3))


    position = np.zeros(3)
    velocity = np.zeros(3)
    GMST = 0
    Tef = np.zeros((3,3))
    position_ef = np.zeros(3)
    velocity_ef = np.zeros(3)
    RR = np.zeros((3,3))
    Tuse = np.zeros((3,3))
    Tef2in = np.zeros((3,3))
    Tuse2ef = np.zeros((3,3))
    r = 0
    latlonr = np.zeros(3)
    B = np.zeros(3)
    Bbf = np.zeros(3)
    Tin2ref = np.zeros((3,3))
    Tref2bf = np.zeros((3,3))
    Hekf = np.zeros((12,6))
    x = np.zeros(43)
    # 7 components (first 3 are ang vel)
    # 6x6 represents covariance matrix

    meas_err = np.zeros(12)
    Tbf2bff = np.zeros((3,3))
    Pekf = np.zeros((6,6))
    K = np.zeros((6,12))
    K1 = np.zeros((6,12))
    S1 = np.zeros((12,6))
    S = np.zeros((12,12))
    R = np.zeros(12)
    Sinv = np.zeros((12,12))
    Ht = np.zeros((6,12))
    dxdt = np.zeros(43)

    EKF_EPSILON = 0.001

    EARTH_MU = 3.986e14 #m**3 / s**2




    def __init__(self, data: tp.ekf_data_t, satellite=None):
        # input ekf_data_t object
        # storing I matrix in ekf_commands object
        self.I = data.I
        self.cubesat = satellite

    def lla_to_ecef(self, lat, long, alt):
        ''' 
        Converts lat long altitude to ecef
        lat: lattitude in degrees
        long: longitude in degrees
        alt: altitude in m
        returns np array position in meters in ecef frame
        '''
        rad = np.float64(6378137.0)        # Radius of the Earth (in meters)
        f = np.float64(1.0/298.257223563)  # Flattening factor WGS84 Model
        cosLat = np.cos(np.radians(lat))
        sinLat = np.sin(np.radians(lat))
        FF     = (1.0-f)**2
        C      = 1/np.sqrt(cosLat**2 + FF * sinLat**2)
        S      = C * FF

        x = (rad * C + alt)*cosLat * np.cos(np.radians(long))
        y = (rad * C + alt)*cosLat * np.sin(np.radians(long))
        z = (rad * S + alt)*sinLat
        return np.array([x, y, z])

    def getGpsVelocity(self, pos, time):
        ''' 
        Calculate gps velocity assuming linear
        pos: ecef position vector in meters at time
        time: timestamp of measurement
        returns velocity from last position measurement to current position measurement
        '''
        if self.gps_r != np.zeros(3) and self.gps_t is not None:
            return (pos - self.gps_r) / (time - self.gps_t)

    def getGPS(self):
        '''
        Get gps measurements if they are available and convert to ecef in m
        Returns True if successful
        Returns None otherwise
        '''
        if True: #not self.cubesat.hardware['GPS']:
            print('No GPS connected')
            return None
        self.cubesat.gps.update()

        if not self.cubesat.gps.has_fix:
            print('waiting for GPS fix')
            return None
        
        speed = self.cubesat.gps.speed_knots
        lat = self.cubesat.gps.latitude
        lon = self.cubesat.gps.longitude
        alt = self.cubesat.gps.altitude_m
        pos = self.lla_to_ecef(lat, lon, alt) #m ecef
        time = self.cubesat.gps.timestamp_utc
        self.gps_v = self.getGpsVelocity(pos, time)
        self.gps_r = pos
        self.gps_t = time
        if self.gps_v is not None:
            return True
        return None

        
    def rk4(self, f, t, state, step):
        '''
        Compute one rk4 step
        f: fuction pointer to differential equation
        t: current time
        state: numpy array concatenation of position and velocity
        step: stepsize of the solver
        returns: new state
        '''
        k1 = f( t, state )
        k2 = f( t + 0.5 * step, state + 0.5 * k1 * step )
        k3 = f( t + 0.5 * step, state + 0.5 * k2 * step )
        k4 = f( t +       step, state +       k3 * step )

        return state + step / 6.0 * ( k1 + 2 * k2 + 2 * k3 + k4 ) 


    def twoBody(self, t, state):
        ''' 
        The circular two body ode
        t : time
        state: np array concatenation of position and velocity
        returns np array concatenation of velocity and acceleration
        '''
        r = state[:3]
        a = -self.EARTH_MU * r / (np.linalg.norm(r) **3)
        return np.array([state[3], state[4], state[5], a[0], a[1], a[2]])

        
    def diffEqInit(self, t, x, data: tp.ekf_data_t):
        # timer command

        self.ww = x[:3]
        x[3:7] = (1/np.linalg.norm(x[3:7]))*x[3:7]
        
        for i in range(0, 6):
            for j in range(0, 6):
                self.Pde[j][i] = x[7+i*6+j]

        # GPS collect position and velocity
        # TODO: add gps access to here and add *1000 per need
        
        if self.getGPS():
            self.position_eq = self.gps_r
            self.velocity_eq = self.gps_v
        else:
            #orbit propogator
            #TODO should timestep be inverse of adcs freq?
            state = self.rk4(self.twoBody, time.monotonic(), np.concatenate((self.position_eq, self.velocity_eq)), 1)
            self.position_eq = state[:3]
            self.velocity_eq = state[3:]

        # Calculate Reference Frame Transformation (Tref) and angular velocity (wref)
        self.Tref, self.wref = ct.T_dart(self.position_eq,self.velocity_eq)

        # Coordinate Transformation from Reference frame to current best estimate s/c body-fixed coordinates
        self.Test = self.quaternion2CTmatrix(data.q)

        # creating transformation matrix to body fixed using current quaternion
        self.T = self.quaternion2CTmatrix(x[3:7])

        # angular velocity in estimated coordinate system
        self.wrI0 = np.dot(self.Test,self.wref)
        self.wbrb = np.dot(self.T,self.wrI0)

        # calculate ang vel of body wrt ref frame in body fixed coordinate system
        self.wbrb = self.ww - self.wbrb
        print()
        print("(diffEQInit) - angular velocity: ")
        print(self.wbrb)

    def diffEqTorques(self, data: tp.ekf_data_t):
        # represents control torque
        self.TC = np.cross(data.M, data.mag_meas_1.data[data.mag_meas_1.iterator])

        # express position in best estimate s/c coordinates
        pos = np.dot(self.Tref,self.position_eq)
        self.position_eq = np.dot(self.Test,pos)
        
        self.rde = np.linalg.norm(self.position_eq)
        self.r_hat = (1/self.rde)*self.position_eq

        self.Ir_hat = np.dot(self.I,self.r_hat)
        self.r_hat_cross_Ir_hat = np.cross(self.r_hat, self.Ir_hat)
        self.TG = (3*data.muE/(self.rde*self.rde*self.rde)) * self.r_hat_cross_Ir_hat

        vel = np.dot(self.Tref,self.velocity_eq)
        self.velocity_eq = np.dot(self.Test,vel)

        Av = data.A*self.velocity_eq
        self.Av_norm = np.linalg.norm(Av)
        q = -.5*(data.rho)*(data.CD)*self.Av_norm
        FD = q*self.velocity_eq
        TD = np.cross(data.rcp, FD)
        # TODO: update model to have tg and td
        self.TT = self.TC + self.TG*0 + self.TD*0
        
        
# checked til here
    def linearCoupling(self):
        Itmp = self.I*1
        self.Iw = np.dot(self.I,self.ww)
        self.Iwx = np.array([[0, -self.Iw[2], self.Iw[1]],
                            [self.Iw[2], 0, -self.Iw[0]],
                            [-self.Iw[1], self.Iw[0], 0]])
        
        self.wx = np.array([[0, -self.ww[2], self.ww[1]],
                            [self.ww[2], 0, -self.ww[0]],
                            [-self.ww[1], self.ww[0], 0]])

        self.wxI = np.dot(self.wx,self.I)
        self.Ii = np.linalg.inv(Itmp)

        self.LADtmp = self.Iwx - self.wxI
        self.LAD = np.dot(self.Ii,self.LADtmp) * 0 
        # TODO: shouldn't just be 0

    def linearGravityGradient(self, data: tp.ekf_data_t):
        # gravity gradient torque
		# linearized = 6*mu/r^3 * ( rx*I*rx - (Ir)x*rx ) * dq;
        rx      = np.zeros((3,3))
        rx_I    = np.zeros((3,3))
        rx_I_rx = np.zeros((3,3))
        Ir      = np.zeros((3,3))
        Irx     = np.zeros((3,3))
        Irx_rx  = np.zeros((3,3))

        rx = [[0.0,                   -self.position_eq[2]/self.rde,    self.position_eq[1]/self.rde],
              [self.position_eq[2]/self.rde,     0.0,                  -self.position_eq[0]/self.rde],
              [self.position_eq[1]/self.rde,     self.position_eq[0]/self.rde,                0.0]]
        
        rx_I = np.dot(rx,self.I)
        rx_I_rx = np.dot(rx_I,rx)
        Ir = np.dot(self.I,self.position_eq)

        Irx = [[0.0, -Ir[2]/self.rde, Ir[1]/self.rde],
               [Ir[2]/self.rde, 0.0, -Ir[0]/self.rde],
               [Ir[1]/self.rde, Ir[0]/self.rde, 0.0]]

        Irx_rx = np.dot(Irx,rx)

        self.LGG = (6*(data.muE)/(self.rde*self.rde*self.rde)) * (rx_I_rx - Irx_rx) * 0 
        # TODO: shouldn't just be 0

    def linearAero(self, data: tp.ekf_data_t):
        LFD  = np.zeros((3,3))
        vx   = np.zeros((3,3))
        rcpx = np.zeros((3,3))

        vx = [[0.0, -self.velocity_eq[2], self.velocity_eq[1]],
              [self.velocity_eq[2], 0.0, -self.velocity_eq[0]],
              [-self.velocity_eq[1], self.velocity_eq[0], 0.0]]

        LFD[:,0] = np.ones(3)*((data.A[1]**2)) - (data.A[2]**2)*(self.velocity_eq[1]*self.velocity_eq[2])
        LFD[:,1] = np.ones(3)*((data.A[2]**2)) - (data.A[0]**2)*(self.velocity_eq[0]*self.velocity_eq[2])
        LFD[:,2] = np.ones(3)*((data.A[0]**2)) - (data.A[1]**2)*(self.velocity_eq[0]*self.velocity_eq[1])
        for i in range(0,3):
            for j in range(0,3):
                LFD[i,j] = -(LFD[i][j]*(self.velocity_eq[i])/self.Av_norm + (vx[i][j])*self.Av_norm) * (data.rho*data.CD)
        
        rcpx = [[ 0.0, -1*(data.rcp[2]), (data.rcp[1])],
                [(data.rcp[2]), 0.0, -1*(data.rcp[0])],
                [-(data.rcp[1]), (data.rcp[0]), 0.0]]

        self.LAE = np.dot(rcpx,LFD)  * 0 
        # TODO: shouldn't just be 0

    def diffEqLinDyn(self,data: tp.ekf_data_t):
        self.linearCoupling()
        self.linearGravityGradient(data)
        self.linearAero(data)

    def assembleLinearEquations(self, data: tp.ekf_data_t):
        wql = np.zeros(3)
        wql = data.ang_vel + self.wrI0

        for i in range(0,3):
            self.F[i+3][i] = 0.5
            for j in range(0,3):
                self.F[i,j] = self.LAD[i,j]
            for j in range(3,6):
                self.F[i,j] = self.LGG[i,j-3] + self.LAE[i,j-3]

        self.F[3][3] =  0.0         
        self.F[3][4] =  0.5*wql[2]   
        self.F[3][5] = -0.5*wql[1]
        self.F[4][3] = -0.5*wql[2]   
        self.F[4][4] =  0.0;         
        self.F[4][5] =  0.5*wql[0]
        self.F[5][3] =  0.5*wql[1]   
        self.F[5][4] = -0.5*wql[0]   
        self.F[5][5] =  0.0

        self.FP = np.dot(self.F,self.Pde)
        #self.Ft = np.transpose(self.FP) 
        self.Ft = self.FP.T
        # TODO: will need a transpose function above since there is none in ulab circuit python
        self.PFt = np.dot(self.Pde,self.Ft)
        self.dP = self.FP + self.PFt

# TODO: maybe remove argument type specifiers at top
    def assembleNonlinearEquation(self, x: float):
        self.Om[0][0] =  0
        self.Om[0][1] = -0.5 * self.wbrb[0]
        self.Om[0][2] = -0.5 * self.wbrb[1]
        self.Om[0][3] = -0.5 * self.wbrb[2]
        self.Om[1][0] =  0.5 * self.wbrb[0]
        self.Om[1][1] =  0
        self.Om[1][2] =  0.5 * self.wbrb[2]
        self.Om[1][3] = -0.5 * self.wbrb[1]
        self.Om[2][0] =  0.5 * self.wbrb[1]
        self.Om[2][1] = -0.5 * self.wbrb[2]
        self.Om[2][2] =  0
        self.Om[2][3] =  0.5 * self.wbrb[0]
        self.Om[3][0] =  0.5 * self.wbrb[2]
        self.Om[3][1] =  0.5 * self.wbrb[1]
        self.Om[3][2] = -0.5 * self.wbrb[0]
        self.Om[3][3] =  0

        dX = np.zeros(43) # 7 + 36
        dX[3:7] = np.dot(self.Om,self.x[3:7])



        self.wxIw = np.cross(self.Iw, self.ww)
        self.wxIx = self.TT + self.wxIw
        dX[0:3] = np.dot(self.Ii,self.wxIw)
        
        for i in range(0,6):
            for j in range(0,6):
                dX[7+i*6+j] = self.dP[i][j]

        return dX

        
    def SatDEest(self, t, x, data: tp.ekf_data_t):
        x = self.diffEqInit(t, x, data)

        self.diffEqTorques(data)

        self.diffEqLinDyn(data)

        self.assembleLinearEquations(data)
        
        return self.assembleNonlinearEquation(x)
        
        

    def q_mult(q0, q1):
        q_new = np.zeros(4)
        q_new1 = np.zeros(3)
        q_new2 = np.zeros(3)

        q2 = np.zeros(4)

        q_new[0] = q0[0]*q1[0] - np.dot(q0[1:], q1[1:])
        q_new1 = q0[0] * q1[1:]
        q_new2 = q1[0] * q0[1:]
        q_new[1:] = np.cross(q0[1:], q1[1:])

        q2[0] = q_new[0]
        q2[1:] = q2[1:] + q_new1 + q_new2

        return q2

    # constants from cryoCubeEKF.c line 410 and on

    def ekf_linearization(self, data: tp.ekf_data_t):

        # TODO: figure out GPS
        self.position = self.gps_r
        self.velocity = self.gps_v

        self.Tin2ref, ang_vel_ref = ct.T_dart(self.position, self.velocity)
        self.Tref2bf = self.quaternion2CTmatrix(data.q)

        GMST = self.gstime(data.t0/86400 + data.jdsatepoch)

        self.Tef = self.Tin2ef(GMST)
        temp = self.RefVef(self.Tef, self.position, self.velocity)
        self.position_ef = temp[0]
        self.velocity_ef = temp[1]
        #self.Tuse2ef = np.transpose(self.Tuse)
        self.Tuse2ef = self.Tuse.T

        #self.Tef2in = np.transpose(self.Tef)
        self.Tef2in = self.Tef.T
        r = np.linalg.norm(self.position)

        try: self.latlonr[0] = np.acos(self.position_ef[2]/r)
        except: self.latlonr[0] = np.arccos(self.position_ef[2]/r)
        self.latlonr[1] = np.arctan2(self.position_ef[1], self.position_ef[0])
        self.latlonr[2] = r * 1000

        self.BBf = self.mag2(self.latlonr, data.t0/(24*3600) + data.jdsatepoch, 13)

        self.B = np.dot(self.Tuse2ef,self.Bbf)
        self.Bbf = np.dot(self.Tef2in,self.B)
        self.B = np.dot(self.Tin2ref,self.Bbf)
        self.Bbf = np.dot(self.Tref2bf,self.B)

        self.Hekf[0][0] = 1.0
        self.Hekf[1][1] = 1.0
        self.Hekf[2][2] = 1.0
        self.Hekf[3][0] = 1.0
        self.Hekf[4][1] = 1.0
        self.Hekf[5][2] = 1.0
            
        self.Hekf[6][4] = -self.Bbf[2]/1000000000.
        self.Hekf[6][5] =  self.Bbf[1]/1000000000.
        self.Hekf[7][3] =  self.Bbf[2]/1000000000.
            
        self.Hekf[7][5] =  self.Bbf[0]/1000000000.
        self.Hekf[8][3] =  self.Bbf[1]/1000000000.
        self.Hekf[8][4] =  self.Bbf[0]/1000000000.
            
        self.Hekf[9][4] =  self.Bbf[2]/1000000000.
        self.Hekf[9][5] =  self.Bbf[1]/1000000000.
        self.Hekf[10][3] = self.Bbf[2]/1000000000.
            
        self.Hekf[10][5] = -self.Bbf[0]/1000000000.
        self.Hekf[11][3] = -self.Bbf[1]/1000000000.
        self.Hekf[11][4] =  self.Bbf[0]/1000000000.

        

    def ekf_nonlinear_propogation(self, data: tp.ekf_data_t):
        self.x[0:3] = data.ang_vel
        self.x[3:7] = data.q
        for i in range(0,6): 
            for j in range(0,6):
                self.x[7+(i*6)+j] = data.P[i][j]

        t = data.t0

        self.dxdt = self.SatDEest(t, self.x, data)

        print("ode45 for ekf")

        self.x = ccODE.ode45(43, self.x, 0, data.t1 - data.t0, 1e-3, data.t1 - data.t0, 1e-6, self.SatDEest, data)
        print("data.t0 is ", data.t0)
        print("data.t1 is ", data.t1)

        self.meas_err[0:3] = data.ang_vel_meas1 - self.x[0:3]
        self.meas_err[3:6] = data.ang_vel_meas2 - self.x[0:3]

        qnorm = np.linalg.norm(self.x[3:7])
        self.x[3:7] = (1/qnorm)*self.x[3:7]
        self.Tbf2bff = ct.quaternion2CTmatrix(self.x[3:7])
        self.GMST = self.gstime(data.jdsatepoch + (data.t1/86400))
        
        self.Tef = self.Tin2ef(self.GMST)

        temp = self.RefVef(self.Tef, self.position, self.velocity)
        self.position_ef = temp[0]
        self.velocity_ef = temp[1]

        self.Tuse = self.Tef2use(self.position_ef)
        #self.Tuse2ef = np.transpose(self.Tuse)
        self.Tuse2ef = self.Tuse.T
        #self.Tef2in = np.transpose(self.Tef)
        self.Tef2in = self.Tef.T
        self.r = np.linalg.norm(self.position)

        if (self.r < self.EKF_EPSILON):
            self.r = 1

        try: self.latlonr[0] = np.acos(self.position_ef[2]/self.r)
        except: self.latlonr[0] = np.arccos(self.position_ef[2]/self.r)
        self.latlonr[1] = np.arctan2(self.position_ef[1],self.position_ef[0])
        self.latlonr[2] = self.r*1000

        self.B = self.mag2(self.latlonr, (data.t0/86400) + data.jdsatepoch, 13)

        self.Bbf = np.dot(self.Tuse2ef,self.B)
        self.B = np.dot(self.Bbf,self.Tef2in)
        self.Bbf = np.dot(self.Tin2ref,self.B)
        self.B = np.dot(self.Tref2bf,self.Bbf)
        self.Bbf = np.dot(self.Tbf2bff,self.B)

        self.meas_err[6:9] = data.mag_meas_1.data[data.mag_meas_1.iterator] - (self.Bbf/1000000000)
        self.meas_err[9:12] = data.mag_meas_2.data[data.mag_meas_2.iterator] - (self.Bbf/1000000000)

        for i in range(0,6):
            for j in range(0,6):
                self.Pekf[i][j] = self.x[7+(6*i)+j]



    

    def ekf_posterior_estimation(self, data: tp.ekf_data_t):
        #self.Ht = np.transpose(self.Hekf)
        self.Ht = self.Hekf.T

        self.S1 = np.dot(self.Hekf,self.Pekf)

        self.S = np.dot(self.S1, self.Ht)
        self.R = data.R
        self.S = self.S + self.R

        self.Sinv = np.linalg.inv(self.S)
        self.K1 = np.dot(self.Ht,self.Sinv)


        self.K = np.dot(self.Pekf,self.K1)

        dx = np.dot(self.K,self.meas_err)

        v_min = np.linalg.norm(dx[3:])

        v_max = v_min

        if (v_min > 1):
            v_min = 1
        else:
            pass

        if (v_max > 1):
            pass
        else:
            v_max = 1

        dq = np.array([np.sqrt(1-v_min), dx[3]/v_max, dx[4]/v_max, dx[5]/v_max])

        data.q = self.q_mult(data.q,self.x[3:7])
        
        data.q = (1/np.linalg.norm(data.q)) * data.q

        data.q = self.q_mult(data.q, dq)

        data.q = (1/np.linalg.norm(data.q)) * data.q

        data.ang_vel = self.x[0:3] + dx[0:3]

        I = np.eye(6)
        
        data.P = np.dot((I - np.dot(self.K,self.Hekf)),self.Pekf)
        return data


    def ekf(self, data: tp.ekf_data_t):
        self.ekf_linearization(data)
        self.ekf_nonlinear_propogation(data)
        data = self.ekf_posterior_estimation(data)
        print("covariance matrix:")
        #print(tabulate(self.Pekf))
        return data

    # HELPER FUNCTIONS ------------------------------

    def q_mult(self, q0, q1):
        q_new = np.zeros(4) 
        q_new1 = np.zeros(3) 
        q_new2 = np.zeros(3)

        q_new[0] = q0[0]*q1[0] - np.dot(q0[1:], q1[1:])
        q_new1 = q0[0]*q1[1:]
        q_new2 = q1[0]*q1[1:]
        q_new[1:] = np.cross(q0[1:],q1[1:])

        q2 = np.zeros(4)
        q2[0] = q_new[0]

        for i in range(0,3):
            q2[i+1] = q_new[i+1] + q_new1[i] + q_new2[i]

        return q2

    def gstime(self, t_daySinceNG12ut1):
        tut1 = (t_daySinceNG12ut1 + 7231) / 36525.0
        temp = -6.2e-6* tut1 * tut1 * tut1 + 0.093104 * tut1 * tut1 + (876600.0*3600 + 8640184.812866) * tut1 + 67310.54841
        temp = (temp * (3.1415926535897932/180) / 240) % (2*3.1415926535897932)
        
        if (temp < 0):
            temp = temp + 2*3.1415926535897932

        return temp

    def Tin2ef(self, GMST: float):
        Tef = np.zeros((3,3))

        Tef[0][0] = np.cos(GMST)
        Tef[0][1] = np.sin(GMST)
        Tef[0][2] = 0.0
        Tef[1][0] = -np.sin(GMST)
        Tef[1][1] = np.cos(GMST)
        Tef[1][2] = 0.0
        Tef[2][0] = 0.0
        Tef[2][1] = 0.0
        Tef[2][2] = 1.0

        return Tef

    def RefVef(self, Tef, R, V):
        e_rot_rate = 7.292115*(10^(-5))

        Ref = np.dot(Tef,R)
        Vef = np.dot(Tef,V)

        Vef[0] = Vef[0] - e_rot_rate*Ref[1]
        Vef[1] = Vef[1] - e_rot_rate*Ref[0]

        return [Ref, Vef]

    def Tef2use(self, Ref):
        r = np.linalg.norm(Ref)

        RR = np.zeros((3,3))

        #RR[0,:] = np.transpose(Ref)/r
        RR[0,:] = (Ref.T)/r
        RR[1][0] = (Ref[2])*(Ref[0])/(r*r*np.sqrt((Ref[0])*(Ref[0]) + (Ref[1])*(Ref[1])))
        RR[1][1] = (Ref[2]*Ref[1])/(r*r*np.sqrt((Ref[0])*(Ref[0]) + (Ref[1])*(Ref[1])))
        RR[1][2] = -np.sqrt((Ref[0])*(Ref[0]) + (Ref[1])*(Ref[1]))/(r*r)

        RR[2][0] = -(Ref[1])/((Ref[0])*(Ref[0]) + (Ref[1])*(Ref[1]))
        RR[2][1] = (Ref[0])/((Ref[0])*(Ref[0]) + (Ref[1])*(Ref[1]))
        RR[2][2] = 0.0

        Tuse = np.zeros((3,3))

        Tuse[0,:] = RR[0,:]

        r = np.linalg.norm(RR[1,:])

        Tuse[1,:] = RR[1,:]/r

        r = np.linalg.norm(RR[2,:])

        Tuse[2,:] = RR[2,:]/r

        return Tuse

    def mag2(self, R, eDate, order):
        pi = 3.14159265359
        RE = 6378136.49
        eD = eDate
        eD2 = [2019,12,31,0,0,0]
        dt = (eD - tc.ymdhms_2_jd(eD2[0], eD2[1], eD2[2], eD2[3], eD2[4], eD2[5]))/365.25
        
        
        gnm = Gnm + dGnmdt*dt
        hnm = Hnm + dHnmdt*dt
        # dt = (date.toordinal(datetime(int(eD[0]), int(eD[1]), int(eD[2]), 
        #                               int(eD[3]), int(eD[4]), int(eD[5]))) - 
        #       date.toordinal(datetime(int(eD2[0]), int(eD2[1]), int(eD2[2]), 
        #                               int(eD2[3]), int(eD2[4]), int(eD2[5]))))/365.25
        
        th = pi/2 - R[0] # latitude
        ph = R[1] # longitude
        r = R[2] # radial position
        
        Sn0 = 1
        Knm = 0
        Pnn = 1
        Pn2 = np.zeros(14)
        Pn1 = Pn2
        Pn1[0] = 1
        dPnndth = 0
        dPn2dth = np.zeros(14) 
        dPn1dth = dPn2dth
        cosph = np.cos(ph)
        sinph = np.sin(ph)
        costh = np.cos(th)
        sinth = np.sin(th)
        Br = 0
        Bth = 0
        Bph = 0
        for n in range(1,order + 1):
            Sn0 = Sn0 * (2 * n - 1) / n
            Snm = Sn0
            A=(RE/r)**(n+2)
            for m in range(0,n+1):
                
                dm1 = (m == 1)
                
                if (m > 0):
                    Snm = Snm * np.sqrt((n - m + 1)*(dm1 + 1)/(n + m))
                if (n > 1):
                    Knm = ((n - 1)**2 - m ** 2) / ((2 * n - 1) * (2 * n - 3))
                
                if (m == n):
                    dPnndth = sinth * dPnndth + costh * Pnn
                    Pnn = sinth * Pnn
                    dPnmdth = dPnndth
                    Pnm = Pnn
                else:
                    dPnmdth = costh * dPn1dth[m] - sinth * Pn1[m] - Knm * dPn2dth[m]
                    Pnm = costh * Pn1[m] - Knm * Pn2[m]
                
                dPn2dth[m] = dPn1dth[m]
                dPn1dth[m] = dPnmdth
                Pn2[m] = Pn1[m]
                Pn1[m] = Pnm
                
                if (m == 0):
                    cosmph = 1
                    sinmph = 0
                else:
                    # use np.dot(a,b)
                    a = np.array([[cosmph, -1*sinmph],[sinmph, cosmph]])
                    b = np.array([cosph, sinph])
                    #C = [cosmph -sinmph sinmph cosmph]*[cosph sinph]'
                    C = np.dot(a,b)
                    cosmph=C[0]
                    sinmph=C[1]
                g = Snm*gnm[n-1][m]
                h = Snm*hnm[n-1][m]
                
                Br=Br+A*(n+1)*(g*cosmph+h*sinmph)*Pnm
                Bth=Bth-A*(g*cosmph+h*sinmph)*dPnmdth
                Bph=Bph-A/(sinth)*m*(-1*g*sinmph+h*cosmph)*Pnm
        
        B = [Br, Bth, Bph]
        return B


    def quaternion2CTmatrix(self, quaternion):
        transform_matrix = np.zeros((3,3))
        transform_matrix[0][0] = 1 - 2*(quaternion[2]*quaternion[2] + quaternion[3]*quaternion[3])
        transform_matrix[0][1] = 2*(quaternion[1]*quaternion[2] + quaternion[0]*quaternion[3])
        transform_matrix[0][2] = 2*(quaternion[1]*quaternion[3] - quaternion[0]*quaternion[2])
        transform_matrix[1][0] = 2*(quaternion[1]*quaternion[2] - quaternion[0]*quaternion[3])
        transform_matrix[1][1] = 1 - 2*(quaternion[1]*quaternion[1] + quaternion[3]*quaternion[3])
        transform_matrix[1][2] = 2*(quaternion[2]*quaternion[3] + quaternion[0]*quaternion[1])
        transform_matrix[2][0] = 2*(quaternion[1]*quaternion[3] + quaternion[0]*quaternion[2])
        transform_matrix[2][1] = 2*(quaternion[2]*quaternion[3] - quaternion[0]*quaternion[1])
        transform_matrix[2][2] = 1 - 2*(quaternion[1]*quaternion[1] + quaternion[2]*quaternion[2])

        return transform_matrix





