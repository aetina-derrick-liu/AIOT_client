#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CMDParser.h"
#include "ReadCpuInfo.h"
#include "CPUInfo.h"
#ifdef WIN32
	#include "libcpu.h"
	extern SPD_INFO spd;
#endif

#ifndef WIN32
char* GetCPUManufacturer()
{
	char* ret;

	ret = MEMAllocting(__func__);
	
	GetCommandResult(ret, "lscpu | grep 'Vendor ID'", ":", 1);

	return MEMReallocting(ret);
}

char* GetCPUName()
{
	char* ret;
	
	ret = MEMAllocting(__func__);
	
	GetCommandResult(ret, "lscpu | grep 'Model name'", ":", 1);

	return MEMReallocting(ret);

}

USHORT GetNumberOfCore()
{
	char* ret;
	USHORT retValue;
	
	ret = MEMAllocting(__func__);
	GetCommandResult(ret, "grep '^processor' /proc/cpuinfo | wc -l", NULL, 0);

	retValue = atoi(ret);

	free(ret);	

	return retValue;
}

USHORT GetL2Size()
{
	char* ret;
	USHORT retValue;
	
	ret = MEMAllocting(__func__);
	
	GetCommandResult(ret, "lscpu | grep 'L2 cache'", ":K", 1);

	retValue = atoi(ret);

	free(ret);	

	return retValue;
}

USHORT GetL3Size()
{
	char* ret;
	USHORT retValue;
	
	ret = MEMAllocting(__func__);
	
	GetCommandResult(ret, "lscpu | grep 'L3 cache'", ":K", 1);

	retValue = atoi(ret);

	free(ret);	

	return retValue;
}

USHORT GetCPUFrequency()
{
	char* ret;
	USHORT retValue;

	ret = MEMAllocting(__func__);
#ifdef __aarch64__
	GetCommandResult(ret, "lscpu | grep 'CPU max MHz'", ":", 1);
	//GetCommandResult(ret, "cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "", 0);
	retValue = atoi(ret);
	//retValue = (USHORT)(atoi(ret) / 1024.0 + 0.5); //KHZ -> MHz
#else
	GetCommandResult(ret, "lscpu | grep 'CPU MHz'", ":", 1);
	retValue = atoi(ret);
#endif

	free(ret);	

	return retValue;
}

float GetCPUUsage()
{
	char* ret;
	float retValue;

	ret = MEMAllocting(__func__);

	GetCommandResult(ret, "sudo top -bn 2 -d 0.01 | grep '^%Cpu' | tail -n 1 | gawk '{print $2+$4+$6}'", NULL, 0);

	retValue = atof(ret);

	free(ret);	

	return retValue;

}

USHORT GetFanRPM()
{
	char* ret;
	USHORT retValue;

	ret = MEMAllocting(__func__);
#ifdef __aarch64__
	GetCommandResult(ret, "cat /sys/devices/pwm-fan/target_pwm","",0);
#else
	GetCommandResult(ret, "sensors | grep 'fan'| awk ' $2 >0 '", " :", 1);
#endif

	retValue = atoi(ret);

	free(ret);	

	return retValue;
}

USHORT GetCoreFrequency(USHORT index)
{
	char* ret, *cmd;
	USHORT retValue;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);

#ifdef __aarch64__
	sprintf(cmd, "cat /sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq", index);
	GetCommandResult(ret, cmd, "", 0);
	retValue = (atoi(ret) / 1024.0 + 0.5);
#else
	sprintf(cmd, "grep '^cpu MHz' /proc/cpuinfo | tail -n +%u", index + 1);
	GetCommandResult(ret, cmd, ":", 1);
	retValue = atoi(ret);
#endif

	free(ret);
	free(cmd);	
	return retValue;
}
/***************************************************************/
#define BUF_MAX 1024
#define MAX_CPU 128

