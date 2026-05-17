#ifndef _included_aclight
#define _included_aclight

void ProcessorFlushCodeCache();

#define ACPI_MACHINE_WIDTH (64)
#define ACPI_CACHE_T ACPI_MEMORY_LIST
#define ACPI_USE_LOCAL_CACHE 1
#define ACPI_FLUSH_CPU_CACHE() ProcessorFlushCodeCache()

#endif 
