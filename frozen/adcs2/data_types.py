#from dataclasses import dataclass
#from enum import Enum
try: 
    import ulab.numpy as np
except:
    import ulab.numpy as np
# ADCS implementation constants
RING_SIZE = 4

ADCS_RESET_VALS_PATH = "/sd/adcs_reset"

""" figure this out with trans_criteria_t
# define bit fields for checking various mode transition criteria
CHECK_MIN_BDOT_TRANS_CRIT               = 0x0001
CHECK_MAX_BDOT_TRANS_CRIT               = 0x0002
CHECK_MIN_ANG_ERR_TRANS_CRIT            = 0x0004
CHECK_MAX_ANG_ERR_TRANS_CRIT            = 0x0008
CHECK_MIN_ANG_VEL_ERR_TRANS_CRIT        = 0x0010
CHECK_MAX_ANG_VEL_ERR_TRANS_CRIT        = 0x0020
CHECK_MIN_RW_ANG_VEL_TRANS_CRIT         = 0x0040
CHECK_MAX_RW_ANG_VEL_TRANS_CRIT         = 0x0080
CHECK_MIN_STATE_COVAR_DIAG_TRANS_CRIT   = 0x0100
CHECK_MAX_STATE_COVAR_DIAG_TRANS_CRIT   = 0x0200
"""

CHECK_MIN_BDOT_TRANS_CRIT               = True
CHECK_MAX_BDOT_TRANS_CRIT               = True
CHECK_MIN_ANG_ERR_TRANS_CRIT            = True
CHECK_MAX_ANG_ERR_TRANS_CRIT            = True
CHECK_MIN_ANG_VEL_ERR_TRANS_CRIT        = True
CHECK_MAX_ANG_VEL_ERR_TRANS_CRIT        = True
CHECK_MIN_RW_ANG_VEL_TRANS_CRIT         = True
CHECK_MAX_RW_ANG_VEL_TRANS_CRIT         = True
CHECK_MIN_STATE_COVAR_DIAG_TRANS_CRIT   = True
CHECK_MAX_STATE_COVAR_DIAG_TRANS_CRIT   = True

"""
CHECK_MIN_BDOT_TRANS_CRIT = 1
CHECK_MAX_BDOT_TRANS_CRIT = 2
CHECK_MIN_ANG_ERR_TRANS_CRIT = 4
CHECK_MAX_ANG_ERR_TRANS_CRIT = 8
CHECK_MIN_ANG_VEL_ERR_TRANS_CRIT = 16
CHECK_MAX_ANG_VEL_ERR_TRANS_CRIT = 16*2
CHECK_MIN_RW_ANG_VEL_TRANS_CRIT = 16*4
CHECK_MAX_RW_ANG_VEL_TRANS_CRIT = 16*8
CHECK_MIN_STATE_COVAR_DIAG_TRANS_CRIT = 256*1
CHECK_MAX_STATE_COVAR_DIAG_TRANS_CRIT = 256*2
"""

# enumerate ADCS states with unique names
# used for self.adcs.executeADCSaction
#class state_t(Enum):
class state_t():
    SAFE1                                   = 0
    SAFE2                                   = 1

    DETUMBLEPREPARE                         = 2
    DETUMBLEMEASURE1                        = 3
    DETUMBLEMEASURE2                        = 4
    DETUMBLEESTIMATE                        = 5
    DETUMBLETRANSITION                      = 6
    DETUMBLECALCULATECONTROL                = 7
    DETUMBLEAPPLYCONTROL                    = 8

    EKFRESTART                              = 9     # A one-state mode for EKF restart (re-initialize some EKF parameters, like P)

    EKFSTARTPREPARE                         = 10
    EKFSTARTMEASURE1                        = 11
    EKFSTARTESTIMATE1                       = 12
    EKFSTARTMEASURE2                        = 13
    EKFSTARTESTIMATE2                       = 14
    EKFSTARTCALCULATECONTROL                = 15
    EKFSTARTAPPLYCONTROL                    = 16
    EKFSTARTTRANSITION                      = 17

    DARTSTARTPREPARE                        = 18
    DARTSTARTMEASURE                        = 19
    DARTSTARTESTIMATE                       = 20
    DARTSTARTGUIDANCE                       = 21
    DARTSTARTTRANSITION                     = 22
    DARTSTARTCALCULATECONTROL               = 23
    DARTSTARTAPPLYCONTROL                   = 24

    DARTPREPARE                             = 25
    DARTMEASURE                             = 26
    DARTESTIMATE                            = 27
    DARTGUIDANCE                            = 28
    DARTTRANSITION                          = 29
    DARTCALCULATECONTROL                    = 30
    DARTAPPLYCONTROL                        = 31

    #commented out for adcs reduction
    # FUZZYSTARTPREPARE                       = 32
    # FUZZYSTARTMEASURE                       = 33
    # FUZZYSTARTESTIMATE                      = 34
    # FUZZYSTARTGUIDANCE                      = 35
    # FUZZYSTARTTRANSITION                    = 36
    # FUZZYSTARTCALCULATECONTROL              = 37
    # FUZZYSTARTAPPLYCONTROL                  = 38

    # FUZZYPREPARE                            = 39
    # FUZZYMEASURE                            = 40
    # FUZZYESTIMATE                           = 41
    # FUZZYGUIDANCE                           = 42
    # FUZZYTRANSITION                         = 43
    # FUZZYCALCULATECONTROL                   = 44
    # FUZZYAPPLYCONTROL                       = 45
    #end commented out for adcs reduction
   # numStates = 46
    numStates = 32
    # DESATURATEPREPARE                       = 46
    # DESATURATEMEASURE                       = 47
    # DESATURATETRANSITION                    = 48
    # DESATURATECALCULATECONTROL              = 49
    # DESATURATEAPPLYCONTROL                  = 50

    # DESATURATESTOPWHEELS                   = 51    # Separate one-state mode for ensuring wheels are stopped

    # DESAT4EKFRESTARTPREPARE                 = 52
    # DESAT4EKFRESTARTMEASURE                 = 53
    # DESAT4EKFRESTARTTRANSITION              = 54
    # DESAT4EKFRESTARTCALCULATECONTROL        = 55
    # DESAT4EKFRESTARTAPPLYCONTROL            = 56

    # DESAT4EKFRESTARTSTOPWHEELS              = 57    # separate one-state mode for ensuring wheels are stopped

    # numStates                               = 58    # when cast to an integer type, this should equal the total number of states

