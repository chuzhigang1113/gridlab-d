// $Id: motor.cpp 1182 2016-08-15 jhansen $
//	Copyright (C) 2008 Battelle Memorial Institute

#include <stdlib.h>	
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include <iostream>

#include "motor.h"

//////////////////////////////////////////////////////////////////////////
// capacitor CLASS FUNCTIONS
//////////////////////////////////////////////////////////////////////////
CLASS* motor::oclass = NULL;
CLASS* motor::pclass = NULL;

/**
* constructor.  Class registration is only called once to 
* register the class with the core. Include parent class constructor (node)
*
* @param mod a module structure maintained by the core
*/

static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;
static PASSCONFIG clockpass = PC_BOTTOMUP;


motor::motor(MODULE *mod):node(mod)
{
	if(oclass == NULL)
	{
		pclass = node::oclass;
		
		oclass = gl_register_class(mod,"motor",sizeof(motor),passconfig|PC_UNSAFE_OVERRIDE_OMIT|PC_AUTOLOCK);
		if (oclass==NULL)
			throw "unable to register class motor";
		else
			oclass->trl = TRL_PRINCIPLE;

		if(gl_publish_variable(oclass,
			PT_INHERIT, "node",
			PT_double, "base_power[W]", PADDR(Pbase),PT_DESCRIPTION,"base power",
			PT_double, "n", PADDR(n),PT_DESCRIPTION,"ratio of stator auxiliary windings to stator main windings",
			PT_double, "Rds[pu]", PADDR(Rds),PT_DESCRIPTION,"d-axis resistance - single-phase model",
			PT_double, "Rqs[pu]", PADDR(Rqs),PT_DESCRIPTION,"q-asis resistance - single-phase model",
			PT_double, "Rs[ohm]", PADDR(Rs),PT_DESCRIPTION,"stator resistance - three-phase model",
			PT_double, "Rr", PADDR(Rr),PT_DESCRIPTION,"rotor resistance - pu for SPIM, ohm for TPIM",
			PT_double, "Xm", PADDR(Xm),PT_DESCRIPTION,"magnetizing reactance - pu for SPIM, ohm for TPIM",
			PT_double, "Xr", PADDR(Xr),PT_DESCRIPTION,"rotor reactance - pu for SPIM, ohm for TPIM",
			PT_double, "Xs[ohm]", PADDR(Xs),PT_DESCRIPTION,"stator leakage reactance - three-phase model",
			PT_double, "Xc_run[pu]", PADDR(Xc1),PT_DESCRIPTION,"running capacitor reactance - single-phase model",
			PT_double, "Xc_start[pu]", PADDR(Xc2),PT_DESCRIPTION,"starting capacitor reactance - single-phase model",
			PT_double, "Xd_prime[pu]", PADDR(Xd_prime),PT_DESCRIPTION,"d-axis reactance - single-phase model",
			PT_double, "Xq_prime[pu]", PADDR(Xq_prime),PT_DESCRIPTION,"q-axis reactance - single-phase model",
			PT_double, "A_sat", PADDR(Asat),PT_DESCRIPTION,"flux saturation parameter, A - single-phase model",
			PT_double, "b_sat", PADDR(bsat),PT_DESCRIPTION,"flux saturation parameter, b - single-phase model",
			PT_double, "H[s]", PADDR(H),PT_DESCRIPTION,"inertia constant",
			PT_double, "J[kg*m^2]", PADDR(Jm),PT_DESCRIPTION,"moment of inertia",
			PT_double, "number_of_poles", PADDR(pf),PT_DESCRIPTION,"number of poles",
			PT_double, "To_prime[s]", PADDR(To_prime),PT_DESCRIPTION,"rotor time constant",
			PT_double, "capacitor_speed[%]", PADDR(cap_run_speed_percentage),PT_DESCRIPTION,"percentage speed of nominal when starting capacitor kicks in",
			PT_double, "trip_time[s]", PADDR(trip_time),PT_DESCRIPTION,"time motor can stay stalled before tripping off ",
            PT_enumeration,"uv_relay_install",PADDR(uv_relay_install),PT_DESCRIPTION,"is under-voltage relay protection installed on this motor",
				PT_KEYWORD,"INSTALLED",(enumeration)uv_relay_INSTALLED,
				PT_KEYWORD,"UNINSTALLED",(enumeration)uv_relay_UNINSTALLED,
            PT_double, "uv_relay_trip_time[s]", PADDR(uv_relay_trip_time),PT_DESCRIPTION,"time low-voltage condition must exist for under-voltage protection to trip ",
            PT_double, "uv_relay_trip_V[pu]", PADDR(uv_relay_trip_V),PT_DESCRIPTION,"pu minimum voltage before under-voltage relay trips",
            PT_enumeration,"contactor_state",PADDR(contactor_state),PT_DESCRIPTION,"the current status of the motor",
				PT_KEYWORD,"OPEN",(enumeration)contactorOPEN,
				PT_KEYWORD,"CLOSED",(enumeration)contactorCLOSED,
            PT_double, "contactor_open_Vmin[pu]", PADDR(contactor_open_Vmin),PT_DESCRIPTION,"pu voltage at which motor contactor opens",
            PT_double, "contactor_close_Vmax[pu]", PADDR(contactor_close_Vmax),PT_DESCRIPTION,"pu voltage at which motor contactor recloses",
			PT_double, "reconnect_time[s]", PADDR(reconnect_time),PT_DESCRIPTION,"time before tripped motor reconnects",

			//Reconcile torque and speed, primarily
			PT_double, "mechanical_torque[pu]", PADDR(Tmech),PT_DESCRIPTION,"mechanical torque applied to the motor",
			//PT_double, "mechanical_torque_state_var[pu]", PADDR(Tmech_eff),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"Internal state variable torque - three-phase model",
			PT_double, "mechanical_torque_state_var[pu]", PADDR(Tmech_eff),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"Internal state variable torque",
			PT_int32, "iteration_count", PADDR(iteration_count),PT_DESCRIPTION,"maximum number of iterations for steady state model",
			PT_double, "delta_mode_voltage_trigger[%]", PADDR(DM_volt_trig_per),PT_DESCRIPTION,"percentage voltage of nominal when delta mode is triggered",
			PT_double, "delta_mode_rotor_speed_trigger[%]", PADDR(DM_speed_trig_per),PT_DESCRIPTION,"percentage speed of nominal when delta mode is triggered",
			PT_double, "delta_mode_voltage_exit[%]", PADDR(DM_volt_exit_per),PT_DESCRIPTION,"percentage voltage of nominal to exit delta mode",
			PT_double, "delta_mode_rotor_speed_exit[%]", PADDR(DM_speed_exit_per),PT_DESCRIPTION,"percentage speed of nominal to exit delta mode",
			PT_double, "maximum_speed_error", PADDR(speed_error),PT_DESCRIPTION,"maximum speed error for transitioning modes",
			PT_double, "rotor_speed[rad/s]", PADDR(wr),PT_DESCRIPTION,"rotor speed",
			// Below added by Zhigang Chu
			PT_double, "rotor_angle[rad]", PADDR(theta),PT_DESCRIPTION,"rotor angle",
			PT_double, "avRatio", PADDR(avRatio),PT_DESCRIPTION,"Ratio between angle dependent load coefficient and speed dependent one",
			PT_double, "time_delay_SPIM_T[s]", PADDR(t_DLD),PT_DESCRIPTION,"Time delay before triangular torque starts",
			PT_double, "R_stall [pu]", PADDR(R_stall),PT_DESCRIPTION,"Stall resistance of SPIM",
			PT_double, "temperature_SPIM", PADDR(temperature_SPIM),PT_DESCRIPTION, "temperature for SPIM",
			PT_double, "Tth", PADDR(Tth),PT_DESCRIPTION,"SPIM thermal time constant",
			// Above added by Zhigang Chu
			PT_enumeration,"motor_status",PADDR(motor_status),PT_DESCRIPTION,"the current status of the motor",
				PT_KEYWORD,"RUNNING",(enumeration)statusRUNNING,
				PT_KEYWORD,"STALLED",(enumeration)statusSTALLED,
				PT_KEYWORD,"TRIPPED",(enumeration)statusTRIPPED,
				PT_KEYWORD,"OFF",(enumeration)statusOFF,
			PT_int32,"motor_status_number",PADDR(motor_status),PT_DESCRIPTION,"the current status of the motor as an integer",
			PT_enumeration,"desired_motor_state",PADDR(motor_override),PT_DESCRIPTION,"Should the motor be on or off",
				PT_KEYWORD,"ON",(enumeration)overrideON,
				PT_KEYWORD,"OFF",(enumeration)overrideOFF,
			PT_enumeration,"motor_operation_type",PADDR(motor_op_mode),PT_DESCRIPTION,"current operation type of the motor - deltamode related",
				PT_KEYWORD,"SINGLE-PHASE",(enumeration)modeSPIM,
				PT_KEYWORD,"THREE-PHASE",(enumeration)modeTPIM,
			PT_enumeration,"triplex_connection_type",PADDR(triplex_connection_type),PT_DESCRIPTION,"Describes how the motor will connect to the triplex devices",
				PT_KEYWORD,"TRIPLEX_1N",(enumeration)TPNconnected1N,
				PT_KEYWORD,"TRIPLEX_2N",(enumeration)TPNconnected2N,
				PT_KEYWORD,"TRIPLEX_12",(enumeration)TPNconnected12,

			// These model parameters are published as hidden
			PT_double, "wb[rad/s]", PADDR(wbase),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"base speed",
			PT_double, "ws[rad/s]", PADDR(ws),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"system speed",
			PT_complex, "psi_b", PADDR(psi_b),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"backward rotating flux",
			PT_complex, "psi_f", PADDR(psi_f),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"forward rotating flux",
			PT_complex, "psi_dr", PADDR(psi_dr),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"Rotor d axis flux",
			PT_complex, "psi_qr", PADDR(psi_qr),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"Rotor q axis flux",
			PT_complex, "Ids", PADDR(Ids),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"time before tripped motor reconnects",
			PT_complex, "Iqs", PADDR(Iqs),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"time before tripped motor reconnects",
			PT_complex, "If", PADDR(If),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"forward current",
			PT_complex, "Ib", PADDR(Ib),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"backward current",
			PT_complex, "Is", PADDR(Is),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor current",
			PT_complex, "electrical_power[VA]", PADDR(motor_elec_power),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor power",
			PT_double, "electrical_torque[pu]", PADDR(Telec),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"electrical torque",
			PT_complex, "Vs", PADDR(Vs),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor voltage",
			PT_bool, "motor_trip", PADDR(motor_trip),PT_DESCRIPTION,"boolean variable to check if motor is tripped",
			PT_double, "trip", PADDR(trip),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"current time in tripped state",
			PT_double, "reconnect", PADDR(reconnect),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"current time since motor was tripped",


			// These model parameters are published as hidden
			PT_complex, "phips", PADDR(phips),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"positive sequence stator flux",
			PT_complex, "phins_cj", PADDR(phins_cj),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"conjugate of negative sequence stator flux",
			PT_complex, "phipr", PADDR(phipr),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"positive sequence rotor flux",
			PT_complex, "phinr_cj", PADDR(phinr_cj),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"conjugate of negative sequence rotor flux",
			PT_double, "per_unit_rotor_speed[pu]", PADDR(wr_pu),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"rotor speed in per-unit",
			PT_complex, "Ias[pu]", PADDR(Ias),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor phase-a stator current",
			PT_complex, "Ibs[pu]", PADDR(Ibs),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor phase-b stator current",
			PT_complex, "Ics[pu]", PADDR(Ics),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor phase-c stator current",
			PT_complex, "Vas[pu]", PADDR(Vas),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor phase-a stator-to-ground voltage",
			PT_complex, "Vbs[pu]", PADDR(Vbs),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor phase-b stator-to-ground voltage",
			PT_complex, "Vcs[pu]", PADDR(Vcs),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"motor phase-c stator-to-ground voltage",
			PT_complex, "Ips", PADDR(Ips),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"positive sequence stator current",
			PT_complex, "Ipr", PADDR(Ipr),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"positive sequence rotor current",
			PT_complex, "Ins_cj", PADDR(Ins_cj),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"conjugate of negative sequence stator current",
			PT_complex, "Inr_cj", PADDR(Inr_cj),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"conjugate of negative sequence rotor current",
			PT_double, "Ls", PADDR(Ls),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"stator synchronous reactance",
			PT_double, "Lr", PADDR(Lr),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"rotor synchronous reactance",
			PT_double, "sigma1", PADDR(sigma1),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"intermediate variable 1 associated with synch. react.",
			PT_double, "sigma2", PADDR(sigma2),PT_ACCESS,PA_HIDDEN,PT_DESCRIPTION,"intermediate variable 2 associated with synch. react.",

			PT_double_array, "relayProtectionTrip",get_relayProtectionTrip_offset(),
			PT_double_array, "overLoadProtectionTrip",get_overLoadProtectionTrip_offset(),
			PT_double_array, "thermalProtectionTrip",get_thermalProtectionTrip_offset(),
			PT_double_array, "contactorProtectionTrip",get_contactorProtectionTrip_offset(),
			PT_double_array, "emsProtectionTrip",get_emsProtectionTrip_offset(),

			PT_double_array, "relayProtectionReconnect",get_relayProtectionReconnect_offset(),
			PT_double_array, "contactorProtectionReconnect",get_contactorProtectionReconnect_offset(),
			PT_double_array, "emsProtectionReconnect",get_emsProtectionReconnect_offset(),

			PT_enumeration,"TPIM_type",PADDR(TPIM_type),PT_DESCRIPTION,"type of the three-phase motor (A, B, C)",
				PT_KEYWORD,"TPIM_A",(enumeration)TPIM_A,
				PT_KEYWORD,"TPIM_B",(enumeration)TPIM_B,
				PT_KEYWORD,"TPIM_C",(enumeration)TPIM_C,

			PT_enumeration,"SPIM_type",PADDR(SPIM_type),PT_DESCRIPTION,"type of the single phase motors (C: constant torque, S: speed dependent torque)",
				PT_KEYWORD,"SPIM_C",(enumeration)SPIM_C,
				PT_KEYWORD,"SPIM_S",(enumeration)SPIM_S,
				PT_KEYWORD,"SPIM_T",(enumeration)SPIM_T,

			NULL) < 1) GL_THROW("unable to publish properties in %s",__FILE__);

		//Publish deltamode functions
		if (gl_publish_function(oclass,	"delta_linkage_node", (FUNCTIONADDR)delta_linkage)==NULL)
			GL_THROW("Unable to publish motor delta_linkage function");
		if (gl_publish_function(oclass,	"delta_freq_pwr_object", (FUNCTIONADDR)delta_frequency_node)==NULL)
			GL_THROW("Unable to publish motor deltamode function");
		if (gl_publish_function(oclass,	"interupdate_pwr_object", (FUNCTIONADDR)interupdate_motor)==NULL)
			GL_THROW("Unable to publish motor deltamode function");
		if (gl_publish_function(oclass,	"pwr_object_swing_swapper", (FUNCTIONADDR)swap_node_swing_status)==NULL)
			GL_THROW("Unable to publish motor swing-swapping function");
		if (gl_publish_function(oclass,	"attach_vfd_to_pwr_object", (FUNCTIONADDR)attach_vfd_to_node)==NULL)
			GL_THROW("Unable to publish motor VFD attachment function");
    }
}

