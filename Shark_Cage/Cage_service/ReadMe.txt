HOW to install/uninstall service:

Installing the Service
You can install the service from the command prompt by running the following command:

C:\>sc create "My Sample Service" binPath= C:\SampleService.exe

A space is required between binPath= and the value[?]. Also, use the full absolute path to the service executable.

You should now see the service in the Windows Services console. From here you can start and stop the service.

Uninstalling the Service
You can uninstall the service from the command prompt by running the following command:

C:\>sc delete "My Sample Service"  

========================================================================
    CONSOLE APPLICATION : Cage service Project Overview
========================================================================

AppWizard has created this Cage service application for you.

This file contains a summary of what you will find in each of the files that
make up your Cage service application.


Cage service.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

Cage service.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

Cage service.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named Cage service.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