#class action_t(Enum):
class action_t():
    SAFE1 = 0
    SAFE2 = 1
    TURN_OFF_MAGNETORQUERS = 2
    MEASUREB = 3
    DIFFERENTIATEB = 4
    DO_NOTHING = 5
    BDOT_CALCULATE_CONTROL = 6
    BDOT_APPLY_CONTROL = 7
    EKF_RESTART = 8
    MEASURE_ALL = 9
    NOMINAL_ESTIMATOR = 10
    EKF_START_EST2 = 11
    NOMINAL_GUIDANCE = 12
    NOMINAL_CALCULATE_CONTROL = 13
    NOMINAL_APPLY_CONTROL = 14
    #FUZZY_GUIDANCE = 15 commented out for adcs reduction
    # DESATURATE_CALCULATE_CONTROL = 16

# enumerate ADCS state actions
# used for self.adcs.executeADCStransition
# what is a better name for this ? See cryocubefunptrrpl.c
# actually number these.....

#class transition_t(Enum):
class transition_t():
    TRANS_SAFE1 = 0
    TRANS_SAFE2 = 1
    TRANS_DETUMBLE_PREPARE = 2
    TRANS_DETUMBLE_MEASURE1 = 3
    TRANS_DETUMBLE_MEASURE2 = 4
    TRANS_DETUMBLE_ESTIMATE = 5
    TRANS_DETUMBLE_TRANSITION = 6
    TRANS_DETUMBLE_CALCULATE_CONTROL = 7
    TRANS_DETUMBLE_APPLY_CONTROL = 8
    TRANS_EKF_RESTART = 9
    TRANS_EKF_START_PREPARE = 10
    TRANS_EKF_START_MEASURE1 = 11
    TRANS_EKF_START_ESTIMATE1 = 12
    TRANS_EKF_START_MEASURE2 = 13
    TRANS_EKF_START_ESTIMATE2 = 14
    TRANS_EKF_START_CALCULATE_CONTROL = 15
    TRANS_EKF_START_APPLY_CONTROL = 16
    TRANS_EKF_START_TRANSITION = 17
    TRANS_DART_START_PREPARE = 18
    TRANS_DART_START_MEASURE = 19
    TRANS_DART_START_ESTIMATE = 20
    TRANS_DART_START_GUIDANCE = 21
    TRANS_DART_START_TRANSITION = 22
    TRANS_DART_START_CALCULATE_CONTROL = 23
    TRANS_DART_START_APPLY_CONTROL = 24
    TRANS_DART_PREPARE = 25
    TRANS_DART_MEASURE = 26
    TRANS_DART_ESTIMATE = 27
    TRANS_DART_GUIDANCE = 28
    TRANS_DART_TRANSITION = 29
    TRANS_DART_CALCULATE_CONTROL = 30
    TRANS_DART_APPLY_CONTROL = 31
    # include fuzzy stuff? yes
    #commented out for adcs reduction
    # TRANS_FUZZY_START_PREPARE= 32
    # TRANS_FUZZY_START_MEASURE= 33
    # TRANS_FUZZY_START_ESTIMATE = 34
    # TRANS_FUZZY_START_GUIDANCE = 35
    # TRANS_FUZZY_START_TRANSITION = 36
    # TRANS_FUZZY_START_CALCULATE_CONTROL= 37
    # TRANS_FUZZY_START_APPLY_CONTROL= 38
    # TRANS_FUZZY_PREPARE= 39
    # TRANS_FUZZY_MEASURE= 40
    # TRANS_FUZZY_ESTIMATE = 41
    # TRANS_FUZZY_GUIDANCE = 42
    # TRANS_FUZZY_TRANSITION = 43
    # TRANS_FUZZY_CALCULATE_CONTROL= 44
    # TRANS_FUZZY_APPLY_CONTROL= 45