int motor::create()
{
	int result = node::create();
	last_cycle = 0;

	//Flag variable
	Tmech = -1.0;
	Tmech_eff = 0.0;

	// Default parameters
	motor_override = overrideON;  // share the variable with TPIM
	motor_trip = 0;  // share the variable with TPIM
	Pbase = -999;
	n = 1.22;              
	Rds =0.0365;           
	Rqs = 0.0729;          
	Rr =-999;
	Xm=-999;
	Xr=-999;
	Xc1 = -2.779;           
	Xc2 = -0.7;            
	Xd_prime = 0.1033;      
	Xq_prime =0.1489;       
	bsat = 0.7212;  
	Asat = 5.6;
	H=-999;
	Jm=-999;
	To_prime = 0.1212;
	trip_time = 10;        
	reconnect_time = 300;
	iteration_count = 1000;  // share the variable with TPIM
	cap_run_speed_percentage = 50;
//	DM_volt_trig_per = 80; // share the variable with TPIM
//	DM_speed_trig_per = 80;  // share the variable with TPIM
//	DM_volt_exit_per = 95; // share the variable with TPIM
//	DM_speed_exit_per = 95; // share the variable with TPIM
	DM_volt_trig_per = 130;
	DM_speed_trig_per = 130;
	DM_volt_exit_per = 130;
	DM_speed_exit_per = 130;
	speed_error = 1e-10; // share the variable with TPIM

	wbase=2.0*PI*nominal_frequency;

	// initial parameter for internal model
	trip = 0;
	reconnect = 0;
	psi_b = complex(0.0,0.0);
    psi_f = complex(0.0,0.0);
    psi_dr = complex(0.0,0.0); 
    psi_qr = complex(0.0,0.0); 
    Ids = complex(0.0,0.0);
    Iqs = complex(0.0,0.0);  
    If = complex(0.0,0.0);
    Ib = complex(0.0,0.0);
//    Is = complex(0.0,0.0);
    Is_prev = complex(-999.9,-999.9); // Initialize Is to avoid temperature initialization before current calculation
    motor_elec_power = complex(0.0,0.0);
    Telec = 0; 
    wr = 0;
    
    // Randomized contactor transition values
    //  Used to determine at what voltages the contactor
    //  will open and closed and votlage drops too low.
    uv_relay_install = uv_relay_UNINSTALLED; //Relay not enabled by default.
    uv_relay_time = 0; //counter for how long we've been in under-voltage condition
    uv_relay_trip_time = 0.02; //20ms default
    uv_relay_trip_V = 0.0;
    uv_lockout = 0;
    
    contactor_open_Vmin = 0.0;
    contactor_close_Vmax = 0.01;
    contactor_state = contactorCLOSED;

    //Mode initialization
    motor_op_mode = modeSPIM; // share the variable with TPIM

    //Three-phase induction motor parameters, 500 HP, 2.3kV
    pf = 4;
    Rs= 0.262;
    Xs= 1.206;
    rs_pu = -999;  // pu
    lls = -999;  //  pu
    lm = -999;  // pu
    rr_pu = -999;  // pu
    llr = -999;  // pu

    // Parameters are for 3000 W motor
    Kfric = 0.0;  // pu
	phips = complex(0.0,0.0); // pu
	phins_cj = complex(0.0,0.0); // pu
	phipr = complex(0.0,0.0); // pu
	phinr_cj = complex(0.0,0.0); // pu
	wr_pu = 0.97; // pu
	Ias = complex(0.0,0.0); // pu
	Ibs = complex(0.0,0.0); // pu
	Ics = complex(0.0,0.0); // pu
	Vas = complex(0.0,0.0); // pu
	Vbs = complex(0.0,0.0); // pu
	Vcs = complex(0.0,0.0); // pu
	Ips = complex(0.0,0.0); // pu
	Ipr = complex(0.0,0.0); // pu
	Ins_cj = complex(0.0,0.0); // pu
	Inr_cj = complex(0.0,0.0); // pu
	Ls = 0.0; // pu
	Lr = 0.0; // pu
	sigma1 = 0.0; // pu
	sigma2 = 0.0; // pu
	ws_pu = 1.0; // pu

	triplex_connected = false;	//By default, not connected to triplex
	triplex_connection_type = TPNconnected12;	//If it does end up being triplex, we default it to L1-L2

	// Initialization of protection parameters
	hasRelayProtection = false;
	hasOverLoadProtection = false;
	hasThermalProtection = false;
	hasContactorProtection = false;
	hasEMSProtection = false;

	relayTrip = false;
	overLoadTrip = false;
	thermalTrip = false;
	contactorTrip = false;
	emsTrip = false;

	relayReconnect = false;
	contactorReconnect = false;
	emsReconnect = false;

	TPIM_type = TPIM_A; // default to motor A

	// Below added by Zhigang Chu
	SPIM_type = SPIM_C; // default to constant torque single phase motor
	theta = -999;
	t_DLD = -999;
	avRatio = -999;
	R_stall = -999;
	temperature_SPIM = 0;
	Tth = -999;
	// Above added by Zhigang Chu

	return result;
}