int read_fields (FILE *fp, unsigned long long int *fields)
{
	int retval;
	char buffer[BUF_MAX];
 
 	if (!fgets (buffer, BUF_MAX, fp))
	{
		perror ("Error"); 
	}
  /* line starts with c and a string. This is to handle cpu, cpu[0-9]+ */
	retval = sscanf (buffer, "c%*s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
                            &fields[0], 
                            &fields[1], 
                            &fields[2], 
                            &fields[3], 
                            &fields[4], 
                            &fields[5], 
                            &fields[6], 
                            &fields[7], 
                            &fields[8], 
                            &fields[9]); 
	if (retval == 0)
	{
		return -1;
	}
	if (retval < 4) /* Atleast 4 fields is to be read */
	{
		fprintf (stderr, "Error reading /proc/stat cpu field\n");
		return 0;
	}
	return 1;
}

/********************************************************************/
float GetCoreUsage(USHORT index)
{
	FILE *fp;
	unsigned long long int fields[10], total_tick[MAX_CPU], total_tick_old[MAX_CPU], idle[MAX_CPU], idle_old[MAX_CPU], del_total_tick[MAX_CPU], del_idle[MAX_CPU];
	int update_cycle = 0, i, cpus = 0, count;
	double percent_usage;

	fp = fopen ("/proc/stat", "r");
	if (fp == NULL)
	{
		perror ("Error");
	}

	while (read_fields (fp, fields) != -1)
	{
		for (i=0, total_tick[cpus] = 0; i<10; i++)
		{
			total_tick[cpus] += fields[i];
		}
		idle[cpus] = fields[3]; /* idle ticks index */
		cpus++;
	}

	for (count = 0; count < cpus; count++)
	{
		fseek (fp, 0, SEEK_SET);
		total_tick_old[count] = total_tick[count];
		idle_old[count] = idle[count];
		if (!read_fields (fp, fields))
		{
			return 0;
		}
		for (i=0, total_tick[count] = 0; i<10; i++)
		{
			total_tick[count] += fields[i];
		}
		idle[count] = fields[3];
		del_total_tick[count] = total_tick[count] - total_tick_old[count];
		del_idle[count] = idle[count] - idle_old[count];
		percent_usage = ((del_total_tick[count] - del_idle[count]) / (double) del_total_tick[count]) * 100;
		if (count != 0 && count-1==index)
		{
			fclose (fp);
			return percent_usage;
		}
	}
	fclose (fp);
	return 0;
}

float GetCoreTemperature(USHORT index)
{
	char* ret, *cmd;
	float retValue = 0;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
   
#ifdef __aarch64__
	GetCommandResult(ret, "cat /sys/devices/virtual/thermal/thermal_zone0/temp" ,NULL,0);
#else
	sprintf(cmd, "sensors | grep 'Core %u'", index);

	GetCommandResult(ret, cmd, ":°C", 1);
#endif
	if(strlen(ret) > 0)
	{
	  retValue = atof(ret) /1000;
	}

	free(ret);	
	free(cmd);	

	return retValue;
}

float GetCoreVoltage(USHORT index)
{
	char* ret, *cmd;
	float retValue = 0;

	ret = MEMAllocting(__func__);
	cmd = MEMAllocting(__func__);
#ifdef __aarch64__
	GetCommandResult(ret, "cat /sys/bus/i2c/drivers/ina3221x/7-0040/iio:device0/in_voltage1_input" ,NULL,0);
#else
	sprintf(cmd, "sensors | grep 'VCore %u'", index);

	GetCommandResult(ret, cmd, ":V", 1);
#endif
	if(strlen(ret) > 0)
	{
	  retValue = atof(ret) /1000;
	}

	free(ret);	
	free(cmd);	

	return retValue;
}
#else
USHORT GetIndex(const char type,HW_PROCESS_DATA* hw_proc)
{
	USHORT ret = 0;
	switch(type)
	{
		case 'V':
		{
			for(int i = 0; i < hw_proc->volt_nb; i++)
			{
				if(strncpy(hw_proc->volts[i].name,"VID",3))
				{
					ret = i;
					break;
				}
			}
			break;
		}
		case 'T':
		{
			for(int i = 0; i < hw_proc->temp_nb; i++)
			{
				if(strncpy(hw_proc->temps[i].name,"Core",4))
				{
					ret = i;
					break;
				}
			}
		}
		case 'U':
		{
			for(int i = 0; i < hw_proc->utiliz_nb; i++)
			{
				if(strncpy(hw_proc->utilizs[i].name,"CPU",3))
				{
					ret = i;
					break;
				}
			}
		}
		default:
		break;

	}
	return ret;
}
#endif