# enumerate ADCS status (either ok/nominal or error)
#class status_t(Enum):
class status_t():
    OK      = 0
    ERROR   = 1

# enumerate ADCS controller options

#class controller_options_t(Enum):
class controller_options_t():
    PRIMARY_MODERN      = 0 # PD
    SECONDARY_MODERN    = 1 # PD
    #PRIMARY_FUZZY       = 2 commented out for adcs reduction

# enumerate ADCS guidance system fuzzy options

#class fuzzy_guidance_options_t(Enum):
#commented out for adcs reduction
# class fuzzy_guidance_options_t():
#     FUZZY_SV        = 0
#     FUZZY_GS        = 1
#     FUZZY_SVGS      = 2

# structure to fill array, then loop back to beginning (overwriting data)
#@dataclass
class ring_meas_t:
    def __init__(
                self,
                iterator        : int,
                time            : np.array, #### float[RING_SIZE]
                data            : np.array #### float[RING_SIZE][3]
                ):
        self.iterator = iterator
        self.time = time
        self.data = data


#@dataclass
class ekf_data_t:
    def __init__(
                self,
                ang_vel         : np.array, #### float[3] # [rad/s] latest estimated angular velocity
                q               : np.array, #### float[4] # latest estimated attitude quaternion (wrt reference)
                P               : np.array, #### float[6][6] # latest estimated state covariance matrix
                #satrec          : elsetrec # orbit model WHERE IS ELSETREC DEFINED
                t0              : float, # [s] time of last measurement/estimate (time after TLE epoch - jdsatepoch in elsetrec)
                t1              : float, # [s] time of this measurement/estimate (time after TLE epoch - jdsatepoch in elsetrec)
                ang_vel_meas1   : np.array, #### float[3] # [rad/s] angular velocity measurement from IMU 1
                ang_vel_meas2   : np.array, #### float[3] # [rad/s] angular velocity measurement from IMU 2
                mag_meas_1      : np.array, #### float[3] # [tesla] magnetic field measurement from IMU 1
                mag_meas_2      : np.array, #### float[3] # [tesla] magnetic field measurement from IMU 2
                M               : np.array, #### float[3] # [A*m^2] magnetic moment (control effort) over last time interval [t0 t1]
                I               : np.array, #### float[3][3] # [kg*m^2] spacecraft mass moment of inertia tensor
                Q               : np.array, #### float[6][6] # [kg*m^2] reaction wheel mass moment of inertia
                R               : np.array, #### float[12][12] # state noise covariance matrix
                muE             : float, # measurement noise covariance matrix
                rho             : float, # [m^3/s^2] equal to 3.986004418e14 m^3/s^2
                CD              : float, # [kg/m^3] atmospheric density
                A               : np.array, #### float[3] # [m^2] projected areas
                rcp             : np.array, #### float[3] # [m] center of pressure
                jdsatepoch      : float
                ):
        self.ang_vel = ang_vel
        self.q = q
        self.P = P
        self.t0 = t0
        self.t1 = t1
        self.ang_vel_meas1 = ang_vel_meas1
        self.ang_vel_meas2 = ang_vel_meas2
        self.mag_meas_1 = mag_meas_1
        self.mag_meas_2 = mag_meas_2
        self.M = M
        self.I = I
        self.Q = R
        self.R = R
        self.muE = muE
        self.rho = rho
        self.CD = CD
        self.A = A
        self.rcp = rcp
        self.jdsatepoch = jdsatepoch




#@dataclass
class control_state_t:
    def __init__(
            self,
            action      : int,
            transition  : int,
            delay       : int
            ):
        self.action = action
        self.transition = transition
        self.delay = delay


# define/name operational mode transitions
class mode_transition_enum():
    Detumble2EKFRestart = 0
    EKFStart2DartStart = 1
    ControlStartBack2EKFStart = 2
    ControlStart2Control = 3
    ControlBack2ControlStart = 4

#@ dataclass
class trans_criteria_t:
    def __init__(
            self,
            things2check        : int, # TODO figuee out what this is for and if it can be implmented easier to understand in python # bit fields for 'this' mode's transition criteria
            min_bdot            : float, # [tesla/s] minimum magnitude of Bdot required for mode transition
            max_bdot            : float, # [tesla/s] maximum magnitude of Bdot required for mode transition
            min_ang_err         : float, # [rad] minimum angular error (axis-angle representation) for mode transition
            max_ang_err         : float, # [rad] maximum angular error (axis-angle representation) for mode transition 
            min_ang_vel_err     : np.array, #### float[3] # [rad/s] minimum angular rate error required for mode transition
            max_ang_vel_err     : np.array, #### float[3] # [rad/s] maximum angular rate error required for mode transition
            min_P_diag          : np.array, #### float[6] # [rad^2/s^2] and [] minimum state error variance (diagonal of covariance matrix)
            max_P_diag          : np.array, #### float[6] # [rad^2/s^2] and [] maximum state error variance (diagonal of covariance matrix)
            _or                 : bool # evaluate constraints using or (default and - i.e. all constraints must be satisfied)
            ):
        self.things2check = things2check
        self.min_bdot = min_bdot
        self.max_bdot = max_bdot
        self.min_ang_err = min_ang_err
        self.max_ang_err = max_ang_err
        self.min_P_diag = min_P_diag
        self.max_P_diag = max_P_diag
        self._or = _or
