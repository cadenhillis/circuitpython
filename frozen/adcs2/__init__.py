print('howdy world from frozen adcs')

from adcs_test.ADCS import ADCS
import adcs_test.data_types as tp
import adcs_test.sensors as sensors  
import adcs_test.actuators as actuators 
import adcs_test.guidance as guidance 
import adcs_test.control as control  
import adcs_test.estimator as estimator 
from adcs_test.state_machine import state_machine



import ulab.numpy as np

satellite = None

jdse = 2458752.5
se = np.array([2019, 9, 26, 0, 0, 0])

k_lyapunov = 7e-4
MOI = np.array([
    [  0.052407570,    -0.000030303398, 0.00065911266  ],
    [ -0.000030303398,  0.052558871,    0.000079690068 ],
    [  0.00065911266,   0.000079690068, 0.010230396    ]])
p_lyapunov = np.diag(np.sqrt(k_lyapunov*np.diag(MOI)))

magmeas1 = tp.ring_meas_t(
    0,
    np.zeros(4), 
    np.zeros((4,3))
    ) 

magmeas2 = tp.ring_meas_t(
    0,
    np.zeros(4), 
    np.zeros((4,3))
    )

ekf_ex = tp.ekf_data_t(
    np.zeros(3),#### float[3] # [rad/s] latest estimated angular velocity
    np.array([1.0,0.0,0.0,0.0]),#### float[4] # latest estimated attitude quaternion (wrt reference)
    np.eye(6) * 1e-6, #### float[6][6] # latest estimated state covariance matrix
    #satrec          : else,trec # orbit model WHERE IS ELSETREC DEFINED
    0.0, # [s] time of last measurement/estimate (time after TLE epoch - jdsatepoch in elsetrec)
    0.0, # [s] time of this measurement/estimate (time after TLE epoch - jdsatepoch in elsetrec)
    np.zeros(3),#### float[3] # [rad/s] angular velocity measurement from IMU 1
    np.zeros(3),#### float[3] # [rad/s] angular velocity measurement from IMU 2
    magmeas1,#### float[3] # [tesla] magnetic field measurement from IMU 1
    magmeas2,#### float[3] # [tesla] magnetic field measurement from IMU 2
    np.zeros(3),#### float[3] # [A*m^2] magnetic moment (control effort) over last time interval [t0 t1]
    np.array([
        [  0.052407570,    -0.000030303398, 0.00065911266  ],
        [ -0.000030303398,  0.052558871,    0.000079690068 ],
        [  0.00065911266,   0.000079690068, 0.010230396    ]
    ]),#### float[3][3] # [kg*m^2] spacecraft mass moment of inertia tensor
    np.eye(6) * 1e-6, 
    # Q #### float[12][12] # state noise covariance matrix
    np.eye(12) * 1e-6, # double R[12][12] # measurement noise covariance matrix
    3.986004356e14, # muE # [m^3/s^2] equal to 3.986004418e14 m^3/s^2
    3.29e-12, # rho # [kg/m^3] atmospheric density
    2.5, # coefficient of drag
    np.array([0.07893, 0.07893, 0.07904]),#### float[3] # [m^2] projected areas
    np.array([0.008979, 0.006566, 0.139]), # center of pressure
    jdse # epoch
)
adcsDataEx = tp.adcs_data_t(
    tp.status_t.OK, # status
    tp.state_t.DETUMBLEPREPARE, 
    tp.state_t.DETUMBLEPREPARE,
    False,
    0,
    0.0,

    # attitude model,
    [['line1'],['line2']], # or int?
    MOI, #### float[3][3]
    0,
    0,
    0,
    0,
    0,

    # on-reset values -- maybe move these elsewhere for more coherence?,
    # these are constants that will probably be stored on SD,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    # nom values -- mabe move elsewhere for coherence?,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    #0,

    0,
    0,
    0,
    0,
    0,
    #0,commented out for adcs reduction
    #0,commented out for adcs reduction
    #0,#commented out for adcs reduction
    #0,#commented out for adcs reduction
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    # sensor measurements,
    magmeas1,
    magmeas2,
    np.array([0.0,0.0,0.0]), # gyro1_meas
    np.array([0.0,0.0,0.0]), # gyro2_meas

    # state estimate,
    np.array([0.0,0.0,0.0]), # bdot_est #### float[3],
    ekf_ex, # actual ekf data
    ekf_ex, # ekf reset values

    # guidance (desired state calculated as part of control algorithm),
    np.array([1.0,0.0,0.0,0.0]), # q_des #### float[4],
    np.array([0.0,0.0,0.0]), #### float[3]

    # controller information,
    0,
    #0,commented out for adcs reduction
    2.0,
    0, # define this type,
    0,
    k_lyapunov, # k_lyapunov
    p_lyapunov,

    # control output,
    np.array([0.0,0.0,0.0]), #### float[3] # moment out cmd
    np.array([1.0,1.0,1.0]), #### float[3] # MAX MOMENT OUT A*(m^2)
    
    np.array([0.0,0.0,0.0]), # gps r
    np.array([0.0,0.0,0.0]),  # gps v

    -1.0 # time detumbled constant

)


sen = sensors.sensors(satellite)
act = actuators.actuators(satellite)
gui = guidance.guidance()
con = control.control()
est = estimator.estimator()

st_ma = state_machine(adcsDataEx, sen, act, gui, con)
st_ma.control.mode = st_ma.control.BDOT_CONTROL
st_ma.guidance.mode = st_ma.guidance.DART
# can also set to st_ma.control.BDOT_CONTROL
adcs = ADCS(st_ma)

