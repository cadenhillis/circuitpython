try:
    import ulab.numpy as np
    import lib.adcs.data_types as tp
except:

    import ulab.numpy as np
    import data_types as tp
class control():
    BDOT_ESTIMATE_INF = 1000000.0
    BDOT_DT_MIN = 0.5

    PRIMARY_MODERN = 0
    # mode 0 is the PRIMARY_MODERN controller
    SECONDARY_MODERN = 1
    # this is a the SECONDARY_MODERN controller
    FUZZY_CONTROLLER = 2
    # this is the PRIMARY_FUZZY controller
    BDOT_CONTROL = 3
    # bdot control algorithm
    LYAPUNOV_CONTROLLER = 4

    K_primary = np.array([
                [ 1.20662e-1, -2.38029e-3,  3.90034e-3,  6.80223e-4,  1.96738e-5, -2.01446e-5],
                [ 2.67911e-3,  1.21171e-1, -3.60745e-1, -1.59717e-5,  6.85443e-4,  9.88759e-6],
                [-1.24382e-2,  1.27247e-2,  1.27694e-1,  1.37907e-4, -6.60057e-5,  7.91466e-4]
    ])

    K_secondary = np.zeros((3,6))

    Ks = [K_primary, K_secondary]

    def __init__(self):
        self.mode = 0

    def any(self,a):
        for i in a:
            if i:
                return True

        return False
    def sign(self,a):
        b = np.array(a.shape)
        for i,v in inumerate(a):
            if v>0:
                b[i] = 1
            elif v<0:
                b[i] = -1
            else:
                b[i]=0
        return b
    def attitude_error(self, q_des : np.array, ang_vel_des : np.array, q_act : np.array, ang_vel_act : np.array):
        print("attitude error - controller")
        print("ang_vel_des:", ang_vel_des)
        print("ang_vel_act:", ang_vel_act)
        ang_vel_err = ang_vel_act - ang_vel_des
        th_err = np.zeros(3)
        const = np.array([  [q_act[0], q_act[1], q_act[2], q_act[3]],
                    [q_act[1], -q_act[0], -q_act[3], q_act[2]],
                    [q_act[2], q_act[3], -q_act[0], -q_act[1]],
                    [q_act[3], -q_act[2], q_act[1], -q_act[0]]    
        ])

        q_err = np.dot(const, q_des)

        if (q_err[0] < 0): 
            q_err = q_err * -1

        if (self.any(q_err[1:] != 0)):
            try: th = 2*np.accos(q_err[0])
            except: th = 2*np.arccos(q_err[0])
            v_norm = np.linalg.norm(q_err[1:])
            if (v_norm != 0):
                th_err[0:3] = th*q_err[1:4]/v_norm
            else:
                th_err[0:3] = np.array([0,0,0])

        return ang_vel_err, th_err
    

    def lyapunov_control(self, data, ang_vel_err, th_err, ang_vel_act):
            w = ang_vel_err
            wTild = np.array([[0, -w[2], w[1]],
                        [w[2], 0, -w[0]],
                        [-w[2], w[0], 0]])
            wDotr = np.array([0,0,0]) # desired angular acceleration. should be 0 ? 
            #wDotr = np.transpose(wDotr)
            wDotr=wDotr.T 
            u = -1*data.K_lyapunov*th_err - np.dot(data.P_lyapunov,ang_vel_err)
            u = u + + np.dot(data.I, wDotr - np.dot(wTild, data.ang_vel_des))
            u = u + np.dot(wTild, np.dot(data.I, ang_vel_act))
            return u



    def modern_controller(self, data, q_des : np.array, ang_vel_des : np.array, q_act : np.array, ang_vel_act : np.array):
        print("modern_controller")
        data = self.differentiateB(data)
        if (self.mode == self.BDOT_CONTROL):
            print("switched")
            return self.bdot_calculate_control(data)
        if (self.mode == self.PRIMARY_MODERN):
            # print("using K_primary")
            gain = self.K_primary
        elif (self.mode == self.SECONDARY_MODERN):
            gain = self.K_secondary
            # print("using K_secondary")
        elif (self.mode == self.FUZZY_CONTROLLER):
            gain = self.K_primary
        elif (self.mode == self.LYAPUNOV_CONTROLLER):
            ang_vel_err, th_err = self.attitude_error(q_des, ang_vel_des, q_act, ang_vel_act)
            print("ang_vel_err")
            print(ang_vel_err)
            print("th_err")
            print(th_err)
            print("ang_vel_des")
            print(data.ang_vel_des)
            print("ang_vel_act")
            print(ang_vel_act)
            print()
            torque_des = self.lyapunov_control(data, ang_vel_err, th_err, ang_vel_act)
            return self.torque2control(data, torque_des)
            
                # wTild = [0 -w(3) w(2);
                #         w(3) 0 -w(1);
                #         -w(2) w(1) 0];
    
                # u = -k*E(1:3) - P*(w-wr) + I*(wDotr - wTild*wr) + wTild*I*w;

        else:
            print("MODE ERROR")

        ang_vel_err, th_err = self.attitude_error(q_des, ang_vel_des, q_act, ang_vel_act)
        print("ang_vel_err is : ", ang_vel_err)
        print("th_err is : ", th_err)

        ang_acc_des = np.zeros(3)
        for i in range(0,3):
            for j in range(0,3):
                ang_acc_des[i] += ( (gain[i,j])*(ang_vel_err[j]) + (gain[i,j+3])*(th_err[j]) )

        # print("moment of inertia is :", data.ekf_data.I)
        print("ang_acc_des is : ", ang_acc_des)
        torque_des = np.dot(data.ekf_data.I, ang_acc_des)
        return self.torque2control(data, torque_des)
    
    def torque2control(self, data, torque_des : np.array):
        b_hat = data.mag1_meas.data[data.mag1_meas.iterator]/np.linalg.norm(data.mag1_meas.data[data.mag1_meas.iterator])
        t_hat = torque_des/np.linalg.norm(torque_des)
        m = np.linalg.norm(torque_des)/np.linalg.norm(data.mag1_meas.data[data.mag1_meas.iterator])
        m_hat = np.cross(b_hat, t_hat)
        moment = m*m_hat
        # saturation step
        if (self.any(np.abs(moment) > data.max_M_moment_out)):
            temp_m = data.max_M_moment_out * self.sign(moment) * (np.abs(moment) > data.max_M_moment_out)
            moment = temp_m + moment*(np.abs(moment) < data.max_M_moment_out)

        data.M_moment_out = moment
        return data
    
    def differentiateB(self, data):
        it1 = data.mag1_meas.iterator
        it0 = (it1 + tp.RING_SIZE - 1) % tp.RING_SIZE
        dt = data.mag1_meas.time[it1] - data.mag1_meas.time[it0]
        if (dt < self.BDOT_DT_MIN):
            data.bdot_est = np.array([self.BDOT_ESTIMATE_INF] * 3)
            return data
        data.bdot_est = (data.mag1_meas.data[it1] - data.mag1_meas.data[it0]) / dt
        # sat.state_error(False)
        return data

    def bdot_calculate_control(self, data):
        if ((data.bdot_est[0] < self.BDOT_ESTIMATE_INF) and (data.bdot_est[1] < self.BDOT_ESTIMATE_INF)
            and (data.bdot_est[2] < self.BDOT_ESTIMATE_INF)):
            B = np.linalg.norm(data.mag1_meas.data[data.mag1_meas.iterator])
            data.M_moment_out = -data.bdot_control * data.bdot_est / B
            # B = np.linalg.norm(np.array(data.mag1_meas.data[data.mag1_meas.iterator]))
            # data.M_moment_out = list(-np.array(data.bdot_control) * np.array(data.bdot_est) / B)
        else:
            data.M_moment_out = np.zeros(3)

                # saturation step
        moment = data.M_moment_out
        if (self.any(np.abs(moment) > data.max_M_moment_out)):
            temp_m = data.max_M_moment_out * self.sign(moment) * (np.abs(moment) > data.max_M_moment_out)
            moment = temp_m + moment*(np.abs(moment) < data.max_M_moment_out)

        data.M_moment_out = moment
        
        # sat.state_error(False)   
        return data
    # NOTE: t/b^2