# find this def
#@ dataclass
class modern_controller_t:
    pass

############################### ADCS DATA CONTAINER ##########################################

"""
container for storing all adcs data. 
TODO: correctly declare all type annotations inside this class
TODO: determine if additional members need to be added.
TODO: determine if "reset" or "nom" values can be saved on an SD as opposed to being stored with every instance of this type....
    TODO: a dict with msgpack would be an efficient way to store these initial values.....
"""
#@dataclass
class adcs_data_t:
    def __init__(
            self,
            status                      : status_t,
            state                       : state_t,
            commanded_state             : state_t,
            go_to_commanded_state_f     : bool,
            state_attempt               : int,
            next_time                   : float,
    
            # attitude model
            TLE                         : list, # or int?
            I                           : np.array, #### float[3][3]
            igrf_g                      : float, 
            igrf_dg                     : float,
            igrf_h                      : float,
            igrf_dh                     : float,
            igrf_epoch                  : float,


            # on-reset values -- maybe move these elsewhere for more coherence?
            # these are constants that will probably be stored on SD
            reset_ang_vel_est           : float,
            reset_q_est                 : float,
            reset_covar_est             : float,
            reset_t0                    : float,
            reset_t1                    : float,
            reset_ang_vel_meas1         : float,
            reset_ang_vel_meas2         : float,
            reset_mag_meas1             : float,
            reset_mag_meas2             : float,
            reset_m                     : float,

            reset_nom_start             : float,
            reset_irw                   : float,
            reset_Q                     : float,
            reset_r1                    : float,
            reset_r2                    : float,
            reset_muE                   : float,
            reset_rho                   : float,
            reset_c                     : float,
            reset_A                     : float,
            reset_rcp                   : float,

            # nom values -- mabe move elsewhere for coherence?
            nom_to_reset                : float,
            nom_start                   : float,
            nom_irw                     : float,
            nom_Q                       : float,
            nom_r2                      : float,
            nom_muE                     : float,
            nom_rho                     : float,
            nom_A                       : float,
            nom_rcp                     : float,

            orbit_params                : float,
            bdot_control_const          : float,
            k_primary                   : float,
            k_secondary                 : float,
            max_m_moment_cmd            : float,
            #commented out for adcs reduction
            # fuzzy_sv_string             : float,
            # fuzzy_gs_string             : float,
            # fuzzy_sv_gs_string          : float,
            # fuzzy_ac_string             : float,
            #end commented out for adcs reduction
            state_delays                : float,
            trans_crit0                 : float,
            trans_crit1                 : float,
            trans_crit2                 : float,
            trans_crit3                 : float,
            trans_crit4                 : float,
            trans_crit5                 : float,
            trans_crit6                 : float,
            trans_crit7                 : float,
            primary_imu_select          : float,

            # sensor measurements
            mag1_meas                   : ring_meas_t,
            mag2_meas                   : ring_meas_t,
            gyro1_meas                  : np.array, #### float[3]
            gyro2_meas                  : np.array, #### float[3]

            # state estimate
            bdot_est                    : np.array, #### float[3]
            ekf_data                    : ekf_data_t,
            ekf_data_rst_values         : ekf_data_t,

            # guidance (desired state calculated as part of control algorithm)
            q_des                       : np.array, #### float[4]
            ang_vel_des                : np.array, #### float[4]
    
            # controller information
            controller_options          : controller_options_t,
            #fuzzy_guidance_option       : fuzzy_guidance_options_t, commented out for adcs reduction
            bdot_control                : float,
            K_primary                   : modern_controller_t, # define this type
            K_secondary                 : modern_controller_t,
            K_lyapunov                  : float, 
            P_lyapunov                  : np.array, 
            # control output
            M_moment_out                : np.array, #### float[3]
            max_M_moment_out            : np.array, #### float[3]

            gps_r : np.array,
            gps_v : np.array,

            time_detumbled              : float
            ):
                self.status = status
                self.state = state
                self.commanded_state = commanded_state
                self.go_to_commanded_state_f = go_to_commanded_state_f
                self.state_attempt = state_attempt
                self.next_time = next_time
                self.TLE = TLE
                self.I = I
                self.igrf_g = igrf_g
                self.igrf_dg = igrf_dg
                self.igrf_h = igrf_h
                self.igrf_dh = igrf_dh
                self.igrf_epoch = igrf_epoch

                self.reset_ang_vel_est = reset_ang_vel_est
                self.reset_q_est = reset_q_est
                self.reset_covar_est = reset_covar_est
                self.reset_t0 = reset_t0
                self.reset_t1 = reset_t1
                self.reset_ang_vel_meas1 = reset_ang_vel_meas1
                self.reset_ang_vel_meas2 =reset_ang_vel_meas2
                self.reset_mag_meas1 = reset_mag_meas1
                self.reset_mag_meas2 = reset_mag_meas2
                self.reset_m = reset_m


                self.reset_nom_start = reset_nom_start
                self.reset_irw = reset_irw
                self.reset_Q = reset_Q
                self.reset_r1 = reset_r1
                self.reset_r2 = reset_r2
                self.reset_muE = reset_muE
                self.reset_rho = reset_rho
                self.reset_c = reset_c
                self.reset_A = reset_A
                self.reset_rcp = reset_rcp

                self.nom_to_reset = nom_to_reset
                self.nom_start = nom_start
                self.nom_irw = nom_irw
                self.nom_Q = nom_Q
                self.nom_r2 = nom_r2
                self.nom_muE = nom_muE
                self.nom_rho = nom_rho
                self.nom_A = nom_A
                self.nom_rcp = nom_rcp
                
                self.orbit_params = orbit_params
                self.bdot_control_const = bdot_control_const
                self.k_primary = k_primary
                self.k_secondary = k_secondary
                self.max_m_moment_cmd = max_m_moment_cmd
                #commented out for adcs reduction
                # self.fuzzy_sv_string = fuzzy_sv_string
                # self.fuzzy_gs_string = fuzzy_gs_string
                # self.fuzzy_sv_gs_string = fuzzy_sv_gs_string
                # self.fuzzy_ac_string = fuzzy_ac_string
                #end commented out for adcs reduction
                self.state_delays = state_delays
                self.trans_crit0 = trans_crit0
                self.trans_crit1 = trans_crit1
                self.trans_crit2 = trans_crit2
                self.trans_crit3 = trans_crit3
                self.trans_crit4 = trans_crit4
                self.trans_crit5 = trans_crit5
                self.trans_crit6 = trans_crit6
                self.trans_crit7 = trans_crit7
                self.primary_imu_select = primary_imu_select

                self.mag1_meas = mag1_meas
                self.mag2_meas = mag2_meas
                self.gyro1_meas = gyro1_meas
                self.gyro2_meas = gyro2_meas

                self.bdot_est = bdot_est
                self.ekf_data = ekf_data
                self.ekf_data_rst_values = ekf_data_rst_values

                self.q_des = q_des
                self.ang_vel_des = ang_vel_des

                self.controller_options = controller_options
                #self.fuzzy_guidance_option = fuzzy_guidance_option commented out for adcs reduction
                self.bdot_control = bdot_control
                self.K_primary = K_primary
                self.K_secondary = K_secondary
                self.K_lyapunov = K_lyapunov 
                self.P_lyapunov = P_lyapunov 

                self.M_moment_out = M_moment_out
                self.max_M_moment_out = max_M_moment_out

                self.gps_r = gps_r
                self.gps_v = gps_v
                self.time_detumbled = time_detumbled




        # and many more.....