int motor::init(OBJECT *parent)
{
	unsigned int rowNum;
	unsigned int colNum;

	OBJECT *obj = OBJECTHDR(this);
	int result = node::init(parent);

	// Check what phases are connected on this motor
	int num_phases = 0;
	if (has_phase(PHASE_A)==true)
		num_phases++;

	if (has_phase(PHASE_B)==true)
		num_phases++;

	if (has_phase(PHASE_C)==true)
		num_phases++;
	
	// error out if we have more than one phase
	if (num_phases == 1)
	{
		motor_op_mode = modeSPIM;
	}
	else if (num_phases == 3)
	{
		motor_op_mode = modeTPIM;
	}
	else
	{
		GL_THROW("motor:%s -- only single-phase or three-phase motors are supported",(obj->name ? obj->name : "Unnamed"));
		/*  TROUBLESHOOT
		The motor only supports single-phase and three-phase motors at this time.  Please use one of these connection types.
		*/
	}

	//Check the initial torque conditions
	if (Tmech == -1.0)
	{
		//See which one we are
		if (motor_op_mode == modeSPIM)
		{
			Tmech = 1.0448;
		}
		else	//Assume 3 phase
		{
			Tmech = 0.95;

			//Check mode
			if (motor_status != statusOFF)
			{
				Tmech_eff = Tmech;
			}
		}
	}
	//Default else - user specified it


	//Check default rated power
	if (Pbase == -999)
	{
		//See which one we are
		if (motor_op_mode == modeSPIM)
		{
			Pbase = 3500;
		}
		else	//Assume 3 phase
		{
			Pbase = 372876;
		}
	}
	//Default else - user specified it

	//Check default magnetizing inductance
	if (Xm == -999)
	{
		//See which one we are
		if (motor_op_mode == modeSPIM)
		{
			Xm = 2.28;
		}
		else	//Assume 3 phase
		{
			Xm = 56.02;
		}
	}
	//Default else - user specified it

	//Check default rotor inductance
	if (Xr == -999)
	{
		//See which one we are
		if (motor_op_mode == modeSPIM)
		{
			Xr = 2.33;
		}
		else	//Assume 3 phase
		{
			Xr = 1.206;
		}
	}
	//Default else - user specified it

    //Check default rotor resistance
	if (Rr == -999)
	{
		//See which one we are
		if (motor_op_mode == modeSPIM)
		{
			Rr = 0.0486;
		}
		else	//Assume 3 phase
		{
			Rr = 0.187;
		}
	}
	//Default else - user specified it

	// Moment of inertia and/or inertia constant
	if ((Jm == -999) && (H == -999)) // none specified
	{
		//See which one we are
		if (motor_op_mode == modeSPIM)
		{
			Jm = 0.0019701;
			H = 0.04;
		}
		else	//Assume 3 phase
		{
			Jm = 11.06;
			H = 0.52694;
		}
	}
	else if ((Jm != -999) && (H == -999)) // Jm but not H specified; calculate H
	{
		H = 1/2*pow(2/pf,2)*Jm*pow(wbase,2)/Pbase;
	}
	else if ((Jm == -999) && (H != -999)) // H but not Jm specified
	{
		Jm = H /(1/2*pow(2/pf,2)*pow(wbase,2)/Pbase); // no need to calculate Jm, only H is needed
	}
	else if ((Jm != -999) && (H != -999)) // both Jm and H specified; priority to H
	{
		gl_warning("motor:%s -- both H and J were specified, H will be used, J is ignored",(obj->name ? obj->name : "Unnamed"));
		/*  TROUBLESHOOT
		Both H and J were specified, H will be used, J is ignored.
		*/
	}

	// Per unit parameters
	Zbase = 3*pow(nominal_voltage,2)/Pbase;
	rs_pu = Rs/Zbase;  // pu
	lls = Xs/Zbase;  //  pu
	rr_pu = Rr/Zbase;  // pu
	llr = Xr/Zbase;  // pu
	lm = Xm/Zbase;  // pu


	// determine the specific phase this motor is connected to
	if (motor_op_mode == modeSPIM)
	{
		if (has_phase(PHASE_S))
		{
			connected_phase = -1;		//Arbitrary
			triplex_connected = true;	//Flag us as triplex
		}
		else	//Three-phase
		{
			//Affirm the triplex flag is not set
			triplex_connected = false;

			if (has_phase(PHASE_A)) {
				connected_phase = 0;
			}
			else if (has_phase(PHASE_B)) {
				connected_phase = 1;
			}
			else {	//Phase C, by default
				connected_phase = 2;
			}
		}
	}
	//Default else -- three-phase diagnostics (none needed right now)

	//Parameters
	if (motor_op_mode == modeSPIM)
	{
		if ((triplex_connected == true) && (triplex_connection_type == TPNconnected12))
		{
			Ibase = Pbase/(2.0*nominal_voltage);	//To reflect LL connection
		}
		else	//"Three-phase" or "normal" triplex
		{
			Ibase = Pbase/nominal_voltage;
		}
	}
	else	//Three-phase
	{
		Ibase = Pbase/nominal_voltage/3.0;
	}


	cap_run_speed = (cap_run_speed_percentage*wbase)/100;
	DM_volt_trig = (DM_volt_trig_per)/100;
	DM_speed_trig = (DM_speed_trig_per*wbase)/100;
	DM_volt_exit = (DM_volt_exit_per)/100;
	DM_speed_exit = (DM_speed_exit_per*wbase)/100;

	//TPIM Items
	Ls = lls + lm; // pu
	Lr = llr + lm; // pu
	sigma1 = Ls - lm * lm / Lr; // pu
	sigma2 = Lr - lm * lm / Ls; // pu

	if ((motor_op_mode == modeTPIM) && (motor_status != statusRUNNING)){
		wr_pu = 0.0; // pu
		wr = 0.0;
	}
	else if ((motor_op_mode == modeTPIM) && (motor_status == statusRUNNING)){
		wr_pu = 1.0; // pu
		wr = wbase;
	}

	wr_pu_prev = wr_pu;

	// Below added by Zhigang Chu
	if (theta == -999) {
		theta = 0; // Starts from angle 0 by default
	}

	if (t_DLD == -999) {
		t_DLD = 0.2; // Wait for some time, and then start triangular torque. 0.2s by default
	}

	if (avRatio == -999) {
		avRatio = 1.333; // By default, Tmech = 6, Tav = 8 = Tmech*avRatio
	}

	if (R_stall == -999) {
			R_stall = 0.124; // Default stalling resistance
		}

	if (Tth == -999) {
				Tth = 10; // Default thermal time constant
			}
	// Above added by Zhigang Chu
    
    // Checking contactor open and close min and max voltages
    if (contactor_open_Vmin > contactor_close_Vmax){
        GL_THROW("motor:%s -- contactor_open_Vmin must be less or equal to than contactor_close_Vmax",(obj->name ? obj->name : "Unnamed"));
    }

    
    // Checking under-voltage relay input parameters
    if (uv_relay_trip_time < 0 ){
        GL_THROW("motor:%s -- uv_relay_trip_time must be greater than or equal to 0",(obj->name ? obj->name : "Unnamed"));
    }
    if (uv_relay_trip_V < 0 || uv_relay_trip_V > 1){
        GL_THROW("motor:%s -- uv_relay_trip_V must be greater than or equal to 0 and less than or equal to 1",(obj->name ? obj->name : "Unnamed"));
    }

    // Protection initialization
    // Check size of each protection type
    // Realy protection
	rowNum = relayProtectionTrip.get_rows();
	colNum = relayProtectionTrip.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of relay protection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 1 ){
        GL_THROW("motor:%s -- Volt-time curve of relay protection must have only one volt-time point",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 0) {
    	//Check to see if we're allocated first
		if (relayTimerList == NULL)
		{
			//Allocate it - one for each bus
			relayTimerList = (double *)gl_malloc(colNum*sizeof(double));
			relayTimerList_prev = (double *)gl_malloc(colNum*sizeof(double));

			//Check to see if it worked
			if (relayTimerList == NULL || relayTimerList_prev == NULL)
			{
				GL_THROW("motor:%s: failed to allocate array for tracking relay trip timers", (obj->name ? obj->name : "Unnamed"));
				/*  TROUBLESHOOT
				While attempting to allocate an array used to track relay trip timers,
				an error occurred.  Please try again.  If the error persists, please submit your code and a bug
				report via the ticketing system.
				*/
			}
		}

		//Zero it
		for (int i = 0; i < colNum; i++)
		{
			relayTimerList[i] = 0.0;	// Starts with 0 timer
			relayTimerList_prev[i] = 0.0;
		}

    	hasRelayProtection = true;
    }

    // Over load protection
	rowNum = overLoadProtectionTrip.get_rows();
	colNum = overLoadProtectionTrip.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of over load protection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 0) {
    	//Check to see if we're allocated first
		if (overLoadTimerList == NULL)
		{
			//Allocate it - one for each bus
			overLoadTimerList = (double *)gl_malloc(colNum*sizeof(double));
			overLoadTimerList_prev = (double *)gl_malloc(colNum*sizeof(double));

			//Check to see if it worked
			if (overLoadTimerList == NULL || overLoadTimerList_prev == NULL)
			{
				GL_THROW("motor:%s: failed to allocate array for tracking overload trip timers", (obj->name ? obj->name : "Unnamed"));
				/*  TROUBLESHOOT
				While attempting to allocate an array used to track relay trip timers,
				an error occurred.  Please try again.  If the error persists, please submit your code and a bug
				report via the ticketing system.
				*/
			}
		}

		//Zero it
		for (int i = 0; i < colNum; i++)
		{
			overLoadTimerList[i] = 0.0;	// Starts with 0 timer
			overLoadTimerList_prev[i] = 0.0;
		}

    	hasOverLoadProtection= true;
    }

    // Thermal protection
	rowNum = thermalProtectionTrip.get_rows();
	colNum = thermalProtectionTrip.get_cols();
	// Distinguish between SPIM and TPIM, using different protection scheme
	if (motor_op_mode == modeTPIM) {
		if (rowNum != 2 && rowNum != 0){
		        GL_THROW("motor:%s -- Volt-time curve of thermal protection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
		    }
		    if (colNum > 0) {
		    	//Check to see if we're allocated first
				if (thermalTimerList == NULL)
				{
					//Allocate it - one for each bus
					thermalTimerList = (double *)gl_malloc(colNum*sizeof(double));
					thermalTimerList_prev = (double *)gl_malloc(colNum*sizeof(double));

					//Check to see if it worked
					if (thermalTimerList == NULL || thermalTimerList_prev == NULL)
					{
						GL_THROW("motor:%s: failed to allocate array for tracking thermal trip timers", (obj->name ? obj->name : "Unnamed"));
						/*  TROUBLESHOOT
						While attempting to allocate an array used to track relay trip timers,
						an error occurred.  Please try again.  If the error persists, please submit your code and a bug
						report via the ticketing system.
						*/
					}
				}

				//Zero it
				for (int i = 0; i < colNum; i++)
				{
					thermalTimerList[i] = 0.0;	// Starts with 0 timer
					thermalTimerList_prev[i] = 0.0;
				}

		    	hasThermalProtection = true;
		    }
	}
	else { // SPIM use temperature to determine thermal protection
		if (rowNum != 1 && rowNum != 0){
				        GL_THROW("motor:%s -- SPIM thermal protection must have only one row",(obj->name ? obj->name : "Unnamed"));
				    }
		if (colNum != 1 && colNum != 0){
				        GL_THROW("motor:%s -- SPIM thermal protection must have only one column",(obj->name ? obj->name : "Unnamed"));
				    }
		if (colNum > 0) {
			//Check to see if we're allocated first
			if (thermalTimerList == NULL)
			{
				//Allocate it - one for each bus
				thermalTimerList = (double *)gl_malloc(colNum*sizeof(double));
				thermalTimerList_prev = (double *)gl_malloc(colNum*sizeof(double));

				//Check to see if it worked
				if (thermalTimerList == NULL || thermalTimerList_prev == NULL)
				{
					GL_THROW("motor:%s: failed to allocate array for tracking thermal trip timers", (obj->name ? obj->name : "Unnamed"));
					/*  TROUBLESHOOT
					While attempting to allocate an array used to track relay trip timers,
					an error occurred.  Please try again.  If the error persists, please submit your code and a bug
					report via the ticketing system.
					 */
				}
			}

			//Zero it
			for (int i = 0; i < colNum; i++)
			{
				thermalTimerList[i] = 0.0;	// Starts with 0 timer
				thermalTimerList_prev[i] = 0.0;
			}

			hasThermalProtection = true;
		}
	}


    // Contactor protection
	rowNum = contactorProtectionTrip.get_rows();
	colNum = contactorProtectionTrip.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of contactor protection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 1 ){
        GL_THROW("motor:%s -- Volt-time curve of contactor protection must have only one volt-time point",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 0) {
    	//Check to see if we're allocated first
		if (contactorTimerList == NULL)
		{
			//Allocate it - one for each bus
			contactorTimerList = (double *)gl_malloc(colNum*sizeof(double));
			contactorTimerList_prev = (double *)gl_malloc(colNum*sizeof(double));

			//Check to see if it worked
			if (contactorTimerList == NULL || contactorTimerList_prev == NULL)
			{
				GL_THROW("motor:%s: failed to allocate array for tracking contactor trip timers", (obj->name ? obj->name : "Unnamed"));
				/*  TROUBLESHOOT
				While attempting to allocate an array used to track relay trip timers,
				an error occurred.  Please try again.  If the error persists, please submit your code and a bug
				report via the ticketing system.
				*/
			}
		}

		//Zero it
		for (int i = 0; i < colNum; i++)
		{
			contactorTimerList[i] = 0.0;	// Starts with 0 timer
			contactorTimerList_prev[i] = 0.0;
		}

    	hasContactorProtection = true;
    }

    // EMS protection
	rowNum = emsProtectionTrip.get_rows();
	colNum = emsProtectionTrip.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of EMS protection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 0) {
    	//Check to see if we're allocated first
		if (emsTimerList == NULL)
		{
			//Allocate it - one for each bus
			emsTimerList = (double *)gl_malloc(colNum*sizeof(double));
			emsTimerList_prev = (double *)gl_malloc(colNum*sizeof(double));

			//Check to see if it worked
			if (emsTimerList == NULL || emsTimerList_prev == NULL)
			{
				GL_THROW("motor:%s: failed to allocate array for tracking EMS trip timers", (obj->name ? obj->name : "Unnamed"));
				/*  TROUBLESHOOT
				While attempting to allocate an array used to track relay trip timers,
				an error occurred.  Please try again.  If the error persists, please submit your code and a bug
				report via the ticketing system.
				*/
			}
		}

		//Zero it
		for (int i = 0; i < colNum; i++)
		{
			emsTimerList[i] = 0.0;	// Starts with 0 timer
			emsTimerList_prev[i] = 0.0;
		}

    	hasEMSProtection= true;
    }

    // Check each reconnection Volt-time curve entered by user
    // Relay reconnection
	rowNum = relayProtectionReconnect.get_rows();
	colNum = relayProtectionReconnect.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of relay reconnection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 1){
        GL_THROW("motor:%s -- Volt-time curve of relay reconnection must have only one volt-time point",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum == 1) {

    	relayTimerReconnect = 0.0;
    	relayTimerReconnect_prev = 0.0;

    }

    // Contactor reconnection
	rowNum = contactorProtectionReconnect.get_rows();
	colNum = contactorProtectionReconnect.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of contactor reconnection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 1){
        GL_THROW("motor:%s -- Volt-time curve of contactor reconnection must have only one volt-time point",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum == 1) {

    	contactorTimerReconnect = 0.0;
    	contactorTimerReconnect_prev = 0.0;

    }

    // EMS reconnection
	rowNum = emsProtectionReconnect.get_rows();
	colNum = emsProtectionReconnect.get_cols();
    if (rowNum != 2 && rowNum != 0){
        GL_THROW("motor:%s -- Volt-time curve of EMS reconnection must have 2 rows, one is for voltage, one is for time",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum > 1){
        GL_THROW("motor:%s -- Volt-time curve of EMS reconnection must have only one volt-time point",(obj->name ? obj->name : "Unnamed"));
    }
    if (colNum == 1) {

    	emsTimerReconnect = 0.0;
    	emsTimerReconnect_prev= 0.0;

    }

	return result;
}

int motor::isa(char *classname)
{
	return strcmp(classname,"motor")==0 || node::isa(classname);
}

TIMESTAMP motor::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	// we need to initialize the last cycle variable
	if (last_cycle == 0) {
		last_cycle = t1;
	}
	else if ((double)t1 > last_cycle) {
		delta_cycle = (double)t1-last_cycle;
	}
	
	//Must be at the bottom, or the new values will be calculated after the fact
	TIMESTAMP result = node::presync(t1);
	return result;
}

TIMESTAMP motor::sync(TIMESTAMP t0, TIMESTAMP t1)
{
	// update voltage and frequency
	updateFreqVolt();

	if (motor_op_mode == modeSPIM)
	{
		if((double)t1 == last_cycle) { // if time did not advance, load old values
			SPIMreinitializeVars();
		}
		else if((double)t1 > last_cycle){ // time advanced, time to update varibles
			SPIMupdateVars();
		}
		else {
			gl_error("current time is less than previous time");
		}

		// update protection
		UpdateProtection(delta_cycle);

		if (motor_override == overrideON && ws > 1 && Vs.Mag() > 0.1 && motor_trip == 0) { // motor is currently connected and grid conditions are not "collapsed"
			// run the steady state solver
			SPIMSteadyState(t1);

			// update current draw
			if (triplex_connected==true)
			{
				//See which type of triplex
				if (triplex_connection_type == TPNconnected1N)
				{
					pre_rotated_current[0] = Is*Ibase;
				}
				else if (triplex_connection_type == TPNconnected2N)
				{
					pre_rotated_current[1] = Is*Ibase;
				}
				else	//Assume it is 12 now
				{
					pre_rotated_current[2] = Is*Ibase;
				}
			}
			else	//"Three-phase" connection
			{
				pre_rotated_current[connected_phase] = Is*Ibase;
			}
		}
		else { // motor is currently disconnected
			if (triplex_connected==true)
			{
				//See which type of triplex
				if (triplex_connection_type == TPNconnected1N)
				{
					pre_rotated_current[0] = complex(0.0,0.0);
				}
				else if (triplex_connection_type == TPNconnected2N)
				{
					pre_rotated_current[1] = complex(0.0,0.0);
				}
				else	//Assume it is 12 now
				{
					pre_rotated_current[2] = complex(0.0,0.0);
				}

				//Set off
				SPIMStateOFF(0);
			}
			else	//"Three-phase" connection
			{
				pre_rotated_current[connected_phase] = 0;
				SPIMStateOFF(0);
			}
		}

		// update motor status
		SPIMUpdateMotorStatus();
	}
	else	//Must be three-phase
	{
		//Three-phase steady-state code
		if((double)t1 == last_cycle) { // if time did not advance, load old values
			TPIMreinitializeVars();
		}
		else if((double)t1 > last_cycle){ // time advanced, time to update variables
			TPIMupdateVars();
		}
		else {
			gl_error("current time is less than previous time");
		}

		// update protection
		UpdateProtection(delta_cycle);

		if (motor_override == overrideON && ws_pu > 0.1 &&  Vas.Mag() > 0.1 && Vbs.Mag() > 0.1 && Vcs.Mag() > 0.1 &&
			motor_trip == 0) { // motor is currently connected and grid conditions are not "collapsed"
			// This needs to be re-visited to determine the lower bounds of ws_pu and Vas, Vbs and Vcs

			// run the steady state solver
			TPIMSteadyState(t1);

			// update current draw -- might need to be pre_rotated_current
			pre_rotated_current[0] = Ias*Ibase; // A
			pre_rotated_current[1] = Ibs*Ibase; // A
			pre_rotated_current[2] = Ics*Ibase; // A
		}
		else { // motor is currently disconnected
			pre_rotated_current[0] = 0; // A
			pre_rotated_current[1] = 0; // A
			pre_rotated_current[2] = 0; // A
			TPIMStateOFF();
		}

		// update motor status
		TPIMUpdateMotorStatus();
	}

	//Must be at the bottom, or the new values will be calculated after the fact
	TIMESTAMP result = node::sync(t1);

	if ((double)t1 > last_cycle) {	
		last_cycle = (double)t1;
	}

	// figure out if we need to enter delta mode on the next pass
	if (motor_op_mode == modeSPIM)
	{
		if (((Vs.Mag() < DM_volt_trig) || (wr < DM_speed_trig)) && deltamode_inclusive)
		{
			// we should not enter delta mode if the motor is tripped or not close to reconnect
//			if ((motor_trip == 1 && reconnect < reconnect_time-1)  || (motor_override == overrideOFF)) {
//				return result;
//			}

            // we are not tripped and the motor needs to enter delta mode to capture the dynamics
			// Feature 1105: we always do delta mode simulations!
            schedule_deltamode_start(t1);
            return t1;
        }
        else
        {
            return result;
        }
        
	}
	else	//Must be three-phase
	{
		//This may need to be updated for three-phase
		if ( ((Vas.Mag() < DM_volt_trig) || (Vbs.Mag() < DM_volt_trig) ||
				(Vcs.Mag() < DM_volt_trig) || (wr < DM_speed_trig))
				&& deltamode_inclusive)
		{
			// we should not enter delta mode if the motor is tripped or not close to reconnect
//			if ((motor_trip == 1 && reconnect < reconnect_time-1) || (motor_override == overrideOFF)) {
//				return result;
//			}

            // we are not tripped and the motor needs to enter delta mode to capture the dynamics
			// Feature 1105: we always do delta mode simulations!
            schedule_deltamode_start(t1);
            return t1;
        }
        else
        {
            return result;
        }
	}
}

