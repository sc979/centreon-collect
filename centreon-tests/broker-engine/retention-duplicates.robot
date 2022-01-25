*** Settings ***
Resource	../resources/resources.robot
Suite Setup	Clean Before Suite
Suite Teardown	Clean After Suite
Test Setup	Stop Processes

Documentation	Centreon Broker tests on dublicated data that could come from retention when centengine or cbd are restarted
Library	Process
Library	DateTime
Library	OperatingSystem
Library	../resources/Engine.py
Library	../resources/Broker.py
Library	../resources/Common.py
Library	../resources/specific-duplication.py

*** Test Cases ***
BERD1
	[Documentation]	Starting/stopping Broker does not create duplicated events.
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Broker Config Clear Outputs Except	central	["ipv4"]
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
	Clear Retention
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Broker
	Sleep	5s
	Clear Cache
	Start Broker
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Files Contain Same Json	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=Contents of /tmp/lua.log and /tmp/lua-engine.log do not match.
	${result}=	Check Multiplicity When Broker Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERD2
	[Documentation]	Starting/stopping Engine does not create duplicated events.
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Broker Config Clear Outputs Except	central	["ipv4"]
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
	Clear Retention
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Engine
	Sleep	5s
	Clear Cache
	Start Engine
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Files Contain Same Json	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=Contents of /tmp/lua.log and /tmp/lua-engine.log do not match.
	${result}=	Check Multiplicity When Engine Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERDUC1
	[Documentation]	Starting/stopping Broker does not create duplicated events in usual cases
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
	Clear Retention
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Broker
	Sleep	5s
	Clear Cache
	Start Broker
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Check Multiplicity When Broker Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERDUCU1
	[Documentation]	Starting/stopping Broker does not create duplicated events in usual cases with unified_sql
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker Sql Output	central	unified_sql
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
	Clear Retention
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Broker
	Sleep	5s
	Clear Cache
	Start Broker
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Check Multiplicity When Broker Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERDUC2
	[Documentation]	Starting/stopping Engine does not create duplicated events in usual cases
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Clear Retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Engine
	Sleep	5s
	Clear Cache
	Start Engine
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Check Multiplicity When Engine Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERDUCU2
	[Documentation]	Starting/stopping Engine does not create duplicated events in usual cases with unified_sql
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Clear Retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Config Broker Sql Output	central	unified_sql
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Engine
	Sleep	5s
	Clear Cache
	Start Engine
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Check Multiplicity When Engine Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERDUC3U1
	[Documentation]	Starting/stopping Broker does not create duplicated events in usual cases with unified_sql and BBDO 3.0
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker Sql Output	central	unified_sql
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
        Broker Config Add Item	module	bbdo_version	3.0.0
        Broker Config Add Item	central	bbdo_version	3.0.0
        Broker Config Add Item	rrd	bbdo_version	3.0.0
	Clear Retention
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Broker
	Sleep	5s
	Clear Cache
	Start Broker
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Check Multiplicity When Broker Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

BERDUC3U2
	[Documentation]	Starting/stopping Engine does not create duplicated events in usual cases with unified_sql and BBDO 3.0
	[Tags]	Broker	Engine	start-stop	duplicate	retention
	Clear Retention
	Config Engine	${1}
	Engine Config Set Value	${0}	log_legacy_enabled	${0}
	Engine Config Set Value	${0}	log_v2_enabled	${1}
	Config Broker	central
	Config Broker Sql Output	central	unified_sql
	Broker Config Add Lua Output	central	test-doubles	${SCRIPTS}test-doubles-c.lua
	Broker Config Log	central	lua	debug
	Config Broker	module
	Broker Config Add Lua Output	module	test-doubles	${SCRIPTS}test-doubles.lua
	Broker Config Log	module	lua	debug
	Config Broker	rrd
        Broker Config Add Item	module	bbdo_version	3.0.0
        Broker Config Add Item	central	bbdo_version	3.0.0
        Broker Config Add Item	rrd	bbdo_version	3.0.0
	${start}=	Get Current Date
	Start Broker
	Start Engine
	${content}=	Create List	lua: initializing the Lua virtual machine
	${result}=	Find In Log with timeout	${centralLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in cbd
	${result}=	Find In Log with timeout	${moduleLog}	${start}	${content}	30
	Should Be True	${result}	msg=Lua not started in centengine
	${result}=	Check Connections
	Should Be True	${result}	msg=Engine and Broker not connected.
	Sleep	5s
	Stop Engine
	Sleep	5s
	Clear Cache
	Start Engine
	Sleep	25s
	Stop Engine
	Stop Broker
	${result}=	Check Multiplicity When Engine Restarted	/tmp/lua-engine.log	/tmp/lua.log
	Should Be True	${result}	msg=There are events sent several times, see /tmp/lua-engine.log and /tmp/lua.log