############################## STATE ACTION & TRANSITION DEFINITIONS ##############################
# TODO: change to dictionary
class Safe1(control_state_t):
    action          = action_t.SAFE1
    transition      = transition_t.TRANS_SAFE1
    delay           = 10     
                                                             #  0
class Safe2(control_state_t):
    action          = action_t.SAFE2
    transition      = transition_t.TRANS_SAFE2
    delay           = 10                                                                   #  1

class DetumblePrepare(control_state_t):
    action          = action_t.TURN_OFF_MAGNETORQUERS
    transition      = transition_t.TRANS_DETUMBLE_PREPARE
    delay           = 0   
                              #  2
class DetumbleMeasure1(control_state_t):
    action          = action_t.MEASUREB
    transition      = transition_t.TRANS_DETUMBLE_MEASURE1
    delay           = 10          
                                 #  3
class DetumbleMeasure2(control_state_t):
    action          = action_t.MEASUREB
    transition      = transition_t.TRANS_DETUMBLE_MEASURE2
    delay           = 0    
                                        #  4
class DetumbleEstimate(control_state_t):
    action          = action_t.DIFFERENTIATEB
    transition      = transition_t.TRANS_DETUMBLE_ESTIMATE
    delay           = 0   
                                    #  5
class DetumbleTransition(control_state_t):
    action          = action_t.DO_NOTHING
    transition      = transition_t.TRANS_DETUMBLE_TRANSITION
    delay           = 0
                                       #  6
class DetumbleCalculateControl(control_state_t):
    action          = action_t.BDOT_CALCULATE_CONTROL
    transition      = transition_t.TRANS_DETUMBLE_CALCULATE_CONTROL
    delay           = 0
             #  7
class DetumbleApplyControl(control_state_t):
    action          = action_t.BDOT_APPLY_CONTROL
    transition      = transition_t.TRANS_DETUMBLE_APPLY_CONTROL
    delay           = 10    
                    #  8
class EKFRestart(control_state_t):
    action          = action_t.EKF_RESTART
    transition      = transition_t.TRANS_EKF_RESTART
    delay           = 0                                                   #  9

class EKFStartPrepare(control_state_t):
    action          = action_t.TURN_OFF_MAGNETORQUERS
    transition      = transition_t.TRANS_EKF_START_PREPARE
    delay           = 0    
                            #  10