TIMESTAMP motor::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	//Must be at the bottom, or the new values will be calculated after the fact
	TIMESTAMP result = node::postsync(t1);
	return result;
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF DELTA MODE
//////////////////////////////////////////////////////////////////////////

//Module-level call
SIMULATIONMODE motor::inter_deltaupdate(unsigned int64 delta_time, unsigned long dt, unsigned int iteration_count_val, bool interupdate_pos)
{
	OBJECT *hdr = OBJECTHDR(this);
	STATUS return_status_val;

	// make sure to capture the current time
	curr_delta_time = gl_globaldeltaclock;

	// I need the time delta in seconds
	double deltaTime = (double)dt/(double)DT_SECOND;

	//mostly for GFA functionality calls
	if ((iteration_count_val==0) && (interupdate_pos == false) && (fmeas_type != FM_NONE)) 
	{
		//Update frequency calculation values (if needed)
		memcpy(&prev_freq_state,&curr_freq_state,sizeof(FREQM_STATES));
	}

	//In the first call we need to initilize the dynamic model
	if ((delta_time==0) && (iteration_count_val==0) && (interupdate_pos == false))	//First run of new delta call
	{
		//Call presync-equivalent items
		NR_node_presync_fxn(0);
		t_DLD = curr_delta_time + t_DLD;
		if (fmeas_type != FM_NONE) {
			//Initialize dynamics
			init_freq_dynamics();
		}

		if (motor_op_mode == modeSPIM)
		{
			// update voltage and frequency
			updateFreqVolt();

			// update previous values for the model
			SPIMupdateVars();

			SPIMSteadyState(curr_delta_time);

			// update motor status
			SPIMUpdateMotorStatus();
		}
		else	//Three-phase
		{
			//first run/deltamode initialization stuff
			// update voltage and frequency
			updateFreqVolt();

			// update previous values for the model
			TPIMupdateVars();

			TPIMSteadyState(curr_delta_time);

			// update motor status
			TPIMUpdateMotorStatus();
		}
	}//End first pass and timestep of deltamode (initial condition stuff)

	if (interupdate_pos == false)	//Before powerflow call
	{
		//Call presync-equivalent items
		if (delta_time>0) {
			NR_node_presync_fxn(0);

			// update voltage and frequency
			updateFreqVolt();
		}

		if (motor_op_mode == modeSPIM)
		{
			// if deltaTime is not small enough we will run into problems
			if (deltaTime > 0.0003) {
				gl_warning("Delta time for the SPIM model needs to be lower than 0.0003 seconds");
			}

			if(curr_delta_time == last_cycle) { // if time did not advance, load old values
				SPIMreinitializeVars();
			}
			else if(curr_delta_time > last_cycle){ // time advanced, time to update varibles
				SPIMupdateVars();
			}
			else {
				gl_error("current time is less than previous time");
			}

			// update protection
			if (delta_time > 0) {
				UpdateProtection(deltaTime);
			}

			if (motor_override == overrideON && ws > 1 && Vs.Mag() > 0.1 && motor_trip == 0) { // motor is currently connected and grid conditions are not "collapsed"
				// run the dynamic solver
//				SPIMDynamic(curr_delta_time, deltaTime);
				SPIMDynamic(deltaTime);

				// update current draw
				if (triplex_connected == true)
				{
					//See which type of triplex
					if (triplex_connection_type == TPNconnected1N)
					{
						pre_rotated_current[0] = Is*Ibase;
					}
					else if (triplex_connection_type == TPNconnected2N)
					{
						pre_rotated_current[1] = Is*Ibase;
					}
					else	//Assume it is 12 now
					{
						pre_rotated_current[2] = Is*Ibase;
					}
				}
				else	//"Three-phase" connection
				{
					pre_rotated_current[connected_phase] = Is*Ibase;
				}
			}
			else { // motor is currently disconnected
				if (triplex_connected == true)
				{
					//See which type of triplex
					if (triplex_connection_type == TPNconnected1N)
					{
						pre_rotated_current[0] = complex(0.0,0.0);
					}
					else if (triplex_connection_type == TPNconnected2N)
					{
						pre_rotated_current[1] = complex(0.0,0.0);
					}
					else	//Assume it is 12 now
					{
						pre_rotated_current[2] = complex(0.0,0.0);
					}

					//Set off
					SPIMStateOFF(deltaTime);
				}
				else	//"Three-phase" connection
				{
					pre_rotated_current[connected_phase] = 0;
					SPIMStateOFF(deltaTime);
				}
			}

			// update motor status
			SPIMUpdateMotorStatus();
		}
		else 	//Must be three-phase
		{
			// if deltaTime is not small enough we will run into problems
			if (deltaTime > 0.0005) {
				gl_warning("Delta time for the TPIM model needs to be lower than 0.0005 seconds");
			}

			if(curr_delta_time == last_cycle) { // if time did not advance, load old values
				TPIMreinitializeVars();
			}
			else if(curr_delta_time > last_cycle){ // time advanced, time to update varibles
				TPIMupdateVars();
			}
			else {
				gl_error("current time is less than previous time");
			}

			// update protection
			if (delta_time>0) {
				UpdateProtection(deltaTime);
			}

			if (motor_override == overrideON && ws_pu > 0.1 && Vas.Mag() > 0.1 && Vbs.Mag() > 0.1 && Vcs.Mag() > 0.1 &&
					motor_trip == 0) { // motor is currently connected and grid conditions are not "collapsed"
				// run the dynamic solver
				TPIMDynamic(curr_delta_time, deltaTime);

				// update current draw -- pre_rotated_current
				pre_rotated_current[0] = Ias*Ibase; // A
				pre_rotated_current[1] = Ibs*Ibase; // A
				pre_rotated_current[2] = Ics*Ibase; // A
			}
			else { // motor is currently disconnected
				pre_rotated_current[0] = 0; // A
				pre_rotated_current[1] = 0; // A
				pre_rotated_current[2] = 0; // A;
				TPIMStateOFF();
			}

			// update motor status
			TPIMUpdateMotorStatus();
		}

		//Call sync-equivalent items (solver occurs at end of sync)
		NR_node_sync_fxn(hdr);

		if (curr_delta_time > last_cycle) {
			last_cycle = curr_delta_time;
		}

		return SM_DELTA;
	}
	else // After powerflow call
	{
		//Perform postsync-like updates on the values
		BOTH_node_postsync_fxn(hdr);
	
		//Frequency measurement stuff
		if (fmeas_type != FM_NONE)
		{
			return_status_val = calc_freq_dynamics(deltaTime);

			//Check it
			if (return_status_val == FAILED)
			{
				return SM_ERROR;
			}
		}//End frequency measurement desired
		//Default else -- don't calculate it

		if (motor_op_mode == modeSPIM)
		{
			// figure out if we need to exit delta mode on the next pass
			if ((Vs.Mag() > DM_volt_exit) && (wr > DM_speed_exit)
					&& ((fabs(wr_pu-wr_pu_prev)*wbase) > speed_error))
			{
				// we return to steady state if the voltage and speed is good
				return SM_EVENT;
//			} else if (motor_trip == 1 && reconnect < reconnect_time-1) {
//				// we return to steady state if the motor is tripped
//				return SM_EVENT;
			} else if (motor_override == overrideOFF) {
				//We're off at the moment, so assume back to event driven mode
				return SM_EVENT;
			}

			//Default - stay in deltamode
			return SM_DELTA;
		}
		else	//Must be three-phase
		{
			// figure out if we need to exit delta mode on the next pass
			if ((Vas.Mag() > DM_volt_exit) && (Vbs.Mag() > DM_volt_exit) && (Vcs.Mag() > DM_volt_exit)
					&& (wr > DM_speed_exit) && ((fabs(wr_pu-wr_pu_prev)*wbase) > speed_error))
			{
				return SM_EVENT;
			}
//			else if (motor_trip == 1 && reconnect < reconnect_time-1) {
//				// we return to steady state if the motor is tripped
//				return SM_EVENT;
//			}
			else if (motor_override == overrideOFF) {
				//We're off at the moment, so assume back to event driven mode
				return SM_EVENT;
			}

			//Default - stay in deltamode
			return SM_DELTA;
		}
	}
}

// function to update motor frequency and voltage
void motor::updateFreqVolt() {
	// update voltage and frequency
	if (motor_op_mode == modeSPIM)
	{
		if ( (SubNode == CHILD) || (SubNode == DIFF_CHILD) ) // if we have a parent, reference the voltage and frequency of the parent
		{
			node *parNode = OBJECTDATA(SubNodeParent,node);
			if (triplex_connected == true)
			{
				//See which type of triplex
				if (triplex_connection_type == TPNconnected1N)
				{
					Vs = parNode->voltage[0]/parNode->nominal_voltage;
					ws = parNode->curr_freq_state.fmeas[0]*2.0*PI;
				}
				else if (triplex_connection_type == TPNconnected2N)
				{
					Vs = parNode->voltage[1]/parNode->nominal_voltage;
					ws = parNode->curr_freq_state.fmeas[1]*2.0*PI;
				}
				else	//Assume it is 12 now
				{
					Vs = parNode->voltaged[0]/(2.0*parNode->nominal_voltage);	//To reflect LL connection
					ws = parNode->curr_freq_state.fmeas[0]*2.0*PI;
				}
			}
			else	//"Three-phase" connection
			{
				Vs = parNode->voltage[connected_phase]/parNode->nominal_voltage;
				ws = parNode->curr_freq_state.fmeas[connected_phase]*2.0*PI;
			}
		}
		else // No parent, use our own voltage
		{
			if (triplex_connected == true)
			{
				//See which type of triplex
				if (triplex_connection_type == TPNconnected1N)
				{
					Vs = voltage[0]/nominal_voltage;
					ws = curr_freq_state.fmeas[0]*2.0*PI;
				}
				else if (triplex_connection_type == TPNconnected2N)
				{
					Vs = voltage[1]/nominal_voltage;
					ws = curr_freq_state.fmeas[1]*2.0*PI;
				}
				else	//Assume it is 12 now
				{
					Vs = voltaged[0]/(2.0*nominal_voltage);	//To reflect LL connection
					ws = curr_freq_state.fmeas[0]*2.0*PI;
				}
			}
			else 	//"Three-phase" connection
			{
				Vs = voltage[connected_phase]/nominal_voltage;
				ws = curr_freq_state.fmeas[connected_phase]*2.0*PI;
			}
		}

		//Make the per-unit value
		ws_pu = ws/wbase;
	}
	else //TPIM model
	{
		if ( (SubNode == CHILD) || (SubNode == DIFF_CHILD) ) // if we have a parent, reference the voltage and frequency of the parent
		{
			node *parNode = OBJECTDATA(SubNodeParent,node);
			// obtain 3-phase voltages
			Vas = parNode->voltage[0]/parNode->nominal_voltage;
			Vbs = parNode->voltage[1]/parNode->nominal_voltage;
			Vcs = parNode->voltage[2]/parNode->nominal_voltage;
			// obtain frequency in pu, take the average of 3-ph frequency ?
			ws_pu = (parNode->curr_freq_state.fmeas[0]+ parNode->curr_freq_state.fmeas[1] + parNode->curr_freq_state.fmeas[2]) / nominal_frequency / 3.0;
		}
		else // No parent, use our own voltage
		{
			Vas = voltage[0]/nominal_voltage;
			Vbs = voltage[1]/nominal_voltage;
			Vcs = voltage[2]/nominal_voltage;
			ws_pu = (curr_freq_state.fmeas[0] + curr_freq_state.fmeas[1] + curr_freq_state.fmeas[2]) / nominal_frequency / 3.0;
		}

		//Make the non-per-unit value
		ws = ws_pu * wbase;
	}
}

