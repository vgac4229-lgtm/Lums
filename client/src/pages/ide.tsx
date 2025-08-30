import { useState } from "react";
import { useVoraxEngine } from "@/hooks/use-vorax-engine";
import Sidebar from "@/components/sidebar";
import CodeEditor from "@/components/code-editor";
import ExecutionPanel from "@/components/execution-panel";
import BottomPanel from "@/components/bottom-panel";
import { Button } from "@/components/ui/button";
import { useToast } from "@/hooks/use-toast";
import { Atom, Save, Square } from "lucide-react";

export default function IDEPage() {
  const [code, setCode] = useState(`// VORAX-L Program - LUM Manipulation Demo
zone A, B, C, D;
mem  buf, cache;

// Initialize system with 7 LUMs in Zone A
#alpha := group(•, •, •, •, •, •, •);
emit A += alpha;

// Tick 1: Fusion operation
fuse A, 2•;

// Tick 2: Split operation - distribute A to B and C
split A -> [B, C];

// Tick 3: Move 2 LUMs from the larger zone to D
move C -> D, 2•;

// Tick 4: Store operation
store B -> buf, 1•;

// Tick 5: Retrieve operation
retrieve buf -> A;

// Tick 6: Cycle operation
cycle D % 3;

// Invariant checks
assert conservation: total_lums = 9;
assert no_duplication: unique_lum_ids;
assert deterministic: replay_gives_same_result;`);

  const { 
    zones, 
    memory, 
    currentTick, 
    totalTicks, 
    isExecuting, 
    logs, 
    executionSteps,
    metrics,
    executeCode,
    stepExecution,
    pauseExecution,
    resetExecution
  } = useVoraxEngine();

  const { toast } = useToast();

  const handleSave = () => {
    // In a real implementation, this would save to a file system or server
    toast({
      title: "Project saved",
      description: "Your VORAX-L code has been saved successfully.",
    });
  };

  const handleStop = () => {
    resetExecution();
    toast({
      title: "Execution stopped",
      description: "The VORAX engine has been reset.",
    });
  };

  const handleExecute = async () => {
    try {
      await executeCode(code);
      toast({
        title: "Execution started",
        description: "Your VORAX-L program is now running.",
      });
    } catch (error) {
      toast({
        title: "Execution failed",
        description: error instanceof Error ? error.message : "Unknown error occurred",
        variant: "destructive",
      });
    }
  };

  return (
    <div className="h-screen flex flex-col">
      {/* Header */}
      <header className="bg-card border-b border-border px-6 py-3 flex items-center justify-between">
        <div className="flex items-center space-x-4">
          <div className="flex items-center space-x-2">
            <div className="w-8 h-8 bg-primary rounded-lg flex items-center justify-center">
              <Atom className="text-primary-foreground w-4 h-4" />
            </div>
            <div>
              <h1 className="text-lg font-semibold">LUMS/VORAX IDE</h1>
              <p className="text-xs text-muted-foreground">Post-Numerical Computing Environment</p>
            </div>
          </div>
        </div>
        
        <div className="flex items-center space-x-3">
          <div className="flex items-center space-x-2 text-sm">
            <span className="text-muted-foreground">Project:</span>
            <span className="text-foreground font-medium">vorax-demo.lums</span>
          </div>
          <div className="w-px h-6 bg-border"></div>
          <Button 
            onClick={handleSave}
            size="sm"
            data-testid="button-save"
          >
            <Save className="w-4 h-4 mr-1.5" />
            Save
          </Button>
          <Button 
            onClick={handleStop}
            size="sm" 
            variant="destructive"
            data-testid="button-stop"
          >
            <Square className="w-4 h-4 mr-1.5" />
            Stop
          </Button>
        </div>
      </header>

      {/* Main Content */}
      <div className="flex-1 flex overflow-hidden">
        <Sidebar zones={zones} memory={memory} />
        
        <div className="resizer"></div>
        
        <CodeEditor 
          code={code} 
          onChange={setCode}
          onExecute={handleExecute}
          onStep={stepExecution}
          onReset={resetExecution}
          isExecuting={isExecuting}
        />
        
        <div className="resizer"></div>
        
        <ExecutionPanel 
          currentTick={currentTick}
          totalTicks={totalTicks}
          isExecuting={isExecuting}
          logs={logs}
          executionSteps={executionSteps}
          zones={zones}
          metrics={metrics}
          onExecute={handleExecute}
          onStep={stepExecution}
          onPause={pauseExecution}
          onReset={resetExecution}
        />
      </div>

      <BottomPanel 
        zones={zones}
        metrics={metrics}
      />
    </div>
  );
}