class EKFStartMeasure1(control_state_t):
    action          = action_t.MEASURE_ALL
    transition      = transition_t.TRANS_EKF_START_MEASURE1
    delay           = 0   
                                      #  11
class EKFStartEstimate1(control_state_t):
    action          = action_t.NOMINAL_ESTIMATOR
    transition      = transition_t.TRANS_EKF_START_ESTIMATE1
    delay           = 10      
                          #  12
class EKFStartMeasure2(control_state_t):
    action          = action_t.MEASURE_ALL
    transition      = transition_t.TRANS_EKF_START_MEASURE2
    delay           = 0      
                                  #  13
class EKFStartEstimate2(control_state_t):
    action          = action_t.EKF_START_EST2
    transition      = transition_t.TRANS_EKF_START_ESTIMATE2
    delay           = 0     
                              #  14
class EKFStartCalculateControl(control_state_t):
    action          = action_t.BDOT_CALCULATE_CONTROL
    transition      = transition_t.TRANS_EKF_START_CALCULATE_CONTROL
    delay           = 0
             
class EKFStartApplyControl(control_state_t):
    action          = action_t.BDOT_APPLY_CONTROL
    transition      = transition_t.TRANS_EKF_START_APPLY_CONTROL
    delay           = 10   
         #  16
class EKFStartTransition(control_state_t):
    action          = action_t.DO_NOTHING
    transition      = transition_t.TRANS_EKF_START_TRANSITION
    delay           = 0                                   #  17

class DartStartPrepare(control_state_t):
    action          = action_t.TURN_OFF_MAGNETORQUERS
    transition      = transition_t.TRANS_DART_START_PREPARE
    delay           = 0                           #  18

class DartStartMeasure(control_state_t):
    action          = action_t.MEASURE_ALL
    transition      = transition_t.TRANS_DART_START_MEASURE
    delay           = 0                                      #  19

class DartStartEstimate(control_state_t):
    action          = action_t.NOMINAL_ESTIMATOR
    transition      = transition_t.TRANS_DART_START_ESTIMATE
    delay           = 0                              #  20

class DartStartGuidance(control_state_t):
    action          = action_t.NOMINAL_GUIDANCE
    transition      = transition_t.TRANS_DART_START_GUIDANCE
    delay           = 0                               #  21

class DartStartTransition(control_state_t):
    action          = action_t.DO_NOTHING
    transition      = transition_t.TRANS_DART_START_TRANSITION
    delay           = 0                                 #  22

class DartStartCalculateControl(control_state_t):
    action          = action_t.NOMINAL_CALCULATE_CONTROL
    transition      = transition_t.TRANS_DART_START_CALCULATE_CONTROL
    delay           = 0     #  23

class DartStartApplyControl(control_state_t):
    action          = action_t.NOMINAL_APPLY_CONTROL
    transition      = transition_t.TRANS_DART_START_APPLY_CONTROL
    delay           = 10                #  24

class DartPrepare(control_state_t):
    action          = action_t.TURN_OFF_MAGNETORQUERS
    transition      = transition_t.TRANS_DART_PREPARE
    delay           = 0                                      #  25

class DartMeasure(control_state_t):
    action          = action_t.MEASURE_ALL
    transition      = transition_t.TRANS_DART_MEASURE
    delay           = 0                                                 #  26

class DartEstimate(control_state_t):
    action          = action_t.NOMINAL_ESTIMATOR
    transition      = transition_t.TRANS_DART_ESTIMATE
    delay           = 0                                         #  27

class DartGuidance(control_state_t):
    action          = action_t.NOMINAL_GUIDANCE
    transition      = transition_t.TRANS_DART_GUIDANCE
    delay           = 0                                          #  28

class DartTransition(control_state_t):
    action          = action_t.DO_NOTHING
    transition      = transition_t.TRANS_DART_TRANSITION
    delay           = 0                                            #  29

class DartCalculateControl(control_state_t):
    action          = action_t.NOMINAL_CALCULATE_CONTROL
    transition      = transition_t.TRANS_DART_CALCULATE_CONTROL
    delay           = 0                #  30

class DartApplyControl(control_state_t):
    action          = action_t.NOMINAL_APPLY_CONTROL
    transition      = transition_t.TRANS_DART_APPLY_CONTROL
    delay           = 10                           #  31

#commented out for adcs reduction
# class FuzzyStartPrepare(control_state_t):
#     action          = action_t.TURN_OFF_MAGNETORQUERS
#     transition      = transition_t.TRANS_FUZZY_START_PREPARE
#     delay           = 0                         #  32

# class FuzzyStartMeasure(control_state_t):
#     action          = action_t.MEASURE_ALL
#     transition      = transition_t.TRANS_FUZZY_START_MEASURE
#     delay           = 0                                    #  33

# class FuzzyStartEstimate(control_state_t):
#     action          = action_t.NOMINAL_ESTIMATOR
#     transition      = transition_t.TRANS_FUZZY_START_ESTIMATE
#     delay           = 0                            #  34