// function to update the previous values for the motor model
void motor::SPIMupdateVars() {

	int colNum;

	wr_prev = wr;
	Telec_prev = Telec; 
	psi_dr_prev = psi_dr;
	psi_qr_prev = psi_qr;
	psi_f_prev = psi_f; 
	psi_b_prev = psi_b;
	Iqs_prev = Iqs;
	Ids_prev =Ids;
	If_prev = If;
	Ib_prev = Ib;
	Is_prev = Is;
	motor_elec_power_prev = motor_elec_power;
	psi_sat_prev = psi_sat;

	reconnect_prev = reconnect;
	motor_trip_prev = motor_trip;
	trip_prev = trip;

	// Update timer for protection
	if (hasRelayProtection == true) {

		colNum = relayProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			relayTimerList_prev[i] = relayTimerList[i];
		}

	}
	if (hasOverLoadProtection == true) {

		colNum = overLoadProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			overLoadTimerList_prev[i] = overLoadTimerList[i];
		}
	}
	if (hasThermalProtection == true) {

		colNum = thermalProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			thermalTimerList_prev[i] = thermalTimerList[i];
		}
	}
	if (hasContactorProtection == true) {

		colNum = contactorProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			contactorTimerList_prev[i] = contactorTimerList[i];
		}

	}
	if (hasEMSProtection == true) {

		colNum = emsProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			emsTimerList_prev[i] = emsTimerList[i];
		}

	}

	// Update reconnection timer
	relayTimerReconnect_prev = relayTimerReconnect;
	contactorTimerReconnect_prev = contactorTimerReconnect;
	emsTimerReconnect_prev = emsTimerReconnect;

}


//TPIM state variable updates - transition them
void motor::TPIMupdateVars() {

	int colNum;

	phips_prev = phips;
	phins_cj_prev = phins_cj;
	phipr_prev = phipr;
	phinr_cj_prev = phinr_cj;
	wr_pu_prev = wr_pu;
	Ips_prev = Ips;
	Ipr_prev = Ipr;
	Ins_cj_prev = Ins_cj;
	Inr_cj_prev = Inr_cj;

	reconnect_prev = reconnect;
	motor_trip_prev = motor_trip;
	trip_prev = trip;

	// Update timer for protection
	if (hasRelayProtection == true) {

		colNum = relayProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			relayTimerList_prev[i] = relayTimerList[i];
		}

	}
	if (hasOverLoadProtection == true) {

		colNum = overLoadProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			overLoadTimerList_prev[i] = overLoadTimerList[i];
		}
	}
	if (hasThermalProtection == true) {

		colNum = thermalProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			thermalTimerList_prev[i] = thermalTimerList[i];
		}
	}
	if (hasContactorProtection == true) {

		colNum = contactorProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			contactorTimerList_prev[i] = contactorTimerList[i];
		}

	}
	if (hasEMSProtection == true) {

		colNum = emsProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			emsTimerList_prev[i] = emsTimerList[i];
		}

	}

	// Update reconnection timer
	relayTimerReconnect_prev = relayTimerReconnect;
	contactorTimerReconnect_prev = contactorTimerReconnect;
	emsTimerReconnect_prev = emsTimerReconnect;

}

// function to reinitialize values for the motor model
void motor::SPIMreinitializeVars() {

	int colNum;

	wr = wr_prev;
	Telec = Telec_prev; 
	psi_dr = psi_dr_prev;
	psi_qr = psi_qr_prev;
	psi_f = psi_f_prev; 
	psi_b = psi_b_prev;
	Iqs = Iqs_prev;
	Ids =Ids_prev;
	If = If_prev;
	Ib = Ib_prev;
	Is = Is_prev;
	motor_elec_power = motor_elec_power_prev;
	psi_sat = psi_sat_prev;

	reconnect = reconnect_prev;
	motor_trip = motor_trip_prev;
	trip = trip_prev;

	// Update timer for protection
	if (hasRelayProtection == true) {

		colNum = relayProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			relayTimerList[i] = relayTimerList_prev[i];
		}

	}
	if (hasOverLoadProtection == true) {

		colNum = overLoadProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			overLoadTimerList[i] = overLoadTimerList_prev[i];
		}
	}
	if (hasThermalProtection == true) {

		colNum = thermalProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			thermalTimerList[i] = thermalTimerList_prev[i];
		}
	}
	if (hasContactorProtection == true) {

		colNum = contactorProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			contactorTimerList[i] = contactorTimerList_prev[i];
		}

	}
	if (hasEMSProtection == true) {

		colNum = emsProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			emsTimerList[i] = emsTimerList_prev[i];
		}

	}

	// Update reconnection timer
	relayTimerReconnect = relayTimerReconnect_prev;
	contactorTimerReconnect = contactorTimerReconnect_prev;
	emsTimerReconnect = emsTimerReconnect_prev;
}

//TPIM initalization routine
void motor::TPIMreinitializeVars() {

	int colNum;

	phips = phips_prev;
	phins_cj = phins_cj_prev;
	phipr = phipr_prev;
	phinr_cj = phinr_cj_prev;
	wr_pu = wr_pu_prev;
	Ips = Ips_prev;
	Ipr = Ipr_prev;
	Ins_cj = Ins_cj_prev;
	Inr_cj = Inr_cj_prev;

	reconnect = reconnect_prev;
	motor_trip = motor_trip_prev;
	trip = trip_prev;

	// Update timer for protection
	if (hasRelayProtection == true) {

		colNum = relayProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			relayTimerList[i] = relayTimerList_prev[i];
		}

	}
	if (hasOverLoadProtection == true) {

		colNum = overLoadProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			overLoadTimerList[i] = overLoadTimerList_prev[i];
		}
	}
	if (hasThermalProtection == true) {

		colNum = thermalProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			thermalTimerList[i] = thermalTimerList_prev[i];
		}
	}
	if (hasContactorProtection == true) {

		colNum = contactorProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			contactorTimerList[i] = contactorTimerList_prev[i];
		}

	}
	if (hasEMSProtection == true) {

		colNum = emsProtectionTrip.get_cols();

		for (int i = 0; i < colNum; i++)
		{
			emsTimerList[i] = emsTimerList_prev[i];
		}

	}

	// Update reconnection timer
	relayTimerReconnect = relayTimerReconnect_prev;
	contactorTimerReconnect = contactorTimerReconnect_prev;
	emsTimerReconnect = emsTimerReconnect_prev;

}

// function to update the status of the motor
void motor::SPIMUpdateMotorStatus() {
	if (motor_override == overrideOFF || ws <= 1 || Vs.Mag() <= 0.1)
	{
		motor_status = statusOFF;
	}
	else if (wr > 1) {
		motor_status = statusRUNNING;
	}
	else if (motor_trip == 1) {
		motor_status = statusTRIPPED;
	}
	else {
		motor_status = statusSTALLED;
	}	
}

//TPIM status update
void motor::TPIMUpdateMotorStatus() {
	if (motor_override == overrideOFF || ws_pu <= 0.1 ||
			Vas.Mag() <= 0.1 || Vbs.Mag() <= 0.1 || Vcs.Mag() <= 0.1) {
		motor_status = statusOFF;
	}
	else if (wr_pu > 0.0) {
		motor_status = statusRUNNING;
	}
	else if (motor_trip == 1) {
		motor_status = statusTRIPPED;
	}
	else {
		motor_status = statusSTALLED;
	}
}

// function to update the protection of the motor
void motor::UpdateProtection(double delta_time) {

	unsigned int colNum;

	// Check each adopted protection type to update trip or reconnect time
	// If the motor is on, check whether to trip
	if (motor_override == overrideON && motor_trip == 0) {

		if (hasRelayProtection == true) {
			motorCheckTrip(delta_time, &(relayProtectionTrip), relayTimerList, relayTrip);
		}
		if (hasOverLoadProtection == true) {
			motorCheckTrip(delta_time, &(overLoadProtectionTrip), overLoadTimerList, overLoadTrip);
		}
		if (hasThermalProtection == true) {
			if (motor_op_mode == modeSPIM) {
				SPIM_CheckThermalTrip(delta_time, &thermalProtectionTrip,thermalTrip);
			}
			else {
				motorCheckTrip(delta_time, &(thermalProtectionTrip), thermalTimerList, thermalTrip);
			}
		}
		if (hasContactorProtection == true) {
			motorCheckTrip(delta_time, &(contactorProtectionTrip), contactorTimerList, contactorTrip);
		}
		if (hasEMSProtection == true) {
			motorCheckTrip(delta_time, &(emsProtectionTrip), emsTimerList, emsTrip);
		}

		// If either of the protection determines trip, then mark motor as trip status
		if (relayTrip || overLoadTrip || thermalTrip || contactorTrip || emsTrip) {

			// Identify motor as tripped
			motor_trip = 1;

			// Reinitialize the timer array of each protection
		    // Relay protection
		    if (hasRelayProtection == true) {
				//Zero it
				for (int i = 0; i < relayProtectionTrip.get_cols(); i++)
				{
					relayTimerList[i] = 0.0;	// Starts with 0 timer
				}
				// Make reconnection status flag as false;
				relayReconnect = false;
		    }

		    // Over load protection
		    if (hasOverLoadProtection == true) {
				//Zero it
				for (int i = 0; i < overLoadProtectionTrip.get_cols(); i++)
				{
					overLoadTimerList[i] = 0.0;	// Starts with 0 timer
				}
		    }

		    // Thermal protection
		    if (hasThermalProtection == true) {
				//Zero it
				for (int i = 0; i < thermalProtectionTrip.get_cols(); i++)
				{
					thermalTimerList[i] = 0.0;	// Starts with 0 timer
				}
		    }

		    // Contactor protection
		    if (hasContactorProtection == true) {
				//Zero it
				for (int i = 0; i < contactorProtectionTrip.get_cols(); i++)
				{
					contactorTimerList[i] = 0.0;	// Starts with 0 timer
				}

				// Make reconnection status flag as false;
				contactorReconnect = false;
		    }

		    // EMS protection
		    if (hasEMSProtection == true) {
				//Zero it
				for (int i = 0; i < emsProtectionTrip.get_cols(); i++)
				{
					emsTimerList[i] = 0.0;	// Starts with 0 timer
				}

				// Make reconnection status flag as false;
				emsReconnect = false;
		    }
		} // end motor trip identification, and reinitialization of timer array
	}
	// If the motor is off, need to check whether to reconnect
	else {
		// The reconnection of thermal and overload takes more than 5 minutes, thus not checked here
		if (hasRelayProtection == true && relayTrip == true) {
			motorCheckReconnect(delta_time, &(relayProtectionReconnect), relayTimerReconnect, relayReconnect);
		}
		if (hasContactorProtection == true && contactorTrip == true) {
			motorCheckReconnect(delta_time, &(contactorProtectionReconnect), contactorTimerReconnect, contactorReconnect);
		}
		if (hasEMSProtection == true && emsTrip == true) {
			motorCheckReconnect(delta_time, &(emsProtectionReconnect), emsTimerReconnect, emsReconnect);
		}
		if (relayReconnect || contactorReconnect || emsReconnect) {

			// Update motor trip and reconnect status
			motor_trip = 0;

			// Reinitialize protection reconnection timer
			relayTimerReconnect = 0.0;
			contactorTimerReconnect = 0.0;
			emsTimerReconnect = 0.0;

			// Reinitialize the trip status of each protection
		    // Realy protection
		    if (hasRelayProtection == true) {
		    	relayTrip = false;
		    }

		    // Over load protection
		    if (hasOverLoadProtection == true) {
				overLoadTrip = false;
		    }

		    // Thermal protection
		    if (hasThermalProtection == true) {
				thermalTrip = false;
		    }

		    // Contactor protection
		    if (hasContactorProtection == true) {
				contactorTrip = false;
		    }

		    // EMS protection
		    if (hasEMSProtection == true) {
				emsTrip = false;
		    }
		}
	}
}

// function to ensure that internal model states are zeros when the motor is OFF
// Should let speed and mechanical torque change slowly -- Zhigang Chu, 07/24/2018
void motor::SPIMStateOFF(double dTime) {
	psi_b = complex(0.0,0.0);
    psi_f = complex(0.0,0.0);
    psi_dr = complex(0.0,0.0); 
    psi_qr = complex(0.0,0.0); 
    Ids = complex(0.0,0.0);
    Iqs = complex(0.0,0.0);  
    If = complex(0.0,0.0);
    Ib = complex(0.0,0.0);
    Is = complex(0.0,0.0);
    motor_elec_power = complex(0.0,0.0);
    Telec = 0.0; 
//    Tmech_eff = 0.0;
//    wr = 0.0;
//	wr_pu = 0.0;
//	theta = 0.0;

    if (dTime != 0){ // Only update these values if delta time has proceeded
        // Change torque according to SPIM_type
        if (SPIM_type == SPIM_C) { // Constant torque
    		Tmech_eff = Tmech;
    	}
    	else if (SPIM_type == SPIM_S) { // Speed dependent torque
    		Tmech_eff = wr_pu * wr_pu *Tmech;
    	}
    	else { // Must be triangular torque
    		Tmech_eff = wr_pu * wr_pu *Tmech;
    		T_av = Tmech * avRatio;
    		double PHAME = PI; // Initial phase angle
    		double stroke = PI/2.0;
    		double twostr = 2.0*stroke;
    		double AAA = fmod((theta + PHAME), stroke);
    		double AAAA;
    		if (curr_delta_time >= t_DLD) {
    			if (fmod((theta + PHAME), twostr) < stroke) {
    				AAAA = T_av + T_av*(AAA - stroke/2.0)*2.0/stroke;
    			}
    			else {
    				AAAA = -T_av + T_av*(3.0*stroke/2.0 - AAA)*2.0/stroke;
    			}
    		}
    		else {
    			AAAA = 0;
    		}

    		Tmech_eff = Tmech_eff + AAAA;
    	}

        // speed equation
    	wr = wr + (((Telec-Tmech_eff)*wbase)/(2*H))*dTime;

    	// speeds below 0 should be avoided
    	if (wr < 0) {
    		wr = 0;
    	}

    	//Get the per-unit version
    	wr_pu = wr / wbase;

    	// Angle equation
    	theta = theta + wr*dTime;
    	theta = fmod(theta, (2.0*PI));
    }

}

