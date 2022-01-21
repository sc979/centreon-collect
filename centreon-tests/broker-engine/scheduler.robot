*** Settings ***
Resource	../resources/resources.robot
Suite Setup	Clean Before Suite
Suite Teardown	Clean After Suite
Test Setup	Stop Processes

Documentation	Centreon Broker and Engine log_v2
Library	DatabaseLibrary
Library	Process
Library	OperatingSystem
Library	DateTime
Library	Collections
Library	../resources/Engine.py
Library	../resources/Broker.py
Library	../resources/Common.py

*** Test Cases ***
ENRSCHE1
	[Documentation]	check next check of reschedule is last_check+interval_check
	[Tags]	Broker	Engine	scheduler
	Config Engine	${1}
	Config Broker	rrd
	Config Broker	central
	Config Broker	module
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Engine Config Set Value	${0}	log_level_checks	debug

	${start}=	Get Current Date

	Start Broker
	Start Engine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected

	Sleep	1m

	${pid}=	Get Process Id	e0
	${content}=	Create List	[checks] [debug] [${pid}] Rescheduling next check of host: host_14

	${log}=	Catenate	SEPARATOR=	${ENGINE_LOG}	/config0/centengine.log
	${result1}=	check reschedule	${log}	${start}	${content}	
	Should Be True	${result1}	msg=the delta of last_check and next_check is not equal to 60.

	Stop Engine
	Stop Broker
