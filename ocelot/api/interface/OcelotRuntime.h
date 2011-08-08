/*! \file OcelotRuntime.h
	\author Gregory Diamos
	\date Tuesday August 11, 2009
	\brief The header file for the OcelotRuntime class.
*/

#ifndef OCELOT_RUNTIME_H_INCLUDED
#define OCELOT_RUNTIME_H_INCLUDED

#include <ocelot/api/interface/OcelotConfiguration.h>

#include <ocelot/trace/interface/MemoryChecker.h>
#include <ocelot/trace/interface/MemoryRaceDetector.h>
#include <ocelot/trace/interface/InteractiveDebugger.h>
#include <ocelot/analysis/interface/ClockCycleCountInstrumentor.h>
#include <ocelot/analysis/interface/BasicBlockInstrumentor.h>
#include <ocelot/analysis/interface/BranchDivergenceInstrumentor.h>
#include <ocelot/analysis/interface/MemoryEfficiencyInstrumentor.h>
#include <ocelot/analysis/interface/ModulePassTestInstrumentor.h>

#include <ocelot/transforms/interface/StructuralTransform.h>
#include <ocelot/transforms/interface/ConvertPredicationToSelectPass.h>
#include <ocelot/transforms/interface/LinearScanRegisterAllocationPass.h>
#include <ocelot/transforms/interface/MIMDThreadSchedulingPass.h>
#include <ocelot/transforms/interface/SyncEliminationPass.h>

namespace ocelot
{
	/*! \brief This is an interface for managing state associated with Ocelot */
	class OcelotRuntime	{
	private:
		trace::MemoryChecker _memoryChecker;
		trace::MemoryRaceDetector _raceDetector;
		trace::InteractiveDebugger _debugger;

		transforms::StructuralTransform _structuralTransform;
		transforms::ConvertPredicationToSelectPass _predicationToSelect;
		transforms::LinearScanRegisterAllocationPass _linearScanAllocation;
		transforms::MIMDThreadSchedulingPass _mimdThreadScheduling;
		transforms::SyncEliminationPass _syncElimination;
		
        analysis::ClockCycleCountInstrumentor _clockCycleCountInstrumentor;
		analysis::BasicBlockInstrumentor _basicBlockInstrumentor;
		analysis::BranchDivergenceInstrumentor _branchDivergenceInstrumentor;
		analysis::MemoryEfficiencyInstrumentor _memoryEfficiencyInstrumentor;
		analysis::ModulePassTestInstrumentor _modulePassTestInstrumentor;

		bool _initialized;
		
	public:
		//! \brief initializes Ocelot runtime state
		OcelotRuntime();
	
		//! \brief initializes the Ocelot runtime object with the 
		//         Ocelot configuration object
		void configure( const api::OcelotConfiguration &c );
					
	};
}

#endif