CPU_INFO* lib_SysInfo_GetCPUInfo()
{
	CPU_INFO* cpuinfoptr = (CPU_INFO*)malloc(sizeof(CPU_INFO));
#ifdef WIN32
	char** tmp = GetWMIData("ROOT\\CIMV2",
							"SELECT * FROM Win32_Processor",
							0,
							5,
							"Manufacturer",
							"Name",
							"NumberOfLogicalProcessors",
							"L2CacheSize",
							"L3CacheSize");
	cpuinfoptr->Manufacturer = *(tmp+0);
	cpuinfoptr->Name = *(tmp+1);
	cpuinfoptr->NumberOfCore = atoi(*(tmp+2));
	cpuinfoptr->L2Size = atoi(*(tmp+3));
	cpuinfoptr->L3Size = atoi(*(tmp+4));
	free(*(tmp+2));
	free(*(tmp+3));
	free(*(tmp+4));
	free(tmp);
#else
	cpuinfoptr->Manufacturer = GetCPUManufacturer();
	cpuinfoptr->Name = GetCPUName();
	cpuinfoptr->NumberOfCore = GetNumberOfCore();
	cpuinfoptr->L2Size = GetL2Size();
	cpuinfoptr->L3Size = GetL3Size();
#endif
	return cpuinfoptr;
}

CPU_DYNAMIC_INFO* lib_SysInfo_GetCPUDyInfo()
{
	USHORT i;
#ifdef WIN32
	int NumberOfCore = 0;
#endif
	CPU_CORE_INFO* prev_node = NULL;
	CPU_DYNAMIC_INFO* ptr = (CPU_DYNAMIC_INFO*)malloc(sizeof(CPU_DYNAMIC_INFO));
#ifdef WIN32
	ptr->core = NULL;
	HW_INFO  hw;
	memset(&hw,  0, sizeof(hw)); 
	if(!init_lib())
        return 0;
	get_hw_info(&hw);
	HW_PROCESS_DATA* hw_proc = &hw.HWProc;
	ptr->Frequency = hw_proc->clocks[0].value;
	ptr->Usage = hw_proc->utilizs[0].value;
	NumberOfCore = hw_proc->clock_nb;

	/***************************/
	USHORT Vdex = GetIndex('V',hw_proc);
	USHORT Tempdex = GetIndex('T',hw_proc);
	USHORT Usgdex = GetIndex('U',hw_proc);
	/***************************/

	HW_MB_DATA *hw_mb = &hw.HWMB;
	ptr->FanRPM = hw_mb->fan;	
#else
	ptr->Frequency = GetCPUFrequency();
	ptr->Usage = GetCPUUsage();
	ptr->FanRPM = GetFanRPM();
#endif

#ifdef WIN32
	for(i = 0; i < NumberOfCore; i++)
#else
	for(i = 0; i < GetNumberOfCore(); i++)
#endif
	{
		CPU_CORE_INFO* current_node = (CPU_CORE_INFO*)malloc(sizeof(CPU_CORE_INFO));
		current_node->next_node = NULL;
		if(prev_node != NULL)
		{
			prev_node->next_node = (void*)current_node;
		}
		else
		{
			current_node->prev_node = NULL;
		}
#ifdef WIN32
		current_node->Frequency = hw_proc->clocks[i].value;
		current_node->Usage = hw_proc->utilizs[Usgdex + i].value;
		current_node->Voltage = hw_proc->volts[Vdex + i].value;
		current_node->Temperature = hw_proc->temps[Tempdex + i].value;
#else
		current_node->Frequency = GetCoreFrequency(i);
		current_node->Usage = GetCoreUsage(i);
		current_node->Temperature = GetCoreTemperature(i);
		current_node->Voltage = GetCoreVoltage(i);
#endif
		if(prev_node != NULL)
		{
			current_node->prev_node = (void*)prev_node;
		}
		else
		{
			ptr->core = current_node;
		}
		prev_node = current_node;
	}
	return ptr;
}