# class FuzzyStartGuidance(control_state_t):
#     action          = action_t.FUZZY_GUIDANCE
#     transition      = transition_t.TRANS_FUZZY_START_GUIDANCE
#     delay           = 0                               #  35

# class FuzzyStartTransition(control_state_t):
#     action          = action_t.DO_NOTHING
#     transition      = transition_t.TRANS_FUZZY_START_TRANSITION
#     delay           = 0                               #  36

# class FuzzyStartCalculateControl(control_state_t):
#     action          = action_t.NOMINAL_CALCULATE_CONTROL
#     transition      = transition_t.TRANS_FUZZY_START_CALCULATE_CONTROL
#     delay           = 0   #  37

# class FuzzyStartApplyControl(control_state_t):
#     action          = action_t.NOMINAL_APPLY_CONTROL
#     transition      = transition_t.TRANS_FUZZY_START_APPLY_CONTROL
#     delay           = 10              #  38

# class FuzzyPrepare(control_state_t):
#     action          = action_t.TURN_OFF_MAGNETORQUERS
#     transition      = transition_t.TRANS_FUZZY_PREPARE
#     delay           = 0                                    #  39

# class FuzzyMeasure(control_state_t):
#     action          = action_t.MEASURE_ALL
#     transition      = transition_t.TRANS_FUZZY_MEASURE
#     delay           = 0                                               #  40

# class FuzzyEstimate(control_state_t):
#     action          = action_t.NOMINAL_ESTIMATOR
#     transition      = transition_t.TRANS_FUZZY_ESTIMATE
#     delay           = 0                                       #  41

# class FuzzyGuidance(control_state_t):
#     action          = action_t.FUZZY_GUIDANCE
#     transition      = transition_t.TRANS_FUZZY_GUIDANCE
#     delay           = 0                                          #  42

# class FuzzyTransition(control_state_t):
#     action          = action_t.DO_NOTHING
#     transition      = transition_t.TRANS_FUZZY_TRANSITION
#     delay           = 0                                          #  43

# class FuzzyCalculateControl(control_state_t):
#     action          = action_t.NOMINAL_CALCULATE_CONTROL
#     transition      = transition_t.TRANS_FUZZY_CALCULATE_CONTROL
#     delay           = 0              #  44

# class FuzzyApplyControl(control_state_t):
#     action          = action_t.NOMINAL_APPLY_CONTROL
#     transition      = transition_t.TRANS_FUZZY_APPLY_CONTROL
#     delay           = 10                         #  45
#end commented out for adcs reduction

ctrl_states = [
    Safe1,
    Safe2,

    DetumblePrepare,
    DetumbleMeasure1,
    DetumbleMeasure2,
    DetumbleEstimate,
    DetumbleTransition,
    DetumbleCalculateControl,
    DetumbleApplyControl,

    EKFRestart,

    EKFStartPrepare,
    EKFStartMeasure1,
    EKFStartEstimate1,
    EKFStartMeasure2,
    EKFStartEstimate2,
    EKFStartCalculateControl,
    EKFStartApplyControl,
    EKFStartTransition,

    DartStartPrepare,
    DartStartMeasure,
    DartStartEstimate,
    DartStartGuidance,
    DartStartTransition,
    DartStartCalculateControl,
    DartStartApplyControl,

    DartPrepare,
    DartMeasure,
    DartEstimate,
    DartGuidance,
    DartTransition,
    DartCalculateControl,
    DartApplyControl,
    #commented out for adcs reduction
    # FuzzyStartPrepare,
    # FuzzyStartMeasure,
    # FuzzyStartEstimate,
    # FuzzyStartGuidance,
    # FuzzyStartTransition,
    # FuzzyStartCalculateControl,
    # FuzzyStartApplyControl,

    # FuzzyPrepare,
    # FuzzyMeasure,
    # FuzzyEstimate,
    # FuzzyGuidance,
    # FuzzyTransition,
    # FuzzyCalculateControl,
    # FuzzyApplyControl,
    #endcommented out for adcs reduction
]
# class DesaturatePrepare(control_state_t):
#     action          = action_t.TURN_OFF_MAGNETORQUERS
#     transition      = transition_t.TRANS_DESATURATE_PREPARE
#     delay           = 0                          #  46
# class DesaturateMeasure(control_state_t):
#     action          = action_t.MEASUREB
#     transition      = transition_t.TRANS_DESATURATE_MEASURE
#     delay           = 0                                        #  47
# class DesaturateTransition(control_state_t):
#     action          = action_t.DO_NOTHING
#     transition      = transition_t.TRANS_DESATURATE_TRANSITION
#     delay           = 0                                #  48
# class DesaturateCalculateControl(control_state_t):
#     action          = action_t.DESATURATE_CALCULATE_CONTROL
#     transition      = transition_t.TRANS_DESATURATE_CALCULATE_CONTROL
#     delay           = 0 #  49
# class DesaturateApplyControl(control_state_t):
#     action          = action_t.NOMINAL_APPLY_CONTROL
#     transition      = transition_t.TRANS_DESATURATE_APPLY_CONTROL
#     delay           = 10               #  50