//TPIM "zero-stating" item
void motor::TPIMStateOFF() {
	phips = complex(0.0,0.0);
	phins_cj = complex(0.0,0.0);
	phipr = complex(0.0,0.0);
	phinr_cj = complex(0.0,0.0);
	wr = 0.0;
	wr_pu = 0.0;
	Ips = complex(0.0,0.0);
	Ipr = complex(0.0,0.0);
	Ins_cj = complex(0.0,0.0);
	Inr_cj = complex(0.0,0.0);
	motor_elec_power = complex(0.0,0.0);
	Telec = 0.0;
	Tmech_eff = 0.0;
}

// Function to calculate the solution to the steady state SPIM model
void motor::SPIMSteadyState(TIMESTAMP t1) {
	double wr_delta = 1;
    psi_sat = 1;
	double psi = -1;
    int32 count = 1;
	double Xc = -1;

	while (fabs(wr_delta) > speed_error && count < iteration_count) {
        count++;
        
        //Kick in extra capacitor if we droop in speed
		if (wr < cap_run_speed) {
           Xc = Xc2;
		}
		else {
           Xc = Xc1; 
		}

		TF[0] = (complex(0.0,1.0)*Xd_prime*ws_pu) + Rds; 
		TF[1] = 0;
		TF[2] = (complex(0.0,1.0)*Xm*ws_pu)/Xr;
		TF[3] = (complex(0.0,1.0)*Xm*ws_pu)/Xr;
		TF[4] = 0;
		TF[5] = (complex(0.0,1.0)*Xc)/ws_pu + (complex(0.0,1.0)*Xq_prime*ws_pu) + Rqs;
		TF[6] = -(Xm*n*ws_pu)/Xr;
		TF[7] = (Xm*n*ws_pu)/Xr;
		TF[8] = Xm/2;
		TF[9] =  -(complex(0.0,1.0)*Xm*n)/2;
		TF[10] = (complex(0.0,1.0)*wr - complex(0.0,1.0)*ws)*To_prime -psi_sat;
		TF[11] = 0;
		TF[12] = Xm/2;
		TF[13] = (complex(0.0,1.0)*Xm*n)/2;
		TF[14] = 0;
		TF[15] = (complex(0.0,1.0)*wr + complex(0.0,1.0)*ws)*-To_prime -psi_sat ;

		// big matrix solving winding currents and fluxes
		invertMatrix(TF,ITF);
		Ids = ITF[0]*Vs.Mag() + ITF[1]*Vs.Mag();
		Iqs = ITF[4]*Vs.Mag() + ITF[5]*Vs.Mag();
		psi_f = ITF[8]*Vs.Mag() + ITF[9]*Vs.Mag();
		psi_b = ITF[12]*Vs.Mag() + ITF[13]*Vs.Mag();
		If = (Ids-(complex(0.0,1.0)*n*Iqs))*0.5;
		Ib = (Ids+(complex(0.0,1.0)*n*Iqs))*0.5;

        //electrical torque 
		Telec = (Xm/Xr)*2.0*(If.Im()*psi_f.Re() - If.Re()*psi_f.Im() - Ib.Im()*psi_b.Re() + Ib.Re()*psi_b.Im()); 

		// Change torque according to SPIM_type
			if (SPIM_type == SPIM_C) { // Constant torque
				Tmech_eff = Tmech;
			}
			else if (SPIM_type == SPIM_S) { // Speed dependent torque
				Tmech_eff = wr_pu * wr_pu *Tmech;
			}
			else { // Must be triangular torque
				Tmech_eff = wr_pu * wr_pu *Tmech;
				T_av = Tmech * avRatio;
				double PHAME = PI; // Initial phase angle
				double stroke = PI/2.0;
				double twostr = 2.0*stroke;
				double AAA = fmod((theta + PHAME), stroke);
				double AAAA;
				if (curr_delta_time >= t_DLD) {
					if (fmod((theta + PHAME), twostr) < stroke) {
						AAAA = T_av + T_av*(AAA - stroke/2.0)*2.0/stroke;
					}
					else {
						AAAA = -T_av + T_av*(3.0*stroke/2.0 - AAA)*2.0/stroke;
					}
				}
				else {
					AAAA = 0;
				}

				Tmech_eff = Tmech_eff + AAAA;
			}

        //calculate speed deviation 
        wr_delta = Telec-Tmech_eff;

        //Calculate saturated flux
        psi = sqrt(pow(psi_f.Re(),2)+pow(psi_f.Im(),2)+pow(psi_b.Re(),2)+pow(psi_b.Im(),2));
		if(psi<=bsat) {
            psi_sat = 1;
		}
		else {
            psi_sat = 1 + Asat*(pow(psi-bsat,2));
		}

        //update the rotor speed
		if (wr+wr_delta > 0) {
			wr = wr+wr_delta;
			wr_pu = wr/wbase;
		}
		else {
			wr = 0;
			wr_pu = 0.0;
		}
	}
    
    psi_dr = psi_f + psi_b;
    psi_qr = complex(0.0,1.0)*psi_f + complex(0,-1)*psi_b;
    // system current and power equations
    if (Is.Re() == -999.9 && Is.Im() == -999.9) {
    	complex Is_init = (Ids + Iqs)*complex_exp(Vs.Arg());
    	temperature_SPIM = Is_init.Mag() * Is_init.Mag() * R_stall;
    }
    Is = (Ids + Iqs)*complex_exp(Vs.Arg());
    motor_elec_power = Vs * ~Is * Pbase; // VA;
}


