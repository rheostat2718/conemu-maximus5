﻿m4_include(`farversion.m4')m4_dnl
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!-- Copyright © 1996-2000 Eugene Roshal, Copyright © COPYRIGHTYEARS FAR Group -->
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3" >
<assemblyIdentity
	version="MAJOR.MINOR.0.BUILD"
	processorArchitecture="*"
	name="Far Manager"
	type="win32"
/>
<description>File and archive manager</description>
<dependency>
	<dependentAssembly>
		<assemblyIdentity
			type="win32"
			name="Microsoft.Windows.Common-Controls"
			version="6.0.0.0"
			processorArchitecture="*"
			publicKeyToken="6595b64144ccf1df"
			language="*"
		/>
	</dependentAssembly>
</dependency>
<!-- Identify the application security requirements -->
<trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
	<security>
		<requestedPrivileges>
			<requestedExecutionLevel
				level="asInvoker"
				uiAccess="false"
			/>
		</requestedPrivileges>
	</security>
</trustInfo>
<compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
	<application>
		<!--The ID below indicates application support for Windows Vista -->
		<supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}"/>
		<!--The ID below indicates application support for Windows 7 -->
		<supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
	</application>
</compatibility>
<asmv3:application>
	<asmv3:windowsSettings xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">
		<dpiAware>true</dpiAware>
	</asmv3:windowsSettings>
</asmv3:application>
</assembly>