# class DesaturateStopWheels(control_state_t):
#     action          = action_t.STOP_WHEELS
#     transition      = transition_t.TRANS_DESATURATE_STOP_WHEELS
#     delay           = 0                              #  51

# class Desat4EKFRestartPrepare(control_state_t):
#     action          = action_t.TURN_OFF_MAGNETORQUERS
#     transition      = transition_t.TRANS_DESAT4EKFRE_PREPARE
#     delay           = 0                   #  52
# class Desat4EKFRestartMeasure(control_state_t):
#     action          = action_t.MEASUREB
#     transition      = transition_t.TRANS_DESAT4EKFRE_MEASURE
#     delay           = 0                                 #  53
# class Desat4EKFRestartTransition(control_state_t):
#     action          = action_t.DO_NOTHING
#     transition      = transition_t.TRANS_DESAT4EKFRE_TRANSITION
#     delay           = 0                         #  54
# class Desat4EKFRestartCalculateControl(control_state_t):
#     action          = action_t.DESATURATE_CALCULATE_CONTROL
#     delay           = transition_t.TRANS_DESAT4EKFRE_CALCULATE_CONTROL         #  55
#     delay           = 0                                                     
# class Desat4EKFRestartApplyControl(control_state_t):
#     action          = action_t.NOMINAL_APPLY_CONTROL
#     transition      = transition_t.TRANS_DESAT4EKFRE_APPLY_CONTROL
#     delay           = 10        #  56



############################## STATE TRANSITION CRITERION ##############################

class ekfStart2DartStart_stopDetumble(trans_criteria_t):
    min_bdot            = 0.0
    max_bdot            = 0.0000001
    min_ang_err         = 0.0
    max_ang_err         = 0.0
    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [0.0, 0.0, 0.0]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    _or                 = False

class ekfStart2DartStart_keepDetumble(trans_criteria_t):
    min_bdot            = 0.0
    max_bdot            = 0.0000001
    min_ang_err         = 0.0
    max_ang_err         = 0.0
    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [0.0, 0.0, 0.0]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    _or                 = False


class Detumble2EKFRestart(trans_criteria_t):
    # min_bdot            = 0.0
    # max_bdot            = 0.0000001
    # min_ang_err         = 0.0
    # max_ang_err         = 0.0
    # min_ang_vel_err     = [0.0, 0.0, 0.0]
    # max_ang_vel_err     = [0.0, 0.0, 0.0]
    # min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    # max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    # _or                 = False
    min_bdot            = 0.0
    max_bdot            = 1
    min_ang_err         = 0.0
    max_ang_err         = 100
    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [.005, 0.005, 0.005]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [1, 1, 1, 1, 1, 1]
    _or                 = False

class EKFStart2DartStart(trans_criteria_t):
    min_bdot            = 0.0
    max_bdot            = 1
    min_ang_err         = 0.0

    max_ang_err         = 100

    max_ang_err         = .02

    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [0.005, 0.005, 0.005]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [1, 1, 1, 1, 1, 1]
    _or                 = False
    # min_bdot            = 0.0
    # max_bdot            = 0.1
    # min_ang_err         = 0.0
    # max_ang_err         = 0.0
    # min_ang_vel_err     = [0.0, 0.0, 0.0]
    # max_ang_vel_err     = [0.0, 0.0, 0.0]
    # min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    # max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    # _or                 = False

class ControlStartBack2EKFStart(trans_criteria_t):
    min_bdot            = 0.0
    max_bdot            = 0.0000001
    min_ang_err         = 0.0
    max_ang_err         = 0.0
    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [0.0, 0.0, 0.0]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    _or                 = False

class ControlStart2Control(trans_criteria_t):
    min_bdot            = 0.0
    max_bdot            = 0.0000001
    min_ang_err         = 0.0
    max_ang_err         = 0.0
    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [0.0, 0.0, 0.0]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    _or                 = False

class ControlBack2ControlStart(trans_criteria_t):
    min_bdot            = 0.0
    max_bdot            = 0.0000001
    min_ang_err         = 0.0
    max_ang_err         = 0.0
    min_ang_vel_err     = [0.0, 0.0, 0.0]
    max_ang_vel_err     = [0.0, 0.0, 0.0]
    min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
    _or                 = False

# class Mode2Desaturate(trans_criteria_t):
#     min_bdot            = 0.0
#     max_bdot            = 0.0000001
#     min_ang_err         = 0.0
#     max_ang_err         = 0.0
#     min_ang_vel_err     = [0.0, 0.0, 0.0]
#     max_ang_vel_err     = [0.0, 0.0, 0.0]
#     min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
#     max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
#     _or                 = False

# class Mode2Desat4EKFRestart(trans_criteria_t):
#     min_bdot            = 0.0
#     max_bdot            = 0.0000001
#     min_ang_err         = 0.0
#     max_ang_err         = 0.0
#     min_ang_vel_err     = [0.0, 0.0, 0.0]
#     max_ang_vel_err     = [0.0, 0.0, 0.0]
#     min_P_diag          = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
#     max_P_diag          = [0.00000001, 0.00000001, 0.00000001, 0.0076, 0.0076, 0.0076]
#     _or                 = False