//TPIM steady state function
void motor::TPIMSteadyState(TIMESTAMP t1) {
		double omgr0_delta = 1;
		int32 count = 1;
		complex alpha = complex(0.0,0.0);
		complex A1, A2, A3, A4;
		complex B1, B2, B3, B4;
		complex C1, C2, C3, C4;
		complex D1, D2, D3, D4;
		complex E1, E2, E3, E4;
		complex Vap;
		complex Van;
		// complex Vaz;

		alpha = complex_exp(2.0/3.0 * PI);

        Vap = (Vas + alpha * Vbs + alpha * alpha * Vcs) / 3.0;
        Van = (Vas + alpha * alpha * Vbs + alpha * Vcs) / 3.0;
        // Vaz = 1.0/3.0 * (Vas + Vbs + Vcs);

        // printf("Enter steady state: \n");

        if (motor_status == statusRUNNING){

			while (fabs(omgr0_delta) > speed_error && count < iteration_count) {
				count++;

				// pre-calculate complex coefficients of the 4 linear equations associated with flux state variables
				A1 = -(complex(0.0,1.0) * ws_pu + rs_pu / sigma1) ;
				B1 =  0.0 ;
				C1 =  rs_pu / sigma1 * lm / Lr ;
				D1 =  0.0 ;
				E1 = Vap ;

				A2 = 0.0;
				B2 = -(complex(0.0,-1.0) * ws_pu + rs_pu / sigma1) ;
				C2 = 0.0 ;
				D2 = rs_pu / sigma1 * lm / Lr ;
				E2 = ~Van ;

				A3 = rr_pu / sigma2 * lm / Ls ;
				B3 =  0.0 ;
				C3 =  -(complex(0.0,1.0) * (ws_pu - wr_pu) + rr_pu / sigma2) ;
				D3 =  0.0 ;
				E3 =  0.0 ;

				A4 = 0.0 ;
				B4 =  rr_pu / sigma2 * lm / Ls ;
				C4 = 0.0 ;
				D4 = -(complex(0.0,1.0) * (-ws_pu - wr_pu) + rr_pu / sigma2) ;
				E4 =  0.0 ;

				// solve the 4 linear equations to obtain 4 flux state variables
				phips = (B1*C2*D3*E4 - B1*C2*D4*E3 - B1*C3*D2*E4 + B1*C3*D4*E2 + B1*C4*D2*E3
					- B1*C4*D3*E2 - B2*C1*D3*E4 + B2*C1*D4*E3 + B2*C3*D1*E4 - B2*C3*D4*E1
					- B2*C4*D1*E3 + B2*C4*D3*E1 + B3*C1*D2*E4 - B3*C1*D4*E2 - B3*C2*D1*E4
					+ B3*C2*D4*E1 + B3*C4*D1*E2 - B3*C4*D2*E1 - B4*C1*D2*E3 + B4*C1*D3*E2
					+ B4*C2*D1*E3 - B4*C2*D3*E1 - B4*C3*D1*E2 + B4*C3*D2*E1)/
					(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
					A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
					A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
					A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
					A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

				phins_cj =  -(A1*C2*D3*E4 - A1*C2*D4*E3 - A1*C3*D2*E4 + A1*C3*D4*E2 + A1*C4*D2*E3
					- A1*C4*D3*E2 - A2*C1*D3*E4 + A2*C1*D4*E3 + A2*C3*D1*E4 - A2*C3*D4*E1
					- A2*C4*D1*E3 + A2*C4*D3*E1 + A3*C1*D2*E4 - A3*C1*D4*E2 - A3*C2*D1*E4
					+ A3*C2*D4*E1 + A3*C4*D1*E2 - A3*C4*D2*E1 - A4*C1*D2*E3 + A4*C1*D3*E2
					+ A4*C2*D1*E3 - A4*C2*D3*E1 - A4*C3*D1*E2 + A4*C3*D2*E1)/
					(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
					A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
					A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
					A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
					A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

				phipr = (A1*B2*D3*E4 - A1*B2*D4*E3 - A1*B3*D2*E4 + A1*B3*D4*E2 + A1*B4*D2*E3
					- A1*B4*D3*E2 - A2*B1*D3*E4 + A2*B1*D4*E3 + A2*B3*D1*E4 - A2*B3*D4*E1
					- A2*B4*D1*E3 + A2*B4*D3*E1 + A3*B1*D2*E4 - A3*B1*D4*E2 - A3*B2*D1*E4
					+ A3*B2*D4*E1 + A3*B4*D1*E2 - A3*B4*D2*E1 - A4*B1*D2*E3 + A4*B1*D3*E2
					+ A4*B2*D1*E3 - A4*B2*D3*E1 - A4*B3*D1*E2 + A4*B3*D2*E1)/
					(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
					A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
					A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
					A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
					A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

				phinr_cj = -(A1*B2*C3*E4 - A1*B2*C4*E3 - A1*B3*C2*E4 + A1*B3*C4*E2 + A1*B4*C2*E3
					- A1*B4*C3*E2 - A2*B1*C3*E4 + A2*B1*C4*E3 + A2*B3*C1*E4 - A2*B3*C4*E1
					- A2*B4*C1*E3 + A2*B4*C3*E1 + A3*B1*C2*E4 - A3*B1*C4*E2 - A3*B2*C1*E4
					+ A3*B2*C4*E1 + A3*B4*C1*E2 - A3*B4*C2*E1 - A4*B1*C2*E3 + A4*B1*C3*E2
					+ A4*B2*C1*E3 - A4*B2*C3*E1 - A4*B3*C1*E2 + A4*B3*C2*E1)/
					(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
					A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
					A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
					A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
					A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

				Ips = (phips - phipr * lm / Lr) / sigma1;  // pu
				Ipr = (phipr - phips * lm / Ls) / sigma2;  // pu
				Ins_cj = (phins_cj - phinr_cj * lm / Lr) / sigma1; // pu
				Inr_cj = (phinr_cj - phins_cj * lm / Ls) / sigma2; // pu
				Telec = (~phips * Ips + ~phins_cj * Ins_cj).Im() ;  // pu

				// iteratively compute speed increment to make sure Telec matches Tmech during steady state mode
				// if it does not match, then update current and Telec using new wr_pu
				omgr0_delta = ( Telec - Tmech_eff ) / ((double)iteration_count);

				//update the rotor speed to make sure electrical torque traces mechanical torque
				if (wr_pu + omgr0_delta > -10) {
					wr_pu = wr_pu + omgr0_delta;
					wr = wr_pu * wbase;
				}
				else {
					wr_pu = -10;
					wr = wr_pu * wbase;
				}

			}  // End while
        } // End if TPIM is assumed running
        else // must be the TPIM stalled or otherwise not running
        {
        	wr_pu = 0.0;
			wr = 0.0;

			// pre-calculate complex coefficients of the 4 linear equations associated with flux state variables
			A1 = -(complex(0.0,1.0) * ws_pu + rs_pu / sigma1) ;
			B1 =  0.0 ;
			C1 =  rs_pu / sigma1 * lm / Lr ;
			D1 =  0.0 ;
			E1 = Vap ;

			A2 = 0.0;
			B2 = -(complex(0.0,-1.0) * ws_pu + rs_pu / sigma1) ;
			C2 = 0.0 ;
			D2 = rs_pu / sigma1 * lm / Lr ;
			E2 = ~Van ;

			A3 = rr_pu / sigma2 * lm / Ls ;
			B3 =  0.0 ;
			C3 =  -(complex(0.0,1.0) * (ws_pu - wr_pu) + rr_pu / sigma2) ;
			D3 =  0.0 ;
			E3 =  0.0 ;

			A4 = 0.0 ;
			B4 =  rr_pu / sigma2 * lm / Ls ;
			C4 = 0.0 ;
			D4 = -(complex(0.0,1.0) * (-ws_pu - wr_pu) + rr_pu / sigma2) ;
			E4 =  0.0 ;

			// solve the 4 linear equations to obtain 4 flux state variables
			phips = (B1*C2*D3*E4 - B1*C2*D4*E3 - B1*C3*D2*E4 + B1*C3*D4*E2 + B1*C4*D2*E3
				- B1*C4*D3*E2 - B2*C1*D3*E4 + B2*C1*D4*E3 + B2*C3*D1*E4 - B2*C3*D4*E1
				- B2*C4*D1*E3 + B2*C4*D3*E1 + B3*C1*D2*E4 - B3*C1*D4*E2 - B3*C2*D1*E4
				+ B3*C2*D4*E1 + B3*C4*D1*E2 - B3*C4*D2*E1 - B4*C1*D2*E3 + B4*C1*D3*E2
				+ B4*C2*D1*E3 - B4*C2*D3*E1 - B4*C3*D1*E2 + B4*C3*D2*E1)/
				(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
				A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
				A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
				A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
				A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

			phins_cj =  -(A1*C2*D3*E4 - A1*C2*D4*E3 - A1*C3*D2*E4 + A1*C3*D4*E2 + A1*C4*D2*E3
				- A1*C4*D3*E2 - A2*C1*D3*E4 + A2*C1*D4*E3 + A2*C3*D1*E4 - A2*C3*D4*E1
				- A2*C4*D1*E3 + A2*C4*D3*E1 + A3*C1*D2*E4 - A3*C1*D4*E2 - A3*C2*D1*E4
				+ A3*C2*D4*E1 + A3*C4*D1*E2 - A3*C4*D2*E1 - A4*C1*D2*E3 + A4*C1*D3*E2
				+ A4*C2*D1*E3 - A4*C2*D3*E1 - A4*C3*D1*E2 + A4*C3*D2*E1)/
				(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
				A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
				A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
				A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
				A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

			phipr = (A1*B2*D3*E4 - A1*B2*D4*E3 - A1*B3*D2*E4 + A1*B3*D4*E2 + A1*B4*D2*E3
				- A1*B4*D3*E2 - A2*B1*D3*E4 + A2*B1*D4*E3 + A2*B3*D1*E4 - A2*B3*D4*E1
				- A2*B4*D1*E3 + A2*B4*D3*E1 + A3*B1*D2*E4 - A3*B1*D4*E2 - A3*B2*D1*E4
				+ A3*B2*D4*E1 + A3*B4*D1*E2 - A3*B4*D2*E1 - A4*B1*D2*E3 + A4*B1*D3*E2
				+ A4*B2*D1*E3 - A4*B2*D3*E1 - A4*B3*D1*E2 + A4*B3*D2*E1)/
				(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
				A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
				A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
				A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
				A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

			phinr_cj = -(A1*B2*C3*E4 - A1*B2*C4*E3 - A1*B3*C2*E4 + A1*B3*C4*E2 + A1*B4*C2*E3
				- A1*B4*C3*E2 - A2*B1*C3*E4 + A2*B1*C4*E3 + A2*B3*C1*E4 - A2*B3*C4*E1
				- A2*B4*C1*E3 + A2*B4*C3*E1 + A3*B1*C2*E4 - A3*B1*C4*E2 - A3*B2*C1*E4
				+ A3*B2*C4*E1 + A3*B4*C1*E2 - A3*B4*C2*E1 - A4*B1*C2*E3 + A4*B1*C3*E2
				+ A4*B2*C1*E3 - A4*B2*C3*E1 - A4*B3*C1*E2 + A4*B3*C2*E1)/
				(A1*B2*C3*D4 - A1*B2*C4*D3 - A1*B3*C2*D4 + A1*B3*C4*D2 + A1*B4*C2*D3 -
				A1*B4*C3*D2 - A2*B1*C3*D4 + A2*B1*C4*D3 + A2*B3*C1*D4 - A2*B3*C4*D1 -
				A2*B4*C1*D3 + A2*B4*C3*D1 + A3*B1*C2*D4 - A3*B1*C4*D2 - A3*B2*C1*D4 +
				A3*B2*C4*D1 + A3*B4*C1*D2 - A3*B4*C2*D1 - A4*B1*C2*D3 + A4*B1*C3*D2 +
				A4*B2*C1*D3 - A4*B2*C3*D1 - A4*B3*C1*D2 + A4*B3*C2*D1) ;

			Ips = (phips - phipr * lm / Lr) / sigma1;  // pu
			Ipr = (phipr - phips * lm / Ls) / sigma2;  // pu
			Ins_cj = (phins_cj - phinr_cj * lm / Lr) / sigma1; // pu
			Inr_cj = (phinr_cj - phins_cj * lm / Ls) / sigma2; // pu
			Telec = (~phips * Ips + ~phins_cj * Ins_cj).Im() ;  // pu
        }

		// system current and power equations
        Ias = Ips + ~Ins_cj ;// pu
        Ibs = alpha * alpha * Ips + alpha * ~Ins_cj ; // pu
        Ics = alpha * Ips + alpha * alpha * ~Ins_cj ; // pu

		motor_elec_power = (Vap * ~Ips + Van * Ins_cj) * Pbase; // VA

}

// Function to calculate the solution to the steady state SPIM model
//void motor::SPIMDynamic(double curr_delta_time, double dTime) {
void motor::SPIMDynamic(double dTime) {
	double psi = -1;
	double Xc = -1;
    
    //Kick in extra capacitor if we droop in speed
	if (wr < cap_run_speed) {
       Xc = Xc2;
	}
	else {
       Xc = Xc1; 
	}

    // Flux equation
	psi_b = (Ib*Xm) / ((complex(0.0,1.0)*(ws+wr)*To_prime)+psi_sat);
	psi_f = psi_f + ( If*(Xm/To_prime) - (complex(0.0,1.0)*(ws-wr) + psi_sat/To_prime)*psi_f )*dTime;   

    //Calculate saturated flux
	psi = sqrt(psi_f.Re()*psi_f.Re() + psi_f.Im()*psi_f.Im() + psi_b.Re()*psi_b.Re() + psi_b.Im()*psi_b.Im());
	if(psi<=bsat) {
        psi_sat = 1;
	}
	else {
        psi_sat = 1 + Asat*((psi-bsat)*(psi-bsat));
	}   

	// Calculate d and q axis fluxes
	psi_dr = psi_f + psi_b;
	psi_qr = complex(0.0,1.0)*psi_f + complex(0,-1)*psi_b;

	// d and q-axis current equations
	Ids = (-(complex(0.0,1.0)*ws_pu*(Xm/Xr)*psi_dr) + Vs.Mag()) / ((complex(0.0,1.0)*ws_pu*Xd_prime)+Rds);  
	Iqs = (-(complex(0.0,1.0)*ws_pu*(n*Xm/Xr)*psi_qr) + Vs.Mag()) / ((complex(0.0,1.0)*ws_pu*Xq_prime)+(complex(0.0,1.0)/ws_pu*Xc)+Rqs); 

	// f and b current equations
	If = (Ids-(complex(0.0,1.0)*n*Iqs))*0.5;
	Ib = (Ids+(complex(0.0,1.0)*n*Iqs))*0.5;

	// system current and power equations
	Is = (Ids + Iqs)*complex_exp(Vs.Arg());
	motor_elec_power = Vs * ~Is * Pbase; // VA;

	// SPIM temperature integration
	double dT_dt = (Is.Mag() * Is.Mag() * R_stall - temperature_SPIM) / Tth;
	if (Is.Mag() > 5) {
		printf("check");
	}
	temperature_SPIM = temperature_SPIM + dT_dt * dTime;

    //electrical torque 
	Telec = (Xm/Xr)*2*(If.Im()*psi_f.Re() - If.Re()*psi_f.Im() - Ib.Im()*psi_b.Re() + Ib.Re()*psi_b.Im()); 

	// Change torque according to SPIM_type
	if (SPIM_type == SPIM_C) { // Constant torque
		Tmech_eff = Tmech;
	}
	else if (SPIM_type == SPIM_S) { // Speed dependent torque
		Tmech_eff = wr_pu * wr_pu *Tmech;
	}
	else { // Must be triangular torque
		Tmech_eff = wr_pu * wr_pu *Tmech;
		T_av = Tmech * avRatio;
		double PHAME = PI; // Initial phase angle
		double stroke = PI/2.0;
		double twostr = 2.0*stroke;
		double AAA = fmod((theta + PHAME), stroke);
		double AAAA;
		if (curr_delta_time >= t_DLD) {
			if (fmod((theta + PHAME), twostr) < stroke) {
				AAAA = T_av + T_av*(AAA - stroke/2.0)*2.0/stroke;
			}
			else {
				AAAA = -T_av + T_av*(3.0*stroke/2.0 - AAA)*2.0/stroke;
			}
		}
		else {
			AAAA = 0;
		}

		Tmech_eff = Tmech_eff + AAAA;
	}

	// speed equation 
	//wr = wr + (((Telec-Tmech)*wbase)/(2*H))*dTime;
	wr = wr + (((Telec-Tmech_eff)*wbase)/(2*H))*dTime;

    // speeds below 0 should be avoided
	if (wr < 0) {
		wr = 0;
	}

	//Get the per-unit version
	wr_pu = wr / wbase;

	theta = theta + wr*dTime;
	theta = fmod(theta, (2.0*PI));
}

//Dynamic updates for TPIM
void motor::TPIMDynamic(double curr_delta_time, double dTime) {
	complex alpha = complex(0.0,0.0);
	complex Vap;
	complex Van;

	// variables related to predictor step
	complex A1p, C1p;
	complex B2p, D2p;
	complex A3p, C3p;
	complex B4p, D4p;
	complex dphips_prev_dt ;
	complex dphins_cj_prev_dt ;
	complex dphipr_prev_dt ;
	complex dphinr_cj_prev_dt ;
	complex domgr0_prev_dt ;

	// variables related to corrector step
	complex A1c, C1c;
	complex B2c, D2c;
	complex A3c, C3c;
	complex B4c, D4c;
	complex dphips_dt ;
	complex dphins_cj_dt ;
	complex dphipr_dt ;
	complex dphinr_cj_dt ;
	complex domgr0_dt ;

	alpha = complex_exp(2.0/3.0 * PI);

    Vap = (Vas + alpha * Vbs + alpha * alpha * Vcs) / 3.0;
    Van = (Vas + alpha * alpha * Vbs + alpha * Vcs) / 3.0;

//    TPIMupdateVars(); // repeated thus removed here

//	if (wr_pu >= 1.0)
//	{
//		Tmech_eff = Tmech;
//	}

	if (TPIM_type == TPIM_A) {
		Tmech_eff = Tmech;
	}
	else if (TPIM_type == TPIM_B || TPIM_type == TPIM_C) {
		Tmech_eff = wr_pu * wr_pu *Tmech;
	}

    //*** Predictor Step ***//
    // predictor step 1 - calculate coefficients
    A1p = -(complex(0.0,1.0) * ws_pu + rs_pu / sigma1) ;
    C1p =  rs_pu / sigma1 * lm / Lr ;

    B2p = -(complex(0.0,-1.0) * ws_pu + rs_pu / sigma1) ;
    D2p = rs_pu / sigma1 *lm / Lr ;

    A3p = rr_pu / sigma2 * lm / Ls ;
    C3p =  -(complex(0.0,1.0) * (ws_pu - wr_pu_prev) + rr_pu / sigma2) ;

    B4p =  rr_pu / sigma2 * lm / Ls ;
    D4p = -(complex(0.0,1.0) * (-ws_pu - wr_pu_prev) + rr_pu / sigma2) ;

    // predictor step 2 - calculate derivatives
    dphips_prev_dt =  ( Vap + A1p * phips_prev + C1p * phipr_prev ) * wbase;  // pu/s
    dphins_cj_prev_dt = ( ~Van + B2p * phins_cj_prev + D2p * phinr_cj_prev ) * wbase; // pu/s
    dphipr_prev_dt  =  ( C3p * phipr_prev + A3p * phips_prev ) * wbase; // pu/s
    dphinr_cj_prev_dt = ( D4p * phinr_cj_prev  + B4p * phins_cj_prev ) * wbase; // pu/s
    domgr0_prev_dt =  ( (~phips_prev * Ips_prev + ~phins_cj_prev * Ins_cj_prev).Im() - Tmech_eff - Kfric * wr_pu_prev ) / (2.0 * H); // pu/s


    // predictor step 3 - integrate for predicted state variable
    phips = phips_prev +  dphips_prev_dt * dTime;
    phins_cj = phins_cj_prev + dphins_cj_prev_dt * dTime;
    phipr = phipr_prev + dphipr_prev_dt * dTime ;
    phinr_cj = phinr_cj_prev + dphinr_cj_prev_dt * dTime ;
    wr_pu = wr_pu_prev + domgr0_prev_dt.Re() * dTime ;
	wr = wr_pu * wbase;

    // predictor step 4 - update outputs using predicted state variables
    Ips = (phips - phipr * lm / Lr) / sigma1;  // pu
    Ipr = (phipr - phips * lm / Ls) / sigma2;  // pu
    Ins_cj = (phins_cj - phinr_cj * lm / Lr) / sigma1 ; // pu
    Inr_cj = (phinr_cj - phins_cj * lm / Ls) / sigma2; // pu


    //*** Corrector Step ***//
    // assuming no boundary variable (e.g. voltage) changes during each time step,
    // so predictor and corrector steps are placed in the same class function

    // corrector step 1 - calculate coefficients using predicted state variables
    A1c = -(complex(0.0,1.0) * ws_pu + rs_pu / sigma1) ;
    C1c =  rs_pu / sigma1 * lm / Lr ;

    B2c = -(complex(0.0,-1.0) * ws_pu + rs_pu / sigma1) ;
    D2c = rs_pu / sigma1 *lm / Lr ;

    A3c = rr_pu / sigma2 * lm / Ls ;
    C3c =  -(complex(0.0,1.0) * (ws_pu - wr_pu) + rr_pu / sigma2) ;  // This coeff. is different from predictor

    B4c =  rr_pu / sigma2 * lm / Ls ;
    D4c = -(complex(0.0,1.0) * (-ws_pu - wr_pu) + rr_pu / sigma2) ; // This coeff. is different from predictor

    // corrector step 2 - calculate derivatives
    dphips_dt =  ( Vap + A1c * phips + C1c * phipr ) * wbase;
    dphins_cj_dt = ( ~Van + B2c * phins_cj + D2c * phinr_cj ) * wbase ;
    dphipr_dt  =  ( C3c * phipr + A3c * phips ) * wbase;
    dphinr_cj_dt = ( D4c * phinr_cj  + B4c * phins_cj ) * wbase;
    domgr0_dt =  1.0/(2.0 * H) * ( (~phips * Ips + ~phins_cj * Ins_cj).Im() - Tmech_eff - Kfric * wr_pu );

    // corrector step 3 - integrate
    phips = phips_prev +  (dphips_prev_dt + dphips_dt) * dTime/2.0;
    phins_cj = phins_cj_prev + (dphins_cj_prev_dt + dphins_cj_dt) * dTime/2.0;
    phipr = phipr_prev + (dphipr_prev_dt + dphipr_dt) * dTime/2.0 ;
    phinr_cj = phinr_cj_prev + (dphinr_cj_prev_dt + dphinr_cj_dt) * dTime/2.0 ;
    wr_pu = wr_pu_prev + (domgr0_prev_dt + domgr0_dt).Re() * dTime/2.0 ;
	wr = wr_pu * wbase;

	if (wr_pu < -10.0) { // speeds below -10 should be avoided
		wr_pu = -10.0;
		wr = wr_pu * wbase;
	}

    // corrector step 4 - update outputs
    Ips = (phips - phipr * lm / Lr) / sigma1;  // pu
    Ipr = (phipr - phips * lm / Ls) / sigma2;  // pu
    Ins_cj = (phins_cj - phinr_cj * lm / Lr) / sigma1; // pu
    Inr_cj = (phinr_cj - phins_cj * lm / Ls) / sigma2; // pu
    Telec = (~phips * Ips + ~phins_cj * Ins_cj).Im() ;  // pu

	// update system current and power equations
    Ias = Ips + ~Ins_cj ;// pu
    Ibs = alpha * alpha * Ips + alpha * ~Ins_cj ; // pu
    Ics = alpha * Ips + alpha * alpha * ~Ins_cj ; // pu

	motor_elec_power = (Vap * ~Ips + Van * Ins_cj) * Pbase; // VA

}

//Function to perform exp(j*val) (basically a complex rotation)
complex motor::complex_exp(double angle)
{
	complex output_val;

	//exp(jx) = cos(x)+j*sin(x)
	output_val = complex(cos(angle),sin(angle));

	return output_val;
}

// Function to do the inverse of a 4x4 matrix
int motor::invertMatrix(complex TF[16], complex ITF[16])
{
    complex inv[16], det;
    int i;

    inv[0] = TF[5]  * TF[10] * TF[15] - TF[5]  * TF[11] * TF[14] - TF[9]  * TF[6]  * TF[15] + TF[9]  * TF[7]  * TF[14] +TF[13] * TF[6]  * TF[11] - TF[13] * TF[7]  * TF[10];
    inv[4] = -TF[4]  * TF[10] * TF[15] + TF[4]  * TF[11] * TF[14] + TF[8]  * TF[6]  * TF[15] - TF[8]  * TF[7]  * TF[14] - TF[12] * TF[6]  * TF[11] + TF[12] * TF[7]  * TF[10];
    inv[8] = TF[4]  * TF[9] * TF[15] - TF[4]  * TF[11] * TF[13] - TF[8]  * TF[5] * TF[15] + TF[8]  * TF[7] * TF[13] + TF[12] * TF[5] * TF[11] - TF[12] * TF[7] * TF[9];
    inv[12] = -TF[4]  * TF[9] * TF[14] + TF[4]  * TF[10] * TF[13] +TF[8]  * TF[5] * TF[14] - TF[8]  * TF[6] * TF[13] - TF[12] * TF[5] * TF[10] + TF[12] * TF[6] * TF[9];
    inv[1] = -TF[1]  * TF[10] * TF[15] + TF[1]  * TF[11] * TF[14] + TF[9]  * TF[2] * TF[15] - TF[9]  * TF[3] * TF[14] - TF[13] * TF[2] * TF[11] + TF[13] * TF[3] * TF[10];
    inv[5] = TF[0]  * TF[10] * TF[15] - TF[0]  * TF[11] * TF[14] - TF[8]  * TF[2] * TF[15] + TF[8]  * TF[3] * TF[14] + TF[12] * TF[2] * TF[11] - TF[12] * TF[3] * TF[10];
    inv[9] = -TF[0]  * TF[9] * TF[15] + TF[0]  * TF[11] * TF[13] + TF[8]  * TF[1] * TF[15] - TF[8]  * TF[3] * TF[13] - TF[12] * TF[1] * TF[11] + TF[12] * TF[3] * TF[9];
    inv[13] = TF[0]  * TF[9] * TF[14] - TF[0]  * TF[10] * TF[13] - TF[8]  * TF[1] * TF[14] + TF[8]  * TF[2] * TF[13] + TF[12] * TF[1] * TF[10] - TF[12] * TF[2] * TF[9];
    inv[2] = TF[1]  * TF[6] * TF[15] - TF[1]  * TF[7] * TF[14] - TF[5]  * TF[2] * TF[15] + TF[5]  * TF[3] * TF[14] + TF[13] * TF[2] * TF[7] - TF[13] * TF[3] * TF[6];
    inv[6] = -TF[0]  * TF[6] * TF[15] + TF[0]  * TF[7] * TF[14] + TF[4]  * TF[2] * TF[15] - TF[4]  * TF[3] * TF[14] - TF[12] * TF[2] * TF[7] + TF[12] * TF[3] * TF[6];
    inv[10] = TF[0]  * TF[5] * TF[15] - TF[0]  * TF[7] * TF[13] - TF[4]  * TF[1] * TF[15] + TF[4]  * TF[3] * TF[13] + TF[12] * TF[1] * TF[7] - TF[12] * TF[3] * TF[5];
    inv[14] = -TF[0]  * TF[5] * TF[14] + TF[0]  * TF[6] * TF[13] + TF[4]  * TF[1] * TF[14] - TF[4]  * TF[2] * TF[13] - TF[12] * TF[1] * TF[6] + TF[12] * TF[2] * TF[5];
    inv[3] = -TF[1] * TF[6] * TF[11] + TF[1] * TF[7] * TF[10] + TF[5] * TF[2] * TF[11] - TF[5] * TF[3] * TF[10] - TF[9] * TF[2] * TF[7] + TF[9] * TF[3] * TF[6];
    inv[7] = TF[0] * TF[6] * TF[11] - TF[0] * TF[7] * TF[10] - TF[4] * TF[2] * TF[11] + TF[4] * TF[3] * TF[10] + TF[8] * TF[2] * TF[7] - TF[8] * TF[3] * TF[6];
    inv[11] = -TF[0] * TF[5] * TF[11] + TF[0] * TF[7] * TF[9] + TF[4] * TF[1] * TF[11] - TF[4] * TF[3] * TF[9] - TF[8] * TF[1] * TF[7] + TF[8] * TF[3] * TF[5];
    inv[15] = TF[0] * TF[5] * TF[10] - TF[0] * TF[6] * TF[9] - TF[4] * TF[1] * TF[10] + TF[4] * TF[2] * TF[9] + TF[8] * TF[1] * TF[6] - TF[8] * TF[2] * TF[5];

    det = TF[0] * inv[0] + TF[1] * inv[4] + TF[2] * inv[8] + TF[3] * inv[12];

    if (det == 0)
        return 0;

    det = complex(1,0) / det;

    for (i = 0; i < 16; i++)
        ITF[i] = inv[i] * det;

    return 1;
}

// Function to check whether trip is determined by each protection
void motor::motorCheckTrip(double delta_time, double_array* motorProtection, double* timerList, bool &tripStatus)
{
	int ct = motorProtection->get_cols();
	double *thresV;

	// Check if the trip condition is met
	for (int i = 0; i < ct; i++) {
		double *tripTime = motorProtection->get_addr(1, i);
		if (timerList[i] >= (*tripTime)) {
			tripStatus = true;
			return;
		}
	}

	// Since there is no trip currently, will iterate over all points of Volt-time curve
	for (int i = ct - 1; i >= 0; i--) {
		thresV = motorProtection->get_addr(0, i);
		// SPIM check
		if (motor_op_mode == modeSPIM)
		{
			if ((Vs.Mag()) > (*thresV)) {
				// If current voltage is larger than the voltage threshold of one point,
				// Directly reset the timer of the point, and the points with smaller voltage threshold
				for (int j = i; j >= 0; j--) {
					timerList[j] = 0.0;
				}
				break;
			}
			else {
				timerList[i] += delta_time;
			}
		}
		// TPIM check
		else {
			if ((Vas.Mag() > (*thresV)) && (Vbs.Mag() > (*thresV)) && (Vcs.Mag() > (*thresV))) {
				// If current voltage is larger than the voltage threshold of one point,
				// Directly reset the timer of the point, and the points with smaller voltage threshold
				for (int j = i; j >= 0; j--) {
					timerList[j] = 0.0;
				}
				break;
			}
			else {
				timerList[i] += delta_time;
			}
		}

	}

	return;
}

// Function to check thermal trip for SPIM
void motor::SPIM_CheckThermalTrip(double delta_time, double_array* motorProtection, bool &tripStatus)
{
	double *threshT = motorProtection->get_addr(0,0); // Threshold temperature

	// Check if the trip condition is met
	if (temperature_SPIM >= *threshT) {
		tripStatus = true;
	}

	return;
}

// Function to check whether reconnection is determined by each protection
void motor::motorCheckReconnect(double delta_time, double_array* motorReconnection, double &reconnectTimer, bool &reconnectStatus)
{
	double *reconnectV = motorReconnection->get_addr(0, 0);
	double *reconnectT = motorReconnection->get_addr(1, 0);

	// Check if needs reconnection
	if (reconnectTimer >= (*reconnectT)) {
		reconnectStatus = true;
		reconnectTimer = 0.0;
		return;
	}

	// Update reconnect timer
	// SPIM check
	if (motor_op_mode == modeSPIM)
	{
		if ((Vs.Mag()) > (*reconnectV)) {
			reconnectTimer += delta_time;
		}
		else {
			reconnectTimer = 0.0;
		}
	}
	// TPIM check
	else {
		if ((Vas.Mag() > (*reconnectV)) && (Vbs.Mag() > (*reconnectV)) && (Vcs.Mag() > (*reconnectV))) {
			reconnectTimer += delta_time;
		}
		else {
			reconnectTimer = 0.0;
		}
	}
	reconnect = reconnectTimer;
	reconnect_time = *reconnectT;
	return;
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE: motor
//////////////////////////////////////////////////////////////////////////

/**
* REQUIRED: allocate and initialize an object.
*
* @param obj a pointer to a pointer of the last object in the list
* @param parent a pointer to the parent of this object
* @return 1 for a successfully created object, 0 for error
*/
EXPORT int create_motor(OBJECT **obj, OBJECT *parent)
{
	try
	{
		*obj = gl_create_object(motor::oclass);
		if (*obj!=NULL)
		{
			motor *my = OBJECTDATA(*obj,motor);
			gl_set_parent(*obj,parent);
			return my->create();
		}
		else
			return 0;
	}
	CREATE_CATCHALL(motor);
}

/**
* Object initialization is called once after all object have been created
*
* @param obj a pointer to this object
* @return 1 on success, 0 on error
*/
EXPORT int init_motor(OBJECT *obj)
{
	try {
		motor *my = OBJECTDATA(obj,motor);
		return my->init(obj->parent);
	}
	INIT_CATCHALL(motor);
}

/**
* Sync is called when the clock needs to advance on the bottom-up pass (PC_BOTTOMUP)
*
* @param obj the object we are sync'ing
* @param t0 this objects current timestamp
* @param pass the current pass for this sync call
* @return t1, where t1>t0 on success, t1=t0 for retry, t1<t0 on failure
*/
EXPORT TIMESTAMP sync_motor(OBJECT *obj, TIMESTAMP t0, PASSCONFIG pass)
{
	TIMESTAMP t1 = TS_INVALID;
	motor *my = OBJECTDATA(obj,motor);
	try
	{
		switch (pass) {
		case PC_PRETOPDOWN:
			t1 = my->presync(obj->clock,t0);
			break;
		case PC_BOTTOMUP:
			t1 = my->sync(obj->clock,t0);
			break;
		case PC_POSTTOPDOWN:
			t1 = my->postsync(obj->clock,t0);
			break;
		default:
			GL_THROW("invalid pass request (%d)", pass);
			break;
		}
		if (pass == clockpass)
			obj->clock = t1;
	}
	SYNC_CATCHALL(motor);
	return t1;
}

/**
* Allows the core to discover whether obj is a subtype of this class.
*
* @param obj a pointer to this object
* @param classname the name of the object the core is testing
*
* @return 0 if obj is a subtype of this class
*/
EXPORT int isa_motor(OBJECT *obj, char *classname)
{
	if(obj != 0 && classname != 0){
		return OBJECTDATA(obj,motor)->isa(classname);
	} else {
		return 0;
	}
}

/** 
* DELTA MODE
*/
EXPORT SIMULATIONMODE interupdate_motor(OBJECT *obj, unsigned int64 delta_time, unsigned long dt, unsigned int iteration_count_val, bool interupdate_pos)
{
	motor *my = OBJECTDATA(obj,motor);
	SIMULATIONMODE status = SM_ERROR;
	try
	{
		status = my->inter_deltaupdate(delta_time,dt,iteration_count_val,interupdate_pos);
		return status;
	}
	catch (char *msg)
	{
		gl_error("interupdate_motor(obj=%d;%s): %s", obj->id, obj->name?obj->name:"unnamed", msg);
		return status;
	}
}

/**@}*/
